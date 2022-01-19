#include <ArduinoJson.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
String domain = "YOUR_DOMAIN";
String API = "/Order";

HTTPClient http;
int menu[10];
//String customer_name = "";
int UID[4];

String openWeather() {
    String url = domain + API;
    String payload = "";

    if ((WiFi.status() == WL_CONNECTED)) {
        http.begin(url);
        int httpCode = http.GET();

        if (httpCode > 0) {
            payload = http.getString();
            Serial.printf("回應本體：%s\n", payload.c_str());
        } else {
            Serial.println("HTTP請求出錯了～");
        }

        http.end();
        return payload;
    }
}

void parse_order(String json) {
    DynamicJsonDocument doc(800);

    deserializeJson(doc, json);
    for (int i = 0; i < 10; i++)
        menu[i] = (int)doc["menu"][i];
    String temp = doc["room"];
    char room_number = temp[0];
    //  customer_name = doc["name"];
    for (int i = 0; i < 4; i++)
        UID[i] = (int)doc["rfid"][i];

    for (int i = 0; i < 10; i++)
        printf("%d\n", menu[i]);
    printf("%c\n", room_number);
    for (int i = 0; i < 4; i++)
        printf("%d\n",  UID[i]);
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\nIP位址：");
    Serial.println(WiFi.localIP());

    String payload = openWeather();

    if (payload != "") {
        parse_order(payload);
    }
}

void loop() {
}
