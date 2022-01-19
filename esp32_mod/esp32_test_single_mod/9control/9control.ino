void setup() {
    Serial.begin(9600);
}

void loop() {
    int vrx, vry, sw;
    sw = analogRead(36);
    vrx = analogRead(39);
    vry = analogRead(34);

    char buf[100];
    sprintf(buf, "VRx=%d, VRy=%d, SW=%d", vrx, vry, sw);
    Serial.println(buf);
    delay(100);
}
