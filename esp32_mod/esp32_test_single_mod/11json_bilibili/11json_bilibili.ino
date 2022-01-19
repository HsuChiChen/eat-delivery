 #include <ArduinoJson.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
String domain = "https://api.bilibili.com";
String API = "/x/relation/stat?vmid=1447797468";

HTTPClient http;

String openWeather()
{
  String url = domain + API;
  String payload = "";

  if ((WiFi.status() == WL_CONNECTED))
  {

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0)
    {
      payload = http.getString();
      Serial.printf("回應本體：%s\n", payload.c_str());
    }
    else
    {
      Serial.println("HTTP請求出錯了～");
    }

    http.end();
    return payload;
  }
}

void parseWeather(String json)
{
  DynamicJsonDocument doc(800);

  deserializeJson(doc, json);
  int fan_num = (float)doc["data"]["follower"];
  printf("follower：%d\n", fan_num);
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP位址：");
  Serial.println(WiFi.localIP());

  String payload = openWeather();

  if (payload != "")
  {
    parseWeather(payload);
  }
}

void loop()
{
}
