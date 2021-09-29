#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "setting/Setting.h"
#include <PubSubClient.h>
#include <setting/Config.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define RESET 12
#define LED 4
#define CS 15
#define CLOCK 0
#define DATA 13
#define DHTPIN 4
#define DHTTYPE DHT11
const String UID = "e58314eb";

DHT dht(DHTPIN, DHTTYPE);
char converChar[16];
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig firebaseConfig;
Setting setting;
Config config;
WiFiClient espClient;
PubSubClient client(espClient);
char SettingPath[20];

unsigned long upLoadDataMillis = 0;
unsigned long disPlayDataMillis = 0;

U8G2_HX1230_96X68_F_3W_SW_SPI u8g2(U8G2_R0, /* clock=*/CLOCK, /* data=*/DATA, /* cs=*/CS, /* reset=*/RESET);
ThreeWire myWire(5, 16, 2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

#define countof(a) (sizeof(a) / sizeof(a[0]))

void writeDiaplay(float t, float h)
{
    RtcDateTime dt = Rtc.GetDateTime();
    char datestring[20];
    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%02u %02u:%02u:%02u"),
               dt.Month(),
               dt.Day(),
               dt.Year(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());
    u8g2.clearBuffer(); // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr);
    sprintf(converChar, "T %.2f C", t); // choose a suitable font
    u8g2.drawStr(0, 10, converChar);    // write something to the internal memory
    sprintf(converChar, "H %.2f %%", h);
    u8g2.drawStr(0, 20, converChar); // write something to the internal memory
    u8g2.drawStr(0, 30, datestring);
    u8g2.sendBuffer(); // transfer internal memory to the display
}


void printSetting() {
    Serial.printf("Config display %ld, upload %ld\n", config.DISPLAY_INTERVAL, config.UPLOAD_DATA_INTERVAL);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    config.parse(payload);
    printSetting();
    config.save();
}

void setup(void)
{
    EEPROM.begin(512);
    Serial.begin(115200);
    config.init();
    dht.begin();
    u8g2.begin();
    Rtc.Begin();
    WiFi.begin(setting.WIFI_SSID, setting.WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the api key (required) */
    firebaseConfig.api_key = setting.API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = setting.USER_EMAIL;
    auth.user.password = setting.USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    firebaseConfig.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

    Firebase.begin(&firebaseConfig, &auth);

    Firebase.reconnectWiFi(true);
    // sprintf(SettingPath, "%s/setting", UID);
    client.setServer(setting.MQTT_SERVER, setting.MQTT_PORT);
    client.setCallback(callback);
    if (client.connect("", "", ""))
    {
        Serial.println("Connected...");
        client.subscribe("e58314eb/setting");
        Serial.print("Subscribe at");
        Serial.print(SettingPath);
        Serial.println();
    }
}

void uploadData(float t, float h)
{
    if (Firebase.ready() && (millis() - upLoadDataMillis > config.UPLOAD_DATA_INTERVAL || upLoadDataMillis == 0))
    {
        upLoadDataMillis = millis();
        u8g2.drawStr(0, 40, "Uploading...");
        u8g2.sendBuffer();
        int a = Rtc.GetDateTime().Epoch64Time();
        String documentPath = "ESP8266/" + UID + "/temp/" + String(a);
        FirebaseJson content;
        content.set("fields/id/stringValue", String(a));
        content.set("fields/temp/doubleValue", t);
        content.set("fields/humidity/doubleValue", h);
        Serial.print("Create document... ");

        if (Firebase.Firestore.createDocument(&fbdo, setting.FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
            Serial.printf("ok\n%s\n\n", fbdo.payload().c_str());
        else
            Serial.println(fbdo.errorReason());
        u8g2.drawStr(0, 40, "               ");
        u8g2.sendBuffer();
    }
}

void loop(void)
{
    if (millis() - disPlayDataMillis > config.DISPLAY_INTERVAL || disPlayDataMillis == 0)
    {
        float t = dht.readTemperature();
        float h = dht.readHumidity();
        disPlayDataMillis = millis();
        writeDiaplay(t, h);
        uploadData(t, t);
    }
}