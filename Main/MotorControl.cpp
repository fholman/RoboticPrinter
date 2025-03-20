#include "MotorControl.h"
#include <TMC2208Stepper.h>

MotorControl::MotorControl(int step1, int step2, int step3, int step4)
  : stepPin1(step1), stepPin2(step2), stepPin3(step3), stepPin4(step4),
    TMC_Serial1(1), TMC_Serial2(2),
    driver(&TMC_Serial1), driver2(&TMC_Serial2), 
    shaftState(false), motor2State(false) {
      motorSetUp();
    }

void MotorControl::motorSetUp(){
  TMC_Serial1.begin(115200, SERIAL_8N1, RX1_PIN, TX1_PIN);
  TMC_Serial2.begin(115200, SERIAL_8N1, RX2_PIN, TX2_PIN);
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
  driver.shaft(true);
  driver.en_spreadCycle(1);           // 1: spreadCycle 0: stealthChop
  driver.pwm_autoscale(0);            // 1: if stealthChop is chosen. Otherwise 0
  driver.mstep_reg_select(1);
  driver.mres(MicroStep);

  driver2.pdn_disable(true);              // Use PDN/UART pin for communication
  driver2.I_scale_analog(false);           // Adjust current from the registers
  driver2.rms_current(Current);        // Set driver current 
  driver2.toff(0);                     
  driver2.shaft(false);
  driver2.en_spreadCycle(1);           // 1: spreadCycle 0: stealthChop
  driver2.pwm_autoscale(0);            // 1: if stealthChop is chosen. Otherwise 0
  driver2.mstep_reg_select(1);
  driver2.mres(MicroStep);

}

void MotorControl::horizontalMove() {
  Serial.println("Motor horizontal move");

  for(int x = 0; x < 14; x++) {
    digitalWrite(stepPin3,HIGH); 
    digitalWrite(stepPin4,HIGH); 
    delayMicroseconds(1000);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin3,LOW); 
    digitalWrite(stepPin4,LOW); 
    delay(25); 
  }
}

void MotorControl::verticleMove() {
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

void MotorControl::swapDirection() {
  if(shaftState) {
    driver2.shaft(false);
    shaftState = false;
  }else {
    driver2.shaft(true);
    shaftState = true;
  }
}

void MotorControl::driver2State() {
  if(motor2State) {
    driver2.toff(0);
    motor2State = false;
  }else {
    driver2.toff(5);
    motor2State = true;
  }
}