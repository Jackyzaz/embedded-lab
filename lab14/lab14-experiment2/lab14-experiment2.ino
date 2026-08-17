#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);  //Module IIC/I2C Interface บางรุFนอาจจะใชN 0x3f

void setup()
{
 lcd.init();
 lcd.backlight(); // เปpด backlight
 lcd.home();
 lcd.print("Hello, world!");
 lcd.setCursor(0, 1);
 lcd.print("CoE PSU");
}

void loop()
{
} 
