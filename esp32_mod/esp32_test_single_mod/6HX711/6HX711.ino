#include "HX711.h"
int Weight = 0;

void setup() {
    Init_Hx711();  //初始化HX711模組連接的IO設置
    Serial.begin(115200);
    Serial.print("Welcome to use!\n");
    Get_Maopi();  //開始校正
    delay(3000);
    Get_Maopi();  //校正結束
    Serial.print("Start!\n");
}

void loop() {
    Weight = Get_Weight();  //計算放在感測器上的重物重量

    Serial.print(Weight);  //顯示重量
    Serial.println("g ");  //顯示單位
        delay(1000);				//延時1s
}
