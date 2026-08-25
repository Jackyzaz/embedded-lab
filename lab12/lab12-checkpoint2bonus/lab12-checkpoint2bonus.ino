int soundPin = 2; // int 0
int relayPin = 5;

volatile int state = 0;
volatile int num_clap = 0;
volatile unsigned long last_clap_time = 0;
volatile uint8_t clap_count = 0;

int prev_num_clap = 0;
int prev_state = 0;

ISR(INT0_vect) {
  unsigned long now = millis();
  if ((now-last_clap_time) > 25) {
    last_clap_time = now;
    clap_count++; // จำนวนครั้งที่ติดต่อกัน
    num_clap++; // จำนวนครั้งสะสม
    if (state == 1 && clap_count == 3) {
      clap_count = 0;
      state = 0;
      digitalWrite(relayPin, LOW);
    } else if (state == 0 && clap_count == 2) {
      clap_count = 0;
      state = 1;
      digitalWrite(relayPin, HIGH);
    }
  }  
}

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(soundPin, INPUT);
  digitalWrite(relayPin, LOW);
  Serial.begin(9600);
  Serial.println("num_clap = 0");
  EIMSK = 0x01; // enable int0
  EICRA = 0x02; // falling edge
  sei();
}

void loop() {
  // แสดงผลจำนวนประมือสะสม
  if (num_clap != prev_num_clap) {
    Serial.print("Clap detected!!! #");
    Serial.println(num_clap);
    prev_num_clap = num_clap; 
  } 
  // แสดงสถานะ LED เมื่อเปลี่ยน state
  if (state != prev_state) {
    if (state == 1) {
      Serial.println("  --> LED is turned OFF");
    } else {
      Serial.println("  --> LED is turned ON");
    }
    prev_state = state;
  } 
  // reset clap count
  if ((clap_count > 0) && 
  (millis() - last_clap_time > 1500)) {
    clap_count = 0;
  }
}
