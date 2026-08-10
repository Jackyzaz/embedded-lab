#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT11 // DHT11
//#define DHTTYPE DHT22 // DHT22
//#define DHTTYPE DHT21 // DHT21

DHT dht(DHTPIN, DHTTYPE); // สั่งใช้มอดูล DHT11, pin2  arduino

void setup() {
  Serial.begin(9600);
  Serial.println("DHT22 test!");
  dht.begin();
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
  Serial.print(" *C \t");

  Serial.print(f);
  Serial.println(" *F\t");
  
}
