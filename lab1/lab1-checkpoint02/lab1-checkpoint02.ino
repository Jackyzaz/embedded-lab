const int segmentA = 14;
const int segmentB = 15;
const int segmentC = 16;
const int segmentD = 17;
const int segmentE = 18;
const int segmentF = 19;
const int segmentG = 8;
const int sw0 = 0;
const int sw1 = 1;
const int sw2 = 2;
const int sw3 = 3;
const int sw4 = 4;
const int sw5 = 5;
const int sw6 = 6;
const int sw7 = 7;

const int led1 = 9;
const int led2 = 10;
const int led3 = 11;

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


//
//int readSwitch() {
//  int count;
//  count = 0;
//  for (int i=sw0; i <= sw7; i++) {
//    if (digitalRead(i)) count ++;
//  }
//  return count;
//}

int readLower() {
  int value = 0;
  if (digitalRead(sw0)) value += 1;
  if (digitalRead(sw1)) value += 2;
  if (digitalRead(sw2)) value += 4;
  if (digitalRead(sw3)) value += 8;
  return value;
}

int readUpper() {
  int value = 0;
  if (digitalRead(sw4)) value += 1;
  if (digitalRead(sw5)) value += 2;
  if (digitalRead(sw6)) value += 4;
  if (digitalRead(sw7)) value += 8;
  return value;
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

  // led setup
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);

}

void resetLed() {
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);

}

void loop() {
  char input = PIND;
  char lower = input & 0x0F;
  char upper = (input & 0xF0) >> 4;
  char x;
//  int lower = readLower();
//  int upper = readUpper();
  resetLed();

  if (upper == lower) {
    digitalWrite(led2, HIGH);
    x = upper;
  } else if (upper < lower) {
    digitalWrite(led1, HIGH);
    x = lower - upper;
  } else {
    // upper > lower
    digitalWrite(led3, HIGH) ;  
    x = upper - lower;
  }
  
  display7Segment(x);

}
