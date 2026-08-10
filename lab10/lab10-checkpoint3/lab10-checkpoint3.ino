#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); // RS,E,D4,D5,D6,D7


#define echoPin 3
#define trigPin 2

int maximumRange = 200;
int minimumRange = 0;
long duration;
float distance;
String range = "Out of Range"; 

String getRange(long duration) {
  if (distance > 200)    return "Out of Range    ";
  else if (distance > 5) return "Range OK        ";
  else                   return "TOO CLOSE !     ";
}

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear(); 
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // measure pulse width ระยะเวลาที่ pulse เปลี่ยนจาก low -> high ในหน่วย us
  duration = pulseIn(echoPin, HIGH); 
  


  // sound speed = 343 m/s
  // or 0.0343 cm/us
  // S = vt
  // 2distance = duration x 0.0343
  // distance = duration x 0.0343 / 2
  // distance = duration x 0.01715
  // 1/0.01715 = 58.2
  distance = (float)duration / 58.2;
  range = getRange(distance);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm"); 

  lcd.setCursor(0, 1);
  lcd.print(range);

  delay(250);
}
