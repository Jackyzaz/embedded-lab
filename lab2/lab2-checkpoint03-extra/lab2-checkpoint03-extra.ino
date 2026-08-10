const int segmentA = 14;
const int segmentB = 15;
const int segmentC = 16;
const int segmentD = 17;
const int segmentE = 18;
const int segmentF = 19;
const int segmentG = 11;

const int c1 = 5;
const int c2 = 4;
const int c3 = 3;
const int c4 = 2;

const int r1 = 9;
const int r2 = 8;
const int r3 = 7;
const int r4 = 6;

char keys[4][4] = {
  {1,2,3,10},
  {4,5,6,11},
  {7,8,9,12},
  {14,0,15,13}
};

const int digit1 = 10;

int temp;
int current;

// --- ฟังก์ชันแสดงผล 7-Segment (เหมือนเดิมของคุณ) ---
void display_0() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, HIGH); digitalWrite(segmentG, LOW); }
void display_1() { digitalWrite(segmentA, LOW);  digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, LOW);  digitalWrite(segmentE, LOW);  digitalWrite(segmentF, LOW);  digitalWrite(segmentG, LOW); }
void display_2() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, HIGH); digitalWrite(segmentC, LOW);  digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, LOW);  digitalWrite(segmentG, HIGH); }
void display_3() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, LOW);  digitalWrite(segmentF, LOW);  digitalWrite(segmentG, HIGH); }
void display_4() { digitalWrite(segmentA, LOW);  digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, LOW);  digitalWrite(segmentE, LOW);  digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_5() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, LOW);  digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, LOW);  digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_6() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, LOW);  digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_7() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, LOW);  digitalWrite(segmentE, LOW);  digitalWrite(segmentF, LOW);  digitalWrite(segmentG, LOW); }
void display_8() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_9() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, LOW);  digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_a() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, LOW);  digitalWrite(segmentE, HIGH); digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_b() { digitalWrite(segmentA, LOW);  digitalWrite(segmentB, LOW);  digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_c() { digitalWrite(segmentA, LOW);  digitalWrite(segmentB, LOW);  digitalWrite(segmentC, LOW);  digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, LOW);  digitalWrite(segmentG, HIGH); }
void display_d() { digitalWrite(segmentA, LOW);  digitalWrite(segmentB, HIGH); digitalWrite(segmentC, HIGH); digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, LOW);  digitalWrite(segmentG, HIGH); }
void display_e() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, LOW);  digitalWrite(segmentC, LOW);  digitalWrite(segmentD, HIGH); digitalWrite(segmentE, HIGH); digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }
void display_f() { digitalWrite(segmentA, HIGH); digitalWrite(segmentB, LOW);  digitalWrite(segmentC, LOW);  digitalWrite(segmentD, LOW);  digitalWrite(segmentE, HIGH); digitalWrite(segmentF, HIGH); digitalWrite(segmentG, HIGH); }

void display7Segment(int value) {
  switch(value) {
    case 0: display_0(); break;
    case 1: display_1(); break;
    case 2: display_2(); break;
    case 3: display_3(); break;
    case 4: display_4(); break;
    case 5: display_5(); break;
    case 6: display_6(); break;
    case 7: display_7(); break;
    case 8: display_8(); break;
    case 9: display_9(); break;
    case 10: display_a(); break;
    case 11: display_b(); break;
    case 12: display_c(); break;
    case 13: display_d(); break;
    case 14: display_e(); break;
    case 15: display_f(); break;
  }
}

// ปรับปรุงฟังก์ชันการสแกนให้ถูกต้องและเสถียร
void scanKey() {
  int rowPins[4] = {r1, r2, r3, r4}; // ขา 4, 5, 6, 7
  int colPins[4] = {c1, c2, c3, c4}; // ขา 0, 1, 2, 3

  for (int r = 0; r < 4; r++) {
    // ปล่อยไฟ LOW เฉพาะแถวที่เลือกตรวจจับ
    digitalWrite(rowPins[r], LOW);

    for (int c = 0; c < 4; c++) {
      // ถ้ากดปุ่ม สัญญาณที่ขาคอลัมน์จะโดนดึงลง LOW (เพราะต่อกับแถวที่เป็น LOW)
      if (digitalRead(colPins[c]) == LOW) {
        current = keys[r][c ];
        display7Segment(current);
        delay(200); // ใส่ดีเลย์สั้นๆ กันปุ่มเด้งเบิ้ล (Debounce)
      }
    }
    // คืนค่าแถวให้เป็น HIGH เพื่อเตรียมสแกนแถวถัดไป
    digitalWrite(rowPins[r], HIGH);
  }
}

void setup() {
  // ตั้งค่าขา 7-Segment ตัวแปรแยกทีละตัวเพื่อความชัวร์
  pinMode(segmentA, OUTPUT);
  pinMode(segmentB, OUTPUT);
  pinMode(segmentC, OUTPUT);
  pinMode(segmentD, OUTPUT);
  pinMode(segmentE, OUTPUT);
  pinMode(segmentF, OUTPUT);
  pinMode(segmentG, OUTPUT);

  // ตั้งค่าขาแถว Keypad (เริ่มต้นให้เป็น HIGH)
  pinMode(r1, OUTPUT); digitalWrite(r1, HIGH);
  pinMode(r2, OUTPUT); digitalWrite(r2, HIGH);
  pinMode(r3, OUTPUT); digitalWrite(r3, HIGH);
  pinMode(r4, OUTPUT); digitalWrite(r4, HIGH);

  // ตั้งค่าขาคอลัมน์ Keypad ให้เป็น INPUT_PULLUP
  pinMode(c1, INPUT_PULLUP);
  pinMode(c2, INPUT_PULLUP);
  pinMode(c3, INPUT_PULLUP);
  pinMode(c4, INPUT_PULLUP);
  
  pinMode(digit1, OUTPUT);
  digitalWrite(digit1, HIGH); // สั่งเปิด Common (สมมติว่าเป็น Common Cathode หรือ Anode ตามวงจรเดิม)
  
  display7Segment(0); // แสดงเลข 0 รอไว้
}

void loop() {
  scanKey();
}
