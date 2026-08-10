#include <Keypad.h>


const int segmentA = 14;
const int segmentB = 15;
const int segmentC = 16;
const int segmentD = 17;
const int segmentE = 18;
const int segmentF = 19;
const int segmentG = 11;
const int digit1 = 10;
const int ROW_NUM = 4; //four rows
const int COLUMN_NUM = 4; //four columns
char keys[ROW_NUM][COLUMN_NUM]={     
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'E','0','F','D'}
};
byte pin_rows[ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( (char *)keys, 
                 pin_rows, pin_column, 
                 ROW_NUM, COLUMN_NUM );

char key;

void display_0() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, LOW);
}

void display_1() {
  digitalWrite(segmentA, LOW);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, LOW);
  digitalWrite(segmentE, LOW);
  digitalWrite(segmentF, LOW);
  digitalWrite(segmentG, LOW);
}

void display_2() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, LOW);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, LOW);
  digitalWrite(segmentG, HIGH);
}
void display_3() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, LOW);
  digitalWrite(segmentF, LOW);
  digitalWrite(segmentG, HIGH);
}
void display_4() {
  digitalWrite(segmentA, LOW);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, LOW);
  digitalWrite(segmentE, LOW);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_5() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, LOW);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, LOW);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_6() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, LOW);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_7() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, LOW);
  digitalWrite(segmentE, LOW);
  digitalWrite(segmentF, LOW);
  digitalWrite(segmentG, LOW);
}
void display_8() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_9() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, LOW);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_a() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, LOW);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_b() {
  digitalWrite(segmentA, LOW);
  digitalWrite(segmentB, LOW);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_c() {
  digitalWrite(segmentA, LOW);
  digitalWrite(segmentB, LOW);
  digitalWrite(segmentC, LOW);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, LOW);
  digitalWrite(segmentG, HIGH);
}
void display_d() {
  digitalWrite(segmentA, LOW);
  digitalWrite(segmentB, HIGH);
  digitalWrite(segmentC, HIGH);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, LOW);
  digitalWrite(segmentG, HIGH);
}
void display_e() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, LOW);
  digitalWrite(segmentC, LOW);
  digitalWrite(segmentD, HIGH);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display_f() {
  digitalWrite(segmentA, HIGH);
  digitalWrite(segmentB, LOW);
  digitalWrite(segmentC, LOW);
  digitalWrite(segmentD, LOW);
  digitalWrite(segmentE, HIGH);
  digitalWrite(segmentF, HIGH);
  digitalWrite(segmentG, HIGH);
}
void display7Segment(char value) { // เปลี่ยนตรงนี้เป็น char
  switch(value) {
    case '0': display_0(); break;
    case '1': display_1(); break;
    case '2': display_2(); break;
    case '3': display_3(); break;
    case '4': display_4(); break;
    case '5': display_5(); break;
    case '6': display_6(); break;
    case '7': display_7(); break;
    case '8': display_8(); break;
    case '9': display_9(); break;
    case 'A': display_a(); break;
    case 'B': display_b(); break;
    case 'C': display_c(); break;
    case 'D': display_d(); break;
    case 'E': display_e(); break;
    case 'F': display_f(); break;
  }
}


void setup() {
  // put your setup code here, to run once: 
  for (int i = segmentA; i<=segmentF; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(segmentG, OUTPUT);

  // setup keypad
  for (int c = 2; c<=5; c++) {
    pinMode(c, INPUT_PULLUP);
  }
  pinMode(digit1, OUTPUT);
  digitalWrite(digit1, HIGH);
  display7Segment('0');
  Serial.begin(9600);
}


void loop() {
  if ( (key=keypad.getKey()) != NO_KEY ) { // get key
    Serial.print("Value: ");
    Serial.println(key);
    display7Segment(key);
  } 
}
