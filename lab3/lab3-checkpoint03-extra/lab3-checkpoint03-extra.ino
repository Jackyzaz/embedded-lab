const int segmentA = 14;
const int segmentB = 15;
const int segmentC = 16;
const int segmentD = 17;
const int segmentE = 18;
const int segmentF = 19;
const int segmentG = 12;

const int c1 = 7;
const int c2 = 6;
const int c3 = 5;
const int c4 = 4;

const int r1 = 11;
const int r2 = 10;
const int r3 = 9;
const int r4 = 8;

const int rowPins[4] = {r1, r2, r3, r4}; 
const int colPins[4] = {c1, c2, c3, c4};

const int digit1 = 13;

const int keychecker = 3; // PD3 -INT1

char keys[4][4] = {     
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'E','0','F','D'}
};

char current;

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
void scanKeyDisplay() {
  Serial.println("INT1!");
  for (int c = 0; c < 4; c++) {
    digitalWrite(colPins[c], HIGH); 
  }

  for (int c = 0; c < 4; c++) {
    digitalWrite(colPins[c], LOW); 
    for (int r = 0; r < 4; r++) {
      if (digitalRead(rowPins[r]) == LOW) { 
        current = keys[r][c];
        display7Segment(current);
      }
    }
    digitalWrite(colPins[c], HIGH); 
  }

  for (int c = 0; c < 4; c++) {
    digitalWrite(colPins[c], LOW);
  }


}




void setup() {
  pinMode(segmentA, OUTPUT);
  pinMode(segmentB, OUTPUT);
  pinMode(segmentC, OUTPUT);
  pinMode(segmentD, OUTPUT);
  pinMode(segmentE, OUTPUT);
  pinMode(segmentF, OUTPUT);
  pinMode(segmentG, OUTPUT);

  pinMode(c1, OUTPUT); digitalWrite(c1, LOW);
  pinMode(c2, OUTPUT); digitalWrite(c2, LOW);
  pinMode(c3, OUTPUT); digitalWrite(c3, LOW);
  pinMode(c4, OUTPUT); digitalWrite(c4, LOW);

  pinMode(r1, INPUT_PULLUP);
  pinMode(r2, INPUT_PULLUP);
  pinMode(r3, INPUT_PULLUP);
  pinMode(r4, INPUT_PULLUP);
  
  pinMode(digit1, OUTPUT);
  digitalWrite(digit1, HIGH); 

  pinMode(keychecker, INPUT);

  EIMSK = 0b00000010; // INT1 (PD3)
  EICRA = 0b00001000; // Falling Edge
  sei();
  
  display7Segment('0'); 
  Serial.begin(9600);
}

void loop() {
  // empty loop
}

ISR(INT1_vect) {
  scanKeyDisplay();
}
