#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
// https://indoor-foodie.herokuapp.com/Arrived
// 設定網路基地台SSID跟密碼
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// 請更換成 Thing Speak WRITE API KEY
//const char* apiKey = "#20";
const char* resource = "/Arrived";

// Thing Speak API server
const char* server = "YOUR_DOMAIN";

unsigned long previousMillis = 0;  // will store last temp was read
const long interval = 2000;        // interval at which to read sensor

void setup() {
    // Initializing serial port for debugging purposes
    Serial.begin(9600);
    delay(10);

    Serial.println("");
    Serial.println("");
    Serial.print("Connecting To: ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    // 等待連線，並從 Console顯示 IP
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi Connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    pinMode(22, OUTPUT);
}

void loop() {
    int touch_value = touchRead(4);
    printf("%u\n", touch_value);
    Serial.print("Connecting to ");
    Serial.print(server);
    WiFiClient client;

    // 使用 80 Port 連線
    if (client.connect(server, 80)) {
        Serial.println(F("connected"));
    } else {
        Serial.println(F("connection failed"));
        return;
    }

    Serial.print("Request resource: ");
    Serial.println(resource);
    client.print(String("GET ") + resource +
                 " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    int timeout = 5 * 10;  // 5 seconds
    while (!!!client.available() && (timeout-- > 0)) {
        delay(100);
    }

    if (!client.available()) {
        Serial.println("No response, going back to sleep");
    }
    while (client.available()) {
        Serial.write(client.read());
    }

    Serial.println("\nclosing connection");
    client.stop();

    // 每三分鐘會上傳一筆資料
    delay(10000);
}
