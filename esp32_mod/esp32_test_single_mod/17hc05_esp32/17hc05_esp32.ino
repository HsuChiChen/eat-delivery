HardwareSerial BT(1);
char val;  // 儲存接收資料的變數

void setup() {
    Serial.begin(9600);
    BT.begin(38400, SERIAL_8N1, 27, 13);  // Serial的TX,RX
    Serial.println("hello world");
}

void loop() {
    if (Serial.available()) {
        val = Serial.read();
        BT.print(val);
    }

    // 若收到藍牙模組的資料，則送到「序列埠監控視窗」
    if (BT.available()) {
        val = BT.read();
        Serial.print(val);
    }
}
