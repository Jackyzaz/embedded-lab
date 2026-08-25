int soundPin = 2;
int relayPin = 5;
unsigned long previous_time=0;

int val;
int state;
int num_clap; // จำนวนครั้ง สะสม
unsigned long last_clap_time = 0;
uint8_t clap_count = 0; // จำนวนครั้ง ณ ช่วงเวลา

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(soundPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  val = check_clap();
  if (val) {
    last_clap_time = millis();
    clap_count ++;
    
    Serial.print("Clap detected!!! #");
    Serial.println(num_clap);
    num_clap++;

    if (state == 1 && clap_count == 3) {
      clap_count = 0;
      state = 0;
      Serial.println("  LED is turned on");
      digitalWrite(relayPin, HIGH);
    } else if (state == 0 && clap_count == 2){
      clap_count = 0;
      state = 1;
      Serial.println("  LED is turned off");
      digitalWrite(relayPin, LOW);
    }
  }
  // reset จำนวนคัร้้งภายใน 1.5 sec
  if ((clap_count > 0) && 
      (millis() - last_clap_time) > 1500) {
    clap_count = 0;
  }
}


bool check_clap() {
  static unsigned long current_time;
  char sw;
  sw = digitalRead(soundPin);
  // 0 = clap
  // 1 = no clap
  if (sw) {
    return false;
  } else {
    current_time = millis();
    if ((current_time-previous_time) > 25) {
      previous_time = millis();
      return true;
    } else {
      previous_time = millis();
      return false;
    }
  }
}
