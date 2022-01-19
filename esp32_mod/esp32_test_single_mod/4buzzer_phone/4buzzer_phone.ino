#define BITS 10
#define BUZZER_PIN 4  // 蜂鳴器接在腳4

void alarmSnd() {
    for (int i = 0; i < 10; i++) {
        ledcWriteTone(0, 1000);
        delay(50);
        ledcWriteTone(0, 2000);
        delay(50);
    }
    ledcWriteTone(0, 0);
    delay(2000);
}

void setup() {
    ledcSetup(0, 2000, BITS);  // PWM預設為20KHz，10位元解析度。
    ledcAttachPin(BUZZER_PIN, 0);
}

void loop() {
    alarmSnd();
}
