#include "DHT.h"
#include <LiquidCrystal.h>

#define DHTPIN 2
#define DHTTYPE DHT11 // DHT11

LiquidCrystal lcd1(8, 9, 10, 11, 12, 13); //RS, E, D4, D5, D6, D7

DHT dht(DHTPIN, DHTTYPE); // สั่งใช้มอดูล DHT11, pin2 arduino

void setup() {
  Serial.begin(9600); // เปิดใช้งาน serial monitor
  Serial.println("DHT11 test!");
  dht.begin(); // สั่งเปิดใช้งาน dht
  lcd1.begin(16,2); // สั่งเปิดใช้ lcd
  lcd1.clear();
}

void loop() {
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // ใส่ true เพื่อให้ได้องศา F
  float f = dht.readTemperature(true); 
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: "); // แสดงความชื้น
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: "); // แสดงค่าอุณภูม
  Serial.print(t); // แบบองศาสเซลเซียส
  Serial.print(" *C ");
  Serial.print(f); // แบบองศาฟาเรนไฮต์
  Serial.println(" *F"); 
  lcd1.setCursor(0, 0); // บรรทัดบน
  lcd1.print("Hum: "); // ความชื้น
  lcd1.print(h, 1);    // ทศนิยม 1 ตำแหน่ง
  lcd1.print(" %");
  lcd1.setCursor(0, 1); // บรรทัดล่าง
  lcd1.print("Temp:"); // ย่อคำเพื่อไม่ให้ล้น
  lcd1.print(t, 1);    // ทศนิยม 1 ตำแหน่ง
  lcd1.print("C ");
  lcd1.print(f, 1);    // ทศนิยม 1 ตำแหน่ง
  lcd1.print("F");
}
