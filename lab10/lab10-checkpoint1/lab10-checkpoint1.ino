#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); // RS,E,D4,D5,D6,D7
const String STUDENT_ID = "6710110428"; 
int id_length = STUDENT_ID.length(); 

void setup() {
  lcd.begin(16, 2);
}

void loop() {
  
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print(STUDENT_ID);
  delay(1000); 

  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayRight();
    delay(200);
  }

  
  lcd.clear(); 
  

  lcd.setCursor(40 - id_length, 1); 
  lcd.print(STUDENT_ID);

  // เลื่อนทั้งหมด 26 ครั้ง
  // - 10 ครั้งแรก: ข้อความจะค่อยๆ ทะลุจากขอบซ้ายโผล่มาทีละตัว 
  // - 16 ครั้งหลัง: เลื่อนต่อไปทางขวาจนหลุดกรอบจอไปจนหมด
  for (int i = 0; i < (id_length + 16); i++) {
    lcd.scrollDisplayRight();
    delay(200);
  }
  
  lcd.clear(); 
  lcd.setCursor(40 - id_length, 0); 
  lcd.print(STUDENT_ID);
  for (int i = 0; i < id_length; i++) {
    lcd.scrollDisplayRight();
    delay(200);
  }
}
