#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 10, 11, 12, 13); // RS,E,D4,D5,D6,D7
byte c[8] = {
  0b00000,
  0b11111,
  0b11111,
  0b11000,
  0b11000,
  0b11000,
  0b11111,
  0b00000
};

byte o[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11001,
  0b11001,
  0b11111,
  0b00000
};

byte e[8] = {
  0b00000,
  0b11111,
  0b11111,
  0b11000,
  0b11111,
  0b11000,
  0b11111,
  0b00000
};

byte arrow[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b00010,
  0b11111,
  0b00010,
  0b00100,
  0b00000
};

byte star[8] = {
  0b00000,
  0b00100,
  0b00100,
  0b01110,
  0b11111,
  0b01110,
  0b00100,
  0b00100
};

byte n[8] = {
  0b00000,
  0b10001,
  0b11001,
  0b11101,
  0b11111,
  0b11011,
  0b11001,
  0b00000
};

byte freee[8] = {
  0b10001,
  0b11011,
  0b01010,
  0b11111,
  0b11111,
  0b01010,
  0b11011,
  0b10001
};


void setup() {
  lcd.begin(16, 2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display
  lcd.createChar(0, c); // Create a custom character
  lcd.createChar(1, o);
  lcd.createChar(2, e);
  lcd.createChar(3, arrow);
  lcd.createChar(4, star);
  lcd.createChar(5, n);
  lcd.createChar(6, freee);



  // Clears the LCD screen
  lcd.clear();

  // Print a message to the LCD
  lcd.print("Custom Character");
}

void loop() {
  lcd.setCursor(1, 1);
  lcd.write(byte(0)); // Display the custom character 0, the heart

  lcd.setCursor(2, 1);
  lcd.write(byte(1));

  lcd.setCursor(3, 1);
  lcd.write(byte(2));

  lcd.setCursor(5, 1);
  lcd.write(byte(3));

  lcd.setCursor(7, 1);
  lcd.write(byte(4));

  lcd.setCursor(9, 1);
  lcd.write(byte(5));

  lcd.setCursor(11, 1);
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(6));
  lcd.write(byte(6));


}
