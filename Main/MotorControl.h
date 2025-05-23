#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

//#include <Arduino.h>
#include <TMC2208Stepper.h>

  #define RX1_PIN 14  // Define correct RX pin for ESP32 UART1
  #define TX1_PIN 12  // Define correct TX pin for ESP32 UART1
  #define RX2_PIN 18  // Define correct RX pin for ESP32 UART2
  #define TX2_PIN 17  // Define correct TX pin for ESP32 UART2

  #define Current 125
  #define MicroStep 8

class MotorControl {

  private:
    // Create hardware serial instances for ESP32 UART
    HardwareSerial TMC_Serial1;  // UART1
    HardwareSerial TMC_Serial2;  // UART2

    // Initialize TMC2208 drivers with correct serial instances
    TMC2208Stepper driver;
    TMC2208Stepper driver2;

    const int stepPin1;
    const int stepPin2;
    const int stepPin3;
    const int stepPin4;

    bool shaftState;

  public:
    MotorControl(int step1, int step2, int step3, int step4);
    void motorSetUp();
    void horizontalMove(uint16_t speed);
    void verticalMove(uint16_t speed); 
    void swapDirection();
    void setShaftState();
    void driverOn();
    void driverOff();
};

#endif