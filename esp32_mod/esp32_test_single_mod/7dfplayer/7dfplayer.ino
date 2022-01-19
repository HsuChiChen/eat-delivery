#include "DFRobotDFPlayerMini.h"

HardwareSerial myHardwareSerial(2);         //ESP32可宣告需要一個硬體序列
DFRobotDFPlayerMini myDFPlayer;             //啟動DFPlayer撥放器
void printDetail(uint8_t type, int value);  //宣告播放控制程式

void setup() {
    Serial.begin(9600);
    //啟動mp3連線
    myHardwareSerial.begin(9600, SERIAL_8N1, 17, 16);  // Serial的TX,RX
    //實際上只用到TX傳送指令，因此RX可不接（接收player狀態）
    Serial.println("Initializing DFPlayer ... (May take 1-2 seconds)");
    myDFPlayer.begin(myHardwareSerial);  //將DFPlayer播放器宣告在HardwareSerial控制
    delay(500);
    myDFPlayer.volume(20);  //設定聲音大小（0-30）
    Serial.println("test1");
    myDFPlayer.pause();
    myDFPlayer.playLargeFolder(1, 1);  //播放mp3內的0001.mp3 3秒鐘
    //    delay(3000);
}

void loop() {
    if (myDFPlayer.available()) {
        printDetail(myDFPlayer.readType(), myDFPlayer.read());  //Print the detail message from DFPlayer to handle different errors and states.
    }
    if (myDFPlayer.readState() == 0) {
        myDFPlayer.playLargeFolder(2, 3);
    }
    delay(20);
}

void printDetail(uint8_t type, int value) {
    switch (type) {
        case TimeOut:
            Serial.println(F("Time Out!"));
            break;
        case WrongStack:
            Serial.println(F("Stack Wrong!"));
            break;
        case DFPlayerCardInserted:
            Serial.println(F("Card Inserted!"));
            break;
        case DFPlayerCardRemoved:
            Serial.println(F("Card Removed!"));
            break;
        case DFPlayerCardOnline:
            Serial.println(F("Card Online!"));
            break;
        case DFPlayerPlayFinished:
            Serial.print(F("Number:"));
            Serial.print(value);
            Serial.println(F(" Play Finished!"));
            break;
        case DFPlayerError:
            Serial.print(F("DFPlayerError:"));
            switch (value) {
                case Busy:
                    Serial.println(F("Card not found"));
                    break;
                case Sleeping:
                    Serial.println(F("Sleeping"));
                    break;
                case SerialWrongStack:
                    Serial.println(F("Get Wrong Stack"));
                    break;
                case CheckSumNotMatch:
                    Serial.println(F("Check Sum Not Match"));
                    break;
                case FileIndexOut:
                    Serial.println(F("File Index Out of Bound"));
                    break;
                case FileMismatch:
                    Serial.println(F("Cannot Find File"));
                    break;
                case Advertise:
                    Serial.println(F("In Advertise"));
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}
