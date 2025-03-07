#include "MotorControl.h"
#include <TMC2208Stepper.h>


void horizontalMove() {
  Serial.println("Motor horizontal move");
  driver2.toff(5);

  for(int x = 0; x < 14; x++) {
    digitalWrite(stepPin3,HIGH); 
    digitalWrite(stepPin4,HIGH); 
    delayMicroseconds(1000);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin3,LOW); 
    digitalWrite(stepPin4,LOW); 
    delay(25); 
  }
  driver2.toff(0);
}

void verticleMove() {
  Serial.println("Motor verticle move");
  driver.toff(5);

  for(int x = 0; x < 96; x++) {
    digitalWrite(stepPin1,HIGH); 
    digitalWrite(stepPin2,HIGH);
    delayMicroseconds(1000);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin1,LOW); 
    digitalWrite(stepPin2,LOW);
    delay(25); 
  }
  driver.toff(0);
}
