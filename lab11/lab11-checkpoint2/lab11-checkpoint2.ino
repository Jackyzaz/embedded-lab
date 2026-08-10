const int strobe = 7;
const int clock = 9;
const int data = 8;

// ตัวแปรควบคุมสถานะการทำงาน
uint8_t mode = 0;
uint8_t lastMode = 0;
// ข้อผิดพลาด - เคยใช้ uint8 แล้วไฟวิ่งไม่ถูกต้องเนื่องจาก overflow
// ใช้ int8_t (ค่าติดลบได้) เพื่อลดความซับซ้อนเวลาไฟวิ่งถอยหลัง
int8_t currentLed = 0;        
unsigned long lastUpdate = 0;  

uint8_t lastButtons = 0;       
int8_t activeLed = 0;

void setup()
{
  pinMode(strobe, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);

  sendCommand(0x8f);  
  reset();
}

void loop()
{
  uint8_t buttons = readTM1638_Buttons();
  
  // 1. เปลี่ยนโหมดตามปุ่มที่ถูกกด
  if (buttons & 0x01) mode = 0;
  else if (buttons & 0x80) mode = 1;
  else if (buttons) mode = 2;

  // 2. หากมีการเปลี่ยนโหมด ให้รีเซ็ตค่าเริ่มต้นใหม่
  if (mode != lastMode) {
    reset();
    if (mode==1) {
      currentLed = 7;
    } else {
      currentLed = 0;
    }
    lastUpdate = millis();
    lastMode = mode;
  }

  // 3. ประมวลผลแอนิเมชันแต่ละโหมด
  // ข้อผิดพลาด - เคยใช้ delay(100) แล้วทำให้ปุ่มตอบสนองช้าลง
  //            เพราะติด blocking จึงเลือกใช้ millis แทน 
  if (mode == 0) {
    if (millis() - lastUpdate >= 100) {
      if (currentLed > 7) {
        delay(500); // ค้างไว้ 0.5 วินาที ก่อนเริ่มใหม่ 
        reset();
        currentLed = 0;
      } else {
        setTM1638LED(1, currentLed);
        currentLed++;
      }
      lastUpdate = millis();
    }
  }
  else if (mode == 1) {
    if (millis() - lastUpdate >= 100) {
      if (currentLed < 0) {
        delay(500); // ค้างไว้ 0.5 วินาที ก่อนเริ่มใหม่
        reset();
        currentLed = 7;
      } else {
        setTM1638LED(1, currentLed);
        currentLed--;
      }
      lastUpdate = millis();
    }
  }
  else if (mode == 2) {
    // ตรวจจับจังหวะเริ่มกดปุ่ม (Rising Edge)
    uint8_t pressed = buttons & ~lastButtons; 
    
    if (pressed & 0x7E) { // หากมีการกดปุ่มดวงที่ 2 ถึง 7
      // วนลูปหาว่าปุ่มไหนถูกกด
      for (int i = 1; i < 7; i++) {
        if ((pressed >> i) & 1) activeLed = i; 
      }
      // อัปเดตไฟ LED ทั้ง 8 ดวงรวดเดียวด้วยลูปเดียวสั้นๆ (ดวงที่เลือก=ติด ดวงอื่น=ดับ)
      for (int i = 0; i < 8; i++) {
        if (i==activeLed) setTM1638LED(1, i); 
      }
    }
  }
  
  lastButtons = buttons;
}

void sendCommand(uint8_t value)
{
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}

void reset()
{
  sendCommand(0x40); // set auto increment mode
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xc0);   // set starting address to 0
  for(uint8_t i = 0; i < 16; i++)
  {
    shiftOut(data, clock, LSBFIRST, 0x00);
  }
  digitalWrite(strobe, HIGH);
}

uint8_t readTM1638_Buttons(void)
{
  uint8_t buttons = 0;
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0x42);

  pinMode(data, INPUT);

  for (uint8_t i = 0; i < 4; i++)
  {
    uint8_t v = shiftIn(data, clock, LSBFIRST) << i;
    buttons |= v;
  }

  pinMode(data, OUTPUT);
  digitalWrite(strobe, HIGH);
  return buttons;
}

void setTM1638LED(uint8_t value, uint8_t position)
{
  pinMode(data, OUTPUT);
  sendCommand(0x44); 
  digitalWrite(strobe, LOW);
  shiftOut(data, clock, LSBFIRST, 0xC1 + (position << 1));
  shiftOut(data, clock, LSBFIRST, value);
  digitalWrite(strobe, HIGH);
}
