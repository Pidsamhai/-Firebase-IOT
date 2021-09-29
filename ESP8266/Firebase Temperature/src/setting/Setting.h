struct Setting {
    static char  WIFI_SSID[10];
    static char  WIFI_PASSWORD[20];
    static char  API_KEY[50];
    static char  FIREBASE_PROJECT_ID[50];
    static char  USER_EMAIL[50];
    static char  USER_PASSWORD[50];
    static char  MQTT_SERVER[50];
    static int   MQTT_PORT;
    static char  MQTT_TOKEN[255];
    void init();
    void save();
};