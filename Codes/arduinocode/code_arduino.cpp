// Code designed for Arduino Mega 2560
// Currently tested and working for 5 strings


// ================== PIN SETUP ==================
const int stepYPin = 2;  // Y.STEP
const int dirYPin  = 5;  // Y.DIR
const int laser    = 53;

int stepPin = stepYPin;
int dirPin  = dirYPin;

int enc1 = 50;
int enc2 = 51;

int ldr1 = A8;   
int ldr2 = A9;
int ldr3 = A10;
int ldr4 = A11;
int ldr5 = A12;

int led1 = 17;
int led2 = 18;
int led3 = 19;
int led4 = 20;
int led5 = 21;

int k = 100;

// ================== STEPPER SETTINGS ==================


int stepsPerRev = 150;       // 150-200    distance between the strings   higher stepsPerRev ====> greater distance between the strings, this is the variable incremented by the encoder.
int pulseWidthMicros  = 50;     // signals for the stepper, generally they should not go below 50.
int millisBtwnSteps   = 50;       //  same here, not less than 45ms. 

int tempo = 4;                // Important for the laser
int corde = 4;
short cordemezzi = stepsPerRev / corde;

int analogVal;

// ====== VARIABLES ADDED FOR THE ENCODER ======
volatile int lastEnc1 = HIGH;
volatile int lastEnc2 = HIGH;

// ====== FUNCTION ADDED FOR THE ENCODER ======
void readEncoder() {
  int a = digitalRead(enc1);
  int b = digitalRead(enc2);

  if (a != lastEnc1) {       // change on A
    if (a == HIGH) {         // rising edge
      if (b == LOW) stepsPerRev++;   // clockwise
      else stepsPerRev--;            // counterclockwise

      if (stepsPerRev < 10) stepsPerRev = 10; // minimum limit
      cordemezzi = stepsPerRev / corde;       // update
    }
  }

  lastEnc1 = a;
  lastEnc2 = b;
}


void setup() {
  Serial.begin(9600);

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(laser, OUTPUT);

  pinMode(enc1, INPUT_PULLUP);
  pinMode(enc2, INPUT_PULLUP);  

  // ===== LDRs  pull-up =====
  pinMode(ldr1, INPUT_PULLUP);
  pinMode(ldr2, INPUT_PULLUP);
  pinMode(ldr3, INPUT_PULLUP);
  pinMode(ldr4, INPUT_PULLUP);
  pinMode(ldr5, INPUT_PULLUP);

  //========== OUTPUT SIGNALS FOR THE DAISY =====

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);        // notes
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
}



void loop() {

  readEncoder();


  int v1 = analogRead(ldr1);
  int v2 = analogRead(ldr2);
  int v3 = analogRead(ldr3);
  int v4 = analogRead(ldr4);
  int v5 = analogRead(ldr5);

   //Debug print
  Serial.print("LDRs: ");
  Serial.print(v1); Serial.print(" ");
  Serial.print(v2); Serial.print(" ");
  Serial.print(v3); Serial.print(" ");
  Serial.print(v4); Serial.print(" ");
  Serial.println(v5);

  // ================== FORWARD ==================

  for (int j = 0; j < corde; j++) {

    digitalWrite(dirPin, HIGH);  // dir 
  
    for (int i = 0; i < cordemezzi; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(millisBtwnSteps);
      
      readEncoder();   // encoder also during movement
    }

    digitalWrite(laser, HIGH);
  
    int v1 = analogRead(ldr1);
    int v2 = analogRead(ldr2);
    int v3 = analogRead(ldr3);
    int v4 = analogRead(ldr4);
    int v5 = analogRead(ldr5);

    if(v1>k){
      digitalWrite(led1, HIGH);
    }else digitalWrite(led1, LOW);
    
    if(v2>k){
      digitalWrite(led2, HIGH);
    }else digitalWrite(led2, LOW);
    
    if(v3>k){
      digitalWrite(led3, HIGH);
    }else digitalWrite(led3, LOW);
    
    if(v4>k){
      digitalWrite(led4, HIGH);
    }else digitalWrite(led4, LOW);

    if(v5>k){
      digitalWrite(led5, HIGH);
    }else digitalWrite(led5, LOW);

    
    delay(tempo);
    digitalWrite(laser, LOW);
  }

  // ================== BACKWARD ==================

  for (int j = 0; j < corde; j++) {

    digitalWrite(dirPin, LOW); // setting dir pin to 0 reverses the direction

    for (int i = 0; i < cordemezzi; i++) {
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(pulseWidthMicros);
      digitalWrite(stepPin, LOW);
      delayMicroseconds(millisBtwnSteps);

      readEncoder();   //
    }

    digitalWrite(laser, HIGH);

    delay(tempo);
    digitalWrite(laser, LOW);
  }

  //delay(10);
}  
