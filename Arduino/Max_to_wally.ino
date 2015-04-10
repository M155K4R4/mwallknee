//Max-to-Wally120 v1.0 10-04-2015
//For controlling OpenKnit via Max
//By Thomas Rutgers (thomasrutgers-at-gmail.com)

//Because on the current openKnit shield pin 0 & 1 is also used for the encoder interrupt, the serial communication is not ideal but works. Using another stepper interval then 900 can cause problems.

#include <Servo.h> 
#include <Encoder.h>

//servos
Servo servoFB; //front bottom
Servo servoFT; //front top
Servo servoBB; //back bottom  
Servo servoBT; //back top
Servo servoCAR; //carriage pick-up
int selectedServo = 0;

//encoder
Encoder myEnc(1, 0);
int oldPosition = -999;

//stepper
int stepperInterval = 900;  //interval between steps, determines speed of the motor rotation
int stepperDir = 0; //0=stop 1=forward 2=backward
unsigned long stepperTime; //for timing the stepper pulse
int stepPin = A1; //STEP pin
int dirPin = A2;  //DIR pin
int resetPin = A0; //RESET pin

void setup() 
{ 
  //stepper driver
  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT); 
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);    
  delay(10);
  digitalWrite(resetPin, HIGH);  
  //servo motors
  servoFB.attach(7);  
  servoFT.attach(8);  
  servoBB.attach(9);   
  servoBT.attach(10);
  servoCAR.attach(11);
  //start serial communication
  Serial.begin(9600);
  //end stop
  attachInterrupt(0, endStop, CHANGE);  //pin 3
}

//////////////////////////////////////////////////////////////////////////////
//
//  Serial protocol from Max:              Serial protocol from Arudino:
//
//  201-205: select servo 1-5              252: negative encoder value (needs homing)
//  0-180: set selected servo position     255: endstop hit
//  210: stepper stop                      251: next 2 value bytes added together makes encoder value
//  211: stepper move forward              0-250: value bytes
//  212: stepper move backward
//  215: stepper reset HIGH
//  216: stepper reset LOW
//
///////////////////////////////////////////////////////////////////////////////

void loop() {
  //read serial
  if (Serial.available() > 0) {
    byte c = Serial.read();
    if ((c>200)&&(c<206)) selectedServo = c-200;
    if ((c>209)&&(c<213)) {
      stepperDir = c-210;
      digitalWrite(dirPin, (stepperDir==2));
      delay(10);
    }
    if (c==215) digitalWrite(resetPin, HIGH);
    if (c==216) digitalWrite(resetPin,LOW);
    if (c<200) {
      if (selectedServo==1) servoFB.write(c);
      if (selectedServo==2) servoFT.write(c);
      if (selectedServo==3) servoBB.write(c);
      if (selectedServo==4) servoBT.write(c);
      if (selectedServo==5) servoCAR.write(c);
    }
  }
  //move stepper
  if (stepperTime < (micros() - stepperInterval)) {
    stepperTime = micros();
    if (stepperDir>0) {
       digitalWrite(stepPin, HIGH);  
       digitalWrite(stepPin, LOW);   
    }
  }
  //check encoder
  int p = myEnc.read();
  if (p != oldPosition) {
    oldPosition = p;
    if (p<0) Serial.write (252);
    else { // send the number using three bytes
      Serial.write(251);
      Serial.write(min(p,250));
      Serial.write(max((p-250),0));
    }
  }
}

void endStop() {
  //endstop reached: stop the stepper motor, set the encoder to 0, send out a message
  digitalWrite(dirPin,LOW);
  stepperDir=0;
  myEnc.write(0);    
  Serial.write(255);
}




