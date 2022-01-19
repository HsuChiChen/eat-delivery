

// 馬達控制設定
const byte in1 = 27;  //In1
const byte in2 = 26;  //In2
const byte ENA = 12;  //ENA


void open_lid(){
    //control front
	digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
	//PWM
	ledcWrite(0, 230);
	Serial.println("open");
	delay(240);
	stop_lid();
}
void close_lid(){
    //control front
	digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
	//PWM
	ledcWrite(0, 230);
	Serial.println("close");
		
	delay(140);
	stop_lid();
}
void stop_lid(){
    //control front
	digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
	//PWM
	ledcWrite(0, 0);
  
}

void setup(){	
	pinMode(in1, OUTPUT);
    pinMode(in2, OUTPUT);
    pinMode(ENA, OUTPUT);
	ledcSetup(0, 5000, 8);
	ledcAttachPin(ENA, 0);
  Serial.begin(9600);
  Serial.println("start");
}

void loop(){
	open_lid();
	
	delay(3000);
	
	close_lid();

	delay(3000);
}
