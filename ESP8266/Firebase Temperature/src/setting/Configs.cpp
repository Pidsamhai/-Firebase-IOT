#include "Config.h"
#include "EEPROM.h"
#include "ArduinoJson.h"

static unsigned long DEFAULT_DISPLAY_INTERVAL = 10000;
static unsigned long DEFAULT_UPLOAD_DATA_INTERVAL = 10000;

void Config::init() {
    Serial.print("==== Begin Read Setting====");
    Config config;
    EEPROM.begin(512);
    EEPROM.get(0, config);
    if (config.DISPLAY_INTERVAL <= DEFAULT_DISPLAY_INTERVAL)
    {
        config.DISPLAY_INTERVAL = DEFAULT_DISPLAY_INTERVAL;
    }
    
    if (config.UPLOAD_DATA_INTERVAL <= DEFAULT_UPLOAD_DATA_INTERVAL)
    {
        config.UPLOAD_DATA_INTERVAL = DEFAULT_UPLOAD_DATA_INTERVAL;
    }
    * this = config;
}

void Config::save() {
    EEPROM.put(0, * this);
    EEPROM.commit();
}

void Config::parse(byte *playload) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, (char *)(playload));
    Config config;
    config.DISPLAY_INTERVAL = doc["display_interval"];
    config.UPLOAD_DATA_INTERVAL = doc["upload_interval"];
    if (config.DISPLAY_INTERVAL <= DEFAULT_DISPLAY_INTERVAL)
    {
        config.DISPLAY_INTERVAL = DEFAULT_DISPLAY_INTERVAL;
    }
    if (config.UPLOAD_DATA_INTERVAL <= DEFAULT_UPLOAD_DATA_INTERVAL)
    {
        config.UPLOAD_DATA_INTERVAL = DEFAULT_UPLOAD_DATA_INTERVAL;
    }
    * this = config;
}