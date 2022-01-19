#include <ArduinoJson.h>  //解析json
#include <HTTPClient.h>   //訪問heroku
// WIFI
const char ssid[] = "旭祺手機";   //修改為你家的WiFi網路名稱 我最帥 / li
const char pwd[] = "f130323558";  //修改為你家的WiFi密碼 david890415 /leelee0222

// 訪問heroku
String domain = "https://indoor-foodie.herokuapp.com";
String update_api = "/Update";
HTTPClient http;
bool update_heroku();

void setup(void) {
    Serial.begin(115200);
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

void loop(void) {
    update_heroku();
    delay(1000);
}

// Update
bool update_heroku() {
    String url = domain + update_api;
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
