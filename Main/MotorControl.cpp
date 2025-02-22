#include "MotorControl.h"

void horizontalMove(int horizontalSteps) {
  for(int x = 0; x < 10; x++) {
    digitalWrite(stepPin1,HIGH); 
    digitalWrite(stepPin2,HIGH); 
    delayMicroseconds(horizontalSteps);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin1,LOW); 
    digitalWrite(stepPin2,LOW); 
    delay(20); 
  }
}

void verticleMove(int verticleSteps) {
  for(int x = 0; x < 10; x++) {
    digitalWrite(stepPin3,HIGH); 
    digitalWrite(stepPin4,HIGH);
    delayMicroseconds(verticleSteps);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin3,LOW); 
    digitalWrite(stepPin4,LOW);
    delay(20); 
  }
}
