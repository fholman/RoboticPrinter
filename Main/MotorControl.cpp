#include "MotorControl.h"
#include <TMC2208Stepper.h>

MotorControl::MotorControl(int step1, int step2, int step3, int step4)
  : stepPin1(step3), stepPin2(step4), stepPin3(step1), stepPin4(step2),
    TMC_Serial1(1), TMC_Serial2(2),
    driver(&TMC_Serial1), driver2(&TMC_Serial2), 
    shaftState(false) {}

void MotorControl::motorSetUp(){
  TMC_Serial1.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
  TMC_Serial2.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);

  driver.push();
  driver2.push(); 

  pinMode(stepPin1,OUTPUT); 
  pinMode(stepPin2,OUTPUT); 
  pinMode(stepPin3,OUTPUT); 
  pinMode(stepPin4,OUTPUT); 

  driver.pdn_disable(true);              // Use PDN/UART pin for communication
  driver.I_scale_analog(false);           // Adjust current from the registers
  driver.rms_current(Current);        // Set driver current 
  driver.toff(0);                     
  driver.shaft(false);
  driver.en_spreadCycle(1);           // 1: spreadCycle 0: stealthChop
  driver.pwm_autoscale(0);            // 1: if stealthChop is chosen. Otherwise 0
  driver.mstep_reg_select(1);
  driver.mres(MicroStep);

  driver2.pdn_disable(true);              // Use PDN/UART pin for communication
  driver2.I_scale_analog(false);           // Adjust current from the registers
  driver2.rms_current(Current);        // Set driver current 
  driver2.toff(0);                     
  driver2.shaft(true);
  driver2.en_spreadCycle(1);           // 1: spreadCycle 0: stealthChop
  driver2.pwm_autoscale(0);            // 1: if stealthChop is chosen. Otherwise 0
  driver2.mstep_reg_select(1);
  driver2.mres(MicroStep);
}

void MotorControl::setShaftState() {
  shaftState = false;
}

// 1:298 = 14 steps a pixel - 14 steps horizontally / 170 steps vertically
// 1:150 = 7 steps a pixel - 7 steps horizontally / 84 steps vertically
// 1:20 = 1 step a pixel - 1 step horizontally / 12 steps vertically
// speed should be no faster than 5 and no slower than maybe 40ish
void MotorControl::horizontalMove(uint16_t speed) {
  for(int x = 0; x < 1; x++) {
    digitalWrite(stepPin3,HIGH); 
    digitalWrite(stepPin4,HIGH); 
    delayMicroseconds(1000);
    digitalWrite(stepPin3,LOW); 
    digitalWrite(stepPin4,LOW); 
    delay(speed); // by changing this time delay between the steps we can change the rotation speed
  }
}

void MotorControl::verticalMove(uint16_t speed) {
  delay(5);
  driver.toff(0);
  driver2.toff(5);
  delay(5);

  for(int x = 0; x < 12; x++) {
    digitalWrite(stepPin1,HIGH); 
    digitalWrite(stepPin2,HIGH);
    delayMicroseconds(1000);
    digitalWrite(stepPin1,LOW); 
    digitalWrite(stepPin2,LOW);
    delay(speed); // by changing this time delay between the steps we can change the rotation speed
  }

  delay(5);
  driver2.toff(0);
  delay(5);
}

void MotorControl::swapDirection() {
  driver.shaft(!shaftState);

  for (int i=0; i<3; i++) {
    horizontalMove(40);
  }

  shaftState = !shaftState;
}

void MotorControl::driverOn() {
  delay(5);
  driver.toff(5);
  delay(5);
}

void MotorControl::driverOff() {
  delay(5);
  driver.toff(0);
  delay(5);
}