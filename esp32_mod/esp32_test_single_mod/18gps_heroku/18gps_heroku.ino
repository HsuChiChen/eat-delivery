#include <ArduinoJson.h>  //解析json
#include <HTTPClient.h>   //訪問heroku
#include <TinyGPS++.h>
TinyGPSPlus gps;  // the TinyGPS++ object
HardwareSerial gpsSerial(1);

// WIFI
const char ssid[] = "四宮輝夜";   //修改為你家的WiFi網路名稱 我最帥 / li
const char pwd[] = "f130323558";  //修改為你家的WiFi密碼 david890415 /leelee0222

// 訪問heroku
String domain = "https://indoor-foodie.herokuapp.com";
String update_api = "/Update";
HTTPClient http;
bool update_heroku();

//GPS
double lat_latest = 20;
double long_latest = 20;

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(9600, SERIAL_8N1, 35, 27);  // Serial的TX,RX  //GPS TX35 RXX27
    Serial.println(F("Arduino - GPS module"));

    //WIFI
    WiFi.mode(WIFI_STA);  //設置WiFi模式
    WiFi.begin(ssid, pwd);
    Serial.print("WiFi connecting");
    //當WiFi連線時會回傳WL_CONNECTED，因此跳出迴圈時代表已成功連線
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }

    Serial.println("");
    Serial.print("IP位址:");
    Serial.println(WiFi.localIP());  //讀取IP位址
    Serial.print("WiFi RSSI:");
    Serial.println(WiFi.RSSI());  //讀取WiFi強度
    printf("Connection succeed!\n");
}

void loop() {
    gps_update();
    update_heroku();
}

void gps_update() {
  for(int i = 0; i<10; i++){
    if (gpsSerial.available() > 0) {
        if (gps.encode(gpsSerial.read())) {
            if (gps.location.isValid()) {
                lat_latest = gps.location.lat();
                long_latest = gps.location.lng();
                printf("replace!");
            } else {
                Serial.println(F("- location: INVALID"));
            }
        }
    }

    if (millis() > 5000 && gps.charsProcessed() < 10)
        Serial.println(F("No GPS data received: check wiring"));
    
    
    }
}

// Update
bool update_heroku() {
    String gps_url = "?lat=" + String(lat_latest, 6) + "&long=" + String(long_latest, 6);
    String url = domain + update_api + gps_url;
    Serial.print(url);
    String payload = "";

    if ((WiFi.status() == WL_CONNECTED)) {
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            payload = http.getString();
            Serial.printf("回應本體：%s\n", payload.c_str());
        } else {
            Serial.println("HTTP Get error");
        }

        http.end();
        if (payload[0] == '1') {
            return true;
        } else {
            return false;
        }
    }
}
