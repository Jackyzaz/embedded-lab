#include <LiquidCrystal.h>

LiquidCrystal lcd1(8, 9, 10, 11, 12, 13); //RS, E, D4, D5, D6, D7

void setup() {
  lcd1.begin(16,2);
  lcd1.clear();
}

void loop() {
  lcd1.setCursor(0,0);
  lcd1.print("  Hello TEST!!!");
  lcd1.setCursor(0,1);
  lcd1.print("Department of Computer Engineering");
  delay(2000);
  for (int i=0; i<18; i++) {
    lcd1.scrollDisplayLeft();
    delay(200);
  }

  delay(2000);
  for (int i=0; i<18; i++) {
    lcd1.scrollDisplayRight();
    delay(300);
  }
  delay(2000);
  for (int i=0; i<3; i++) {
    lcd1.noDisplay();
    delay(500); 
  }
  delay(3000);
  lcd1.clear();

}
