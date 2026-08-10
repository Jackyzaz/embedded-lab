#include "DHT.h"
#include <LiquidCrystal.h>

#define DHTPIN 2
#define DHTTYPE DHT11 // DHT11
//#define DHTTYPE DHT22 // DHT22
//#define DHTTYPE DHT21 // DHT21

LiquidCrystal lcd1(8, 9, 10, 11, 12, 13); //RS, E, D4, D5, D6, D7

DHT dht(DHTPIN, DHTTYPE); // สั่งใช้มอดูล DHT11, pin2 arduino

void setup() {
  Serial.begin(9600);
  Serial.println("DHT11 test!");
  dht.begin();

  lcd1.begin(16,2);
  lcd1.clear();
}

void loop() {
  delay(2000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");

  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");

  Serial.print(f);
  Serial.println(" *F"); 

  lcd1.setCursor(0, 0);
  lcd1.print("Hum: "); 
  lcd1.print(h, 1);    
  lcd1.print(" %");
  
  lcd1.setCursor(0, 1);
  lcd1.print("Temp:"); // ย่อคำเพื่อไม่ให้ล้น
  lcd1.print(t, 1);    // ทศนิยม 1 ตำแหน่ง
  lcd1.print("C ");
  lcd1.print(f, 1);    // ทศนิยม 1 ตำแหน่ง
  lcd1.print("F");
}
