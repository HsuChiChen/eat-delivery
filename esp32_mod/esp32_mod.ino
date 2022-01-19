#include <Arduino.h>      //arduino基本功能
#include <ArduinoJson.h>  //解析json
#include <HTTPClient.h>   //訪問heroku
#include <MFRC522.h>      //RFID
#include <SPI.h>          //RFID
#include <U8g2lib.h>      //OLED顯示編碼
#include <WiFi.h>         //WIFI聯網功能

#include "DFRobotDFPlayerMini.h"  //mp3撥放器
#include "HX711.h"                //秤重功能

template <typename T>
class Queue {
    struct Node {
        T room_number;
        int menu[10];
        int UID[4];
        Node *next;
        long waiting_time;
    };
    Node *head;
    Node *tail;
    int qsize;
    long start_time;

   public:
    Queue() {
        head = NULL;
        tail = NULL;
        qsize = 0;
    }

    int size() {
        return qsize;
    }

    bool empty() {
        if (qsize == 0) {
            return true;
        } else {
            return false;
        }
    }
    void put(int (&menu)[10], const T &room_number, int (&UID)[4]) {
        Node *newNode = new Node;
        if (qsize) {
            tail->next = newNode;
            newNode->room_number = room_number;
            for (int i = 0; i < 10; i++)
                newNode->menu[i] = menu[i];
            for (int i = 0; i < 4; i++)
                newNode->UID[i] = UID[i];
            newNode->waiting_time = millis();
            newNode->next = NULL;
            tail = newNode;
        } else {
            head = tail = newNode;
            newNode->room_number = room_number;
            for (int i = 0; i < 10; i++)
                newNode->menu[i] = menu[i];
            for (int i = 0; i < 4; i++)
                newNode->UID[i] = UID[i];
            newNode->waiting_time = millis();
            newNode->next = NULL;
        }
        qsize++;
    }

    void delete_node() {
        Node *temp;

        if (empty()) {
            printf("queue is empty\n");
        } else {
            temp = head;
            head = head->next;
            delete temp;
            qsize--;
        }
    }

    long read_time() {
        return millis() - head->waiting_time;
    }

    int menu(int value) {
        if (empty()) {
            printf("empty menu\n");
            return -1;
        } else if (value < 0 && value > 10) {
            printf("invalid menu number\n");
            return -1;
        } else {
            return head->menu[value];
        }
    }

    int uid(int value) {
        if (empty()) {
            printf("empty uid\n");
            return -1;

        } else if (value < 0 && value > 4) {
            printf("invalid uid number\n");
            return -1;
        } else {
            return head->UID[value];
        }
    }

    char room_number() {
        if (empty()) {
            printf("iempty room_number\n");
            return 'a';  //error messenage
        } else {
            return head->room_number;
        }
    }

    void destroyQueue() {
        while (!empty()) {
            printf("delete\n");
            delete_node();
        }
    }

    ~Queue() {
        destroyQueue();
    }
};

// WIFI
const char ssid[] = "我最帥";      //修改為你家的WiFi網路名稱 我最帥 / li
const char pwd[] = "david890415";  //修改為你家的WiFi密碼 david890415 /leelee0222

// OLED
// U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 32, 33, U8X8_PIN_NONE); // for software I2C
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
void oled_wifi();
void oled_empty();
void oled_cook();
void oled_deliver(long count_5s);
void oled_cancel(long count_3s);
int joystick_state = 0;
String oled_string;
void oled_weight();
int rfid_err = 0;
void oled_rfid_init();

// queue for starving customer
Queue<char> queue;

// button
bool enter;

// buzzer
#define BITS 10
#define BUZZER_PIN 4  // 蜂鳴器接在腳4

// build-in bluetooth
// BluetoothSerial SerialBT;
// char *pin = "1234";  //建立連接時的密碼，不設置與hc05連接不上
HardwareSerial SerialBT(1);

// HX711
int weight = 0;
int ideal_weight = 0;

// RFID
#define RST_PIN 32
#define SS_PIN 5  //RC522卡上的SDA
MFRC522 mfrc522;
// byte uid[] = {0x44, 0xE7, 0x02, 0x28};
//這是我們指定的卡片UID，可由讀取UID的程式取得特定卡片的UID，再修改這行

// mp3撥放器
HardwareSerial myHardwareSerial(2);         //ESP32可宣告需要一個硬體序列
DFRobotDFPlayerMini myDFPlayer;             //啟動DFPlayer撥放器
void printDetail(uint8_t type, int value);  //宣告播放控制程式
long count_state3 = 0;
int order_name = 0;

// HTTP client (accces heroku)
String domain = "https://indoor-foodie.herokuapp.com";
String update_api = "/Update";
String order_api = "/Order";

HTTPClient http;
bool update_heroku();
void update_order();
void notice_status(String API);
long heroku_interval = 0;

// stauts
int state = 0;
void state_for_cook();
void state_to_room();
void arrive_announce();
void state_for_RFID();
void state_for_get_order();
void state_go_back();

void setup(void) {
    // Serial port
    Serial.begin(115200);
    Serial.println();
    Serial.println("-----------");

    // HX711
    Init_Hx711();  //初始化連接的IO設置
    Get_Maopi();   //開始校正calibration
    Serial.println("HX711 start calibration");

    // button
    pinMode(15, INPUT_PULLDOWN);
    Serial.println("button OK!");

    // DFPlayer
    myHardwareSerial.begin(9600, SERIAL_8N1, 17, 16);  // Serial的TX,RX
    //實際上只用到TX傳送指令，因此RX可不接（接收player狀態）
    Serial.println("DFPlayer OK!");
    myDFPlayer.begin(myHardwareSerial);  //將DFPlayer播放器宣告在HardwareSerial控制
    myDFPlayer.volume(20);               //設定聲音大小（0-30）
    myDFPlayer.playLargeFolder(1, 1);    // 0001送餐系統啟動中.wav

    // build-in bluetooth
    // SerialBT.setPin(pin);
    // SerialBT.begin("ESP32test");  //Bluetooth device name
    // Serial.println("The device started, now you can pair it with bluetooth!");

    // HC-05
    SerialBT.begin(38400, SERIAL_8N1, 27, 13);  // Serial的TX,RX
    Serial.println("HC-05 OK!");

    // OLED
    u8g2.begin();
    u8g2.enableUTF8Print();            // enable UTF8 support for the Arduino print() function
    u8g2.setFont(u8g2_font_7x13B_mf);  // font
    Serial.println("OLED OK!");

    // buzzer
    ledcSetup(0, 2000, BITS);  // PWM預設為20KHz，10位元解析度。
    ledcAttachPin(BUZZER_PIN, 0);
    Serial.println("buzzer OK!");

    Serial.println("-----------");
    //WIFI
    WiFi.mode(WIFI_STA);  //設置WiFi模式
    WiFi.begin(ssid, pwd);
    Serial.print("WiFi connecting");
    //當WiFi連線時會回傳WL_CONNECTED，因此跳出迴圈時代表已成功連線
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        oled_wifi();
        delay(100);
        if ((millis() % 7000) < 150)
            myDFPlayer.playLargeFolder(1, 2);  // 0002請確認無線網路連線.wav
    }

    Serial.println("");
    Serial.print("IP位址:");
    Serial.println(WiFi.localIP());  //讀取IP位址
    Serial.print("WiFi RSSI:");
    Serial.println(WiFi.RSSI());  //讀取WiFi強度
    Serial.println("WIFI Connection OK!");
    Serial.println("-----------");

    // RFID
    SPI.begin();
    mfrc522.PCD_Init(SS_PIN, RST_PIN);  // 初始化MFRC522卡
    Serial.print("Reader : ");
    mfrc522.PCD_DumpVersionToSerial();  // 顯示讀卡設備的版本
    Serial.println("RFID reader OK!");
    Serial.println("-----------");

    // HX711
    Get_Maopi();  //校正
    Serial.println("HX711 calibration OK!");
}

void loop(void) {
    // http client to heroku
    if (queue.empty() && millis() - heroku_interval > 3000) {  // 沒訂單時，3秒更新一次
        heroku_interval = millis();
        if (update_heroku())
            update_order();
    }
    //    else if (millis() - heroku_interval > 10000) {  // 有訂單時，10秒更新一次
    //        heroku_interval = millis();
    //        if (update_heroku())
    //            update_order();
    //    }

    // read data from server
    // server.handleClient();
    switch (state) {
        case 0:
            state_for_cook();
            break;
        case 1:
            state_to_room();
            rfid_err = 0;
            break;
        case 2:
            arrive_announce();
            break;
        case 3:
            state_for_RFID();
            break;
        case 4:
            state_for_get_order();
            break;
        case 5:
            state_go_back();
            break;
        default:
            break;
    }

    // bluetooth for test
    if (Serial.available()) {
        SerialBT.write(Serial.read());
    }
    //    if (SerialBT.available()) {
    //        Serial.write(SerialBT.read());
    //    }
}

// state == 0
void state_for_cook() {
    // read data from Joystick
    int vrx, vry, sw;
    sw = analogRead(36);
    vrx = analogRead(39);
    vry = analogRead(34);
    //    printf("VRx=%d, VRy=%d, SW=%d\n", vrx, vry, sw);

    // analysis data from Joystick
    if (vrx <= 2500 && vry > 4000)
        joystick_state = 1;
    else if (vrx > 2500 && vry > 4000)
        joystick_state = 2;
    else if (vry < 100)
        joystick_state = 0;

    //OLED
    if (queue.empty())
        oled_empty();
    else
        oled_cook();

    // bottom from button 15
    enter = digitalRead(15);
    // Serial.println(enter);

    // OLED選擇deliver，並按下按鈕
    if (!queue.empty() && enter && joystick_state == 2) {
        Serial.println("deliver confirm");
        switch (queue.room_number()) {
            case 'h':
                myDFPlayer.playLargeFolder(1, 5);  // 0005將於五秒後發車，目的地是房號h.wav
                break;
            case 'j':
                myDFPlayer.playLargeFolder(1, 6);  // 0006將於五秒後發車，目的地是房號j.wav
                break;
            case 'k':
                myDFPlayer.playLargeFolder(1, 7);  // 0007將於五秒後發車，目的地是房號k.wav
                break;
            default:
                myDFPlayer.playLargeFolder(1, 8);  // 0008查無此房號.wav
        }

        long count_5s = millis();
        while (millis() - count_5s < 5000) {  //5s
            oled_deliver(count_5s);
            delay(50);
        }
        state = 1;
        SerialBT.write(queue.room_number());  //車輛移動到h的位置
        notice_status("/MoveOn");             // notice line
        ideal_weight = Get_Weight();
        myDFPlayer.playLargeFolder(2, 6);  // 0006dance_money_crop.mp3
        return;
    }

    // OLED選擇cancel，並按下按鈕
    if (!queue.empty() && enter && joystick_state == 1) {
        Serial.println("cancel confirm");
        long count_3s = millis();
        notice_status("/Canceled");           // notice line
        myDFPlayer.playLargeFolder(1, 4);     // 0004顧客訂單取消中.wav
        while (millis() - count_3s < 3000) {  //5s
            oled_cancel(count_3s);
            delay(50);
        }
        queue.delete_node();
        return;
    }
}

void oled_wifi() {
    u8g2.firstPage();
    do {
        // row 1 - status
        u8g2.setCursor(5, 10);
        u8g2.print("Wifi connecting");
        // row 2 - wating time
        oled_string = "waiting time: " + (String)(millis() / 1000) + "s";
        u8g2.drawStr(0, 30, oled_string.c_str());
        // row 3 - progress bar
        u8g2.drawBox(0, 40, millis() / 100 % 127, 15);
    } while (u8g2.nextPage());
}

void oled_cook() {
    u8g2.firstPage();
    do {
        // row 1 - number of waiting customer
        u8g2.setCursor(0, 10);
        String order_msg = "+" + (String)queue.size();
        u8g2.print(order_msg.c_str());
        // row 1 - 10 object
        int cursor_pos = 5;
        u8g2.setCursor(cursor_pos, 10);
        char memu_list[10];
        String memu_list_msg = "";
        int non_zero_number = 0;
        int currrent_number = 1;
        for (int i = 0; i < 10; i++) {
            if (queue.menu(i) > 0) {
                non_zero_number++;
            }
        }

        for (int i = 0; i < 10; i++) {
            if (queue.menu(i) > 0) {
                if (currrent_number < non_zero_number && currrent_number < 3) {
                    memu_list_msg += (String)(i + 1) + ":" + (String)queue.menu(i) + ",";
                    cursor_pos += 13;
                    u8g2.setCursor(cursor_pos, 10);
                    currrent_number++;
                } else if (currrent_number == non_zero_number) {
                    memu_list_msg += (String)(i + 1) + ":" + (String)queue.menu(i);
                } else if (currrent_number == 3) {  // OLED顯示大小限制，因此>3筆訂單者，最多只會顯示2筆訂單
                    memu_list_msg += "...";
                    break;
                }
            }
        }
        u8g2.print(memu_list_msg.c_str());
        // row 2 - wating time
        u8g2.setCursor(0, 30);
        oled_string = "waiting time: " + (String)(queue.read_time() / 1000) + "s";
        u8g2.print(oled_string.c_str());
        // row 3 - cancel
        u8g2.setCursor(6, 55);
        u8g2.print("cancel");
        // row 3 - cancel box for Joystick
        if (joystick_state == 1)
            u8g2.drawRFrame(1, 42, 50, 20, 7);
        // row 3 - deliver
        u8g2.setCursor(70, 55);
        u8g2.print("deliver");
        // row 3 - deliver box for Joystick
        if (joystick_state == 2)
            u8g2.drawRFrame(65, 42, 60, 20, 7);
        // row 4 - progress bar
        u8g2.drawBox(0, 60, queue.read_time() / 50 % 127, 4);
    } while (u8g2.nextPage());
}

void oled_deliver(long count_5s) {
    u8g2.firstPage();
    do {
        // row 1 - status
        u8g2.setCursor(5, 10);
        u8g2.print("Start in");
        // row 2 - wating time
        u8g2.setFont(u8g2_font_inr38_mn);  // font
        u8g2.setCursor(55, 55);
        oled_string = (String)(5 - (millis() - count_5s) / 1000);
        u8g2.print(oled_string.c_str());
        u8g2.setFont(u8g2_font_7x13B_mf);  // font
        // row 3 - progress bar
        u8g2.drawBox(0, 60, (millis() - count_5s) * 0.0254, 4);  // 1/1000 * 127 / 5 = 0.0254
    } while (u8g2.nextPage());
}

void oled_cancel(long count_3s) {
    u8g2.firstPage();
    do {
        // row 1 - state
        u8g2.setCursor(20, 10);
        u8g2.print("This order");
        // row 2 - notice
        u8g2.setCursor(10, 35);
        u8g2.print("is being canceled!");
        // row 3 - progress bar
        u8g2.drawBox(0, 60, (millis() - count_3s) * 0.0423, 4);  // 1/1000 * 127 / 3 = 0.0423
    } while (u8g2.nextPage());
}

void oled_empty() {
    u8g2.firstPage();
    do {
        // row 1 - status
        u8g2.setCursor(30, 10);
        u8g2.print("No order~~");
        u8g2.drawBox(millis() / 25 % 167 - 40, 30, 40, 15);
        u8g2.drawDisc(millis() / 25 % 167 - 8, 50, 5, U8G2_DRAW_ALL);
        u8g2.drawDisc(millis() / 25 % 167 - 33, 50, 5, U8G2_DRAW_ALL);
    } while (u8g2.nextPage());
}

// state == 1
void state_to_room() {
    // HX711
    weight = Get_Weight();  //更新重量值
    // Serial.print(weight);   //顯示重量
    // Serial.println("g ");   //顯示單位
    // OLED
    oled_weight();
    // LoseWeight
    if (abs(weight - ideal_weight) > 70) {
        notice_status("/LoseWeight");                            // notice line
        Serial.println("重量異常減少，停車，直到重量恢復正常");  //error
        myDFPlayer.playLargeFolder(2, 1);                        // 0001食物重量異常減少.wav
        SerialBT.write('t');                                     //車輛停止
        long count_16s = millis();
        while (1) {
            weight = Get_Weight();  //更新重量值
            oled_weight();
            if ((millis() - count_16s) < 4150 && (millis() - count_16s) > 4000) {
                myDFPlayer.playLargeFolder(2, 2);  // 0002cool_alarm_sound.mp3
            }

            //收到mega板的藍芽資訊 for test
            if (weight > 1000) {
                state = 2;
                myDFPlayer.playLargeFolder(5, 1);  // 0001餐點已送達，您的餐點為.wav
                notice_status("/Arrived");         // notice line
                order_name = 0;
                return;
            }

            // 重量恢復正常
            if (abs(weight - ideal_weight) < 50) {
                Serial.println("車輛恢復行駛~~");
                myDFPlayer.playLargeFolder(2, 6);  // 0006dance_money_crop.wav
                SerialBT.write('2');
                return;
            }
        }
    }

    //收到mega板的藍芽資訊，代表移動任務完成
    if (SerialBT.available()) {
        char completed = SerialBT.read();
        printf("completed 1 : %c\n", completed);
        Serial.write(completed);
        if (completed == '1') {
            state = 2;
            myDFPlayer.playLargeFolder(5, 1);  // 0001餐點已送達，您的餐點為.wav
            notice_status("/Arrived");         // notice line
            order_name = 0;
            return;
        }
    }
}

void oled_weight() {
    u8g2.firstPage();
    do {
        // row 1 - state
        u8g2.setCursor(5, 10);
        u8g2.print("current weight (g)");
        // row 2 - wating time
        u8g2.setFont(u8g2_font_inr38_mn);  // font
        //alignment of LSB
        if (weight < 0)
            u8g2.setCursor(60, 55);
        else if (weight >= 0 && weight < 10)
            u8g2.setCursor(90, 55);
        else if (weight >= 10 && weight < 100)
            u8g2.setCursor(60, 55);
        else if (weight >= 100 && weight < 1000)
            u8g2.setCursor(30, 55);
        else
            u8g2.setCursor(0, 55);

        oled_string = (String)(weight);
        u8g2.print(oled_string.c_str());
        u8g2.setFont(u8g2_font_7x13B_mf);  // font
        // row 3 - progress bar
        u8g2.drawBox(0, 60, millis() / 100 % 127, 4);
    } while (u8g2.nextPage());
}

// state == 2
void arrive_announce() {
    // mp3
    // when music isn't playing
    if (myDFPlayer.readState() == 0 && order_name < 21) {
        if (order_name < 20 && order_name % 2 == 0 && queue.menu(order_name / 2) > 0) {
            myDFPlayer.playLargeFolder(3, order_name / 2 + 1);  // 餐點名稱
        } else if (order_name < 20 && order_name % 2 == 1 && queue.menu(order_name / 2) > 0) {
            if (queue.menu(order_name / 2) > 21) {
                myDFPlayer.playLargeFolder(4, 21);  // 餐點數量超過21份
            } else {
                myDFPlayer.playLargeFolder(4, queue.menu(order_name / 2));  // 餐點數量
            }
        } else if (order_name == 20) {
            myDFPlayer.playLargeFolder(5, 2);  // 0002請將房卡放置於感應處，確認交易.wav
        }
        order_name++;
    }

    oled_order_list();
    if (order_name == 21) {
        count_state3 = millis();
        state = 3;
        return;
    }
}

void oled_order_list() {
    u8g2.firstPage();
    do {
        // row 1 - state
        u8g2.setCursor(5, 10);
        u8g2.print("Order arrived");
        // row 2 - notice
        u8g2.setCursor(5, 30);
        u8g2.print("Confirm your order");
        // row 3 - notice
        u8g2.setCursor(5, 50);
        int non_zero_number = 0;
        for (int i = 0; i < 10; i++) {
            if (queue.menu(i) > 0) {
                non_zero_number++;
            }
        }
        oled_string = "You have " + (String)(non_zero_number) + " order";
        u8g2.print(oled_string.c_str());
        // row 4 - progress bar
        u8g2.drawBox(0, 60, millis() / 100 % 127, 4);
    } while (u8g2.nextPage());
}

// state == 3
void state_for_RFID() {
    // 20 second -> timeout
    if (millis() - count_state3 > 60000) {
        notice_status("/Timeout");         // notice line
        myDFPlayer.playLargeFolder(5, 4);  // 0004逾時未完成交易手續，車輛將離開，此筆訂單不成立.wav
        state = 5;
        SerialBT.write('3');
        SerialBT.write('2');  //車輛移動到起始位置
        return;
    }

    // 顧客沒逼卡，就拿走東西
    weight = Get_Weight();  //更新重量值
    if (weight < 20) {
        notice_status("/Accidient");               // notice line
        Serial.println("顧客沒逼卡，就拿走東西");  //error
        myDFPlayer.playLargeFolder(5, 8);          //0008請先完成付款手續.wav
        long count4s = millis();
        while (1) {
            //5秒後撥放警報聲
            if (myDFPlayer.readState() == 0 && abs(millis() - count4s) > 5000)
                myDFPlayer.playLargeFolder(5, 5);  // 0005alarm.mp3

            weight = Get_Weight();  //更新重量值
            oled_weight();
            // 重量恢復正常
            if (abs(weight - ideal_weight) < 50) {
                myDFPlayer.pause();                //pause the mp3
                myDFPlayer.playLargeFolder(5, 2);  // 0002請將房卡放置於感應處，確認交易.wav
                return;
            }
        }
    }

    // OLED
    if (!rfid_err)
        oled_rfid_init();
    else
        oled_rfid_err();

    // RFID
    // 檢查是不是偵測到新的卡
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        // 顯示卡片的UID
        Serial.print("Card UID:");
        dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);  // 顯示卡片的UID
        Serial.println();
        Serial.print("PICC type: ");
        MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
        Serial.println(mfrc522.PICC_GetTypeName(piccType));  //顯示卡片的類型

        // 從queue中抓取目前的UID
        byte uid[] = {queue.uid(0), queue.uid(1), queue.uid(2), queue.uid(3)};

        //把取得的UID，拿來比對我們指定好的UID
        bool they_match = true;        // 初始值是假設為真
        for (int i = 0; i < 4; i++) {  // 卡片UID為4段，分別做比對
            if (uid[i] != mfrc522.uid.uidByte[i]) {
                they_match = false;  // 如果任何一個比對不正確，they_match就為false，然後就結束比對
                break;
            }
        }

        //在監控視窗中顯示比對的結果
        if (they_match) {
            Serial.println("Access Granted!");
            Serial.println("--------------");
            myDFPlayer.playLargeFolder(5, 3);  // 0003請拿取餐點，祝您用餐愉快!.wav
            SerialBT.write('3');               //傳3，開蓋
            state = 4;
            return;
        } else {
            Serial.println("Access Denied!");
            Serial.println("--------------");
            rfid_err++;
            myDFPlayer.playLargeFolder(5, 7);  // 0007請使用正確的房卡感應.wav
            notice_status("/Rfid");            // notice line
        }
        mfrc522.PICC_HaltA();  // 卡片進入停止模式
    }

    // RFID錯誤超過3次
    if (rfid_err == 3) {
        myDFPlayer.playLargeFolder(5, 6);  // 0006RFID錯誤超過三次，車輛將離開，此筆訂單不成立
        state = 5;
        SerialBT.write('3');  //車輛移動到起始位置
        SerialBT.write('2');  //車輛移動到起始位置
        return;
    }
}

// 把讀取到的UID，用16進位顯示出來
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void oled_rfid_init() {
    u8g2.firstPage();
    do {
        // row 1 - state
        u8g2.setCursor(5, 10);
        u8g2.print("Order arrived");
        // row 2 - notice
        u8g2.setCursor(5, 30);
        u8g2.print("Hold Your Room card");
        // row 3 - notice
        u8g2.setCursor(5, 50);
        u8g2.print("against there");
        // row 4 - progress bar
        u8g2.drawBox(0, 60, millis() / 100 % 127, 4);
    } while (u8g2.nextPage());
}

void oled_rfid_err() {
    u8g2.firstPage();
    do {
        // row 1 - state
        u8g2.setCursor(5, 10);
        u8g2.print("Your order arrived");
        // row 2 - notice
        u8g2.setCursor(5, 30);
        u8g2.print("You got the wrong card");
        // row 3 - error
        oled_string = "error count : " + (String)(rfid_err);
        u8g2.drawStr(10, 50, oled_string.c_str());
        // row 4 - progress barweight
        u8g2.drawBox(0, 60, millis() / 100 % 127, 4);
    } while (u8g2.nextPage());
}

// state == 4
void state_for_get_order() {
    oled_get_order();
    weight = Get_Weight();
    // 20 second -> timeout
    if (millis() - count_state3 > 60000) {
        notice_status("/Timeout");         // notice line
        myDFPlayer.playLargeFolder(5, 4);  // 0004逾時未完成交易手續，車輛將離開，此筆訂單不成立.wav
        state = 5;
        SerialBT.write('3');
        SerialBT.write('2');  //車輛移動到起始位置
        return;
    }

    if (weight < 20) {
        notice_status("/Completed");  // notice line
        state = 5;
        SerialBT.write('2');  //車輛移動到起始位置
        return;
    }
}

void oled_get_order() {
    u8g2.firstPage();
    do {
        // row 1 - state
        u8g2.setCursor(5, 10);
        u8g2.print("Enjoy your food");
        // row 2 - notice
        u8g2.setCursor(5, 30);
        u8g2.print("Have a nice day");
        // row 3 - progress bar
        u8g2.drawBox(0, 60, millis() / 100 % 127, 4);
    } while (u8g2.nextPage());
}

// state == 5
void state_go_back() {
    // when music isn't playing
    if (myDFPlayer.readState() == 0) {
        myDFPlayer.playLargeFolder(2, 9);  // 0009uruha_rushia_bgm.mp3
    }

    oled_go_back();
    //收到mega板的藍芽資訊 for test
    weight = Get_Weight();
    if (weight > 1500) {
        state = 0;
        myDFPlayer.pause();                //pause the mp3
        myDFPlayer.playLargeFolder(5, 9);  // 0009任務完成，繼續下一筆訂單.wav
        queue.delete_node();
        return;
    }

    //收到mega板的藍芽資訊，代表移動任務完成
    if (SerialBT.available()) {
        char completed = SerialBT.read();
        printf("completed 2 : %c\n", completed);
        Serial.write(completed);
        if (completed == '1') {
            state = 0;
            myDFPlayer.pause();                //pause the mp3
            myDFPlayer.playLargeFolder(5, 9);  // 0009任務完成，繼續下一筆訂單.wav
            queue.delete_node();
            return;
        }
    }
}

void oled_go_back() {
    u8g2.firstPage();
    do {
        // row 1 - status
        u8g2.setCursor(30, 10);
        u8g2.print("go back~~");
        u8g2.drawBox(millis() / 25 % 167 - 40, 30, 40, 15);
        u8g2.drawDisc(millis() / 25 % 167 - 8, 50, 5, U8G2_DRAW_ALL);
        u8g2.drawDisc(millis() / 25 % 167 - 33, 50, 5, U8G2_DRAW_ALL);
    } while (u8g2.nextPage());
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
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
        if (payload[0] == '1') {
            return true;
        } else {
            return false;
        }
    }
}

void update_order() {
    String url = domain + order_api;
    String payload = "";

    if ((WiFi.status() == WL_CONNECTED)) {
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
            payload = http.getString();
            //             Serial.printf("回應本體：%s\n", payload.c_str());
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }

    // 解析json
    int menu_latest[10];
    int UID_latest[4];
    DynamicJsonDocument doc(800);
    deserializeJson(doc, payload);
    for (int i = 0; i < 10; i++)
        menu_latest[i] = (int)doc["menu"][i];
    String temp = doc["room"];
    char room_number_latest = temp[0];
    //  customer_name = doc["name"];
    for (int i = 0; i < 4; i++)
        UID_latest[i] = (int)doc["rfid"][i];

    printf("--------------\nwe get new order!\n");
    for (int i = 0; i < 10; i++)
        printf("menu[%d] : %d\n", i, menu_latest[i]);
    printf("room_number : %c\n", room_number_latest);
    for (int i = 0; i < 4; i++)
        printf("UID[%d] : %d\n", i, UID_latest[i]);
    printf("--------------\n");
    queue.put(menu_latest, room_number_latest, UID_latest);  // pass by ref

    myDFPlayer.playLargeFolder(1, 3);  // 0003收到新訂單.wav
    // buzzer to gettig one order
    printf("buzzer notice\n");
    for (int i = 0; i < 10; i++) {
        ledcWriteTone(0, 1000);
        delay(50);
        ledcWriteTone(0, 2000);
        delay(50);
    }
    ledcWriteTone(0, 0);
}

void notice_status(String API) {
    String url = domain + API;
    printf("send notice to \"%s\" \n", url.c_str());
    if ((WiFi.status() == WL_CONNECTED)) {
        http.begin(url);
        int httpCode = http.GET();
        if (httpCode > 0) {
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
    }
}
