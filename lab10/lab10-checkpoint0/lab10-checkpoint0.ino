#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); // RS,E,D4,D5,D6,D7
void setup() {
  lcd.begin(16, 2); // 16 chars on 2 lines
  lcd.clear();
}
void loop() {
  lcd.setCursor(0, 0);
  lcd.print(" Hello World !");
  lcd.setCursor(1, 0);
  lcd.print(" I love CoE.");
  delay(2000);
  for (int i = 0; i < 18; i++) {
    lcd.scrollDisplayLeft();
    delay(200);
  }
  delay(2000);
  for (int i = 0; i < 18; i++) {
    lcd.scrollDisplayRight();
    delay(200);
  }
  delay(2000);
  for (int i = 0; i < 3; i++) {
    lcd.noDisplay();
    delay(500);
    lcd.display();
    delay(500);
  }
  delay(3000);
  lcd.clear();
}
