const int segmentA = 14;
const int segmentB = 15;
const int segmentC = 16;
const int segmentD = 17;
const int segmentE = 18;
const int segmentF = 19;
const int segmentG = 8;

const int digit1 = 9;
const int digit2 = 10;
const int digit3 = 11;
const int digit4 = 12;

const int sw0 = 0;
const int sw1 = 1;
const int sw2 = 2;
const int sw3 = 3;
const int sw4 = 4;
const int sw5 = 5;
const int sw6 = 6;
const int sw7 = 7;

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

void setup() {
  // put your setup code here, to run once:
  for (int i = sw0; i<= sw7; i++) {
    pinMode(i, INPUT);
  }
  
  for (int i = segmentA; i<=segmentF; i++){
    pinMode(i, OUTPUT);
  }
  pinMode(segmentG, OUTPUT);
  
  for (int i = digit1; i<=digit4; i++){
    pinMode(i, OUTPUT);
  }
  digitalWrite(digit1, LOW);
  digitalWrite(digit2, LOW);
  digitalWrite(digit3, LOW);
  digitalWrite(digit4, LOW);

//  Serial.begin(9600);
}

void loop() {
  char input = PIND;
  char lower = input & 0x0F;
  char upper = (input >> 4) & 0x0F;
  int result; 
  result = lower * upper;
//  Serial.print("result");
//  Serial.println(result);
  char result0 = result % 10;
  char result1 = (result / 10) % 10;
  char result2 = (result / 100) % 10;

  digitalWrite(digit1, HIGH);
  digitalWrite(digit2, LOW);
  digitalWrite(digit3, LOW);
  display7Segment(result0);
  delay(5);

  digitalWrite(digit1, LOW);
  if (result2 != 0) {
    digitalWrite(digit2, HIGH);
  } else if (result1 != 0) {
    digitalWrite(digit2, HIGH);
  }
  
  
  digitalWrite(digit3, LOW);
  display7Segment(result1);
  delay(5);

  digitalWrite(digit1, LOW);
  digitalWrite(digit2, LOW);
  if (result2 != 0) digitalWrite(digit3, HIGH);
  display7Segment(result2);
  delay(5);
}
