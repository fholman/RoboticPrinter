#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TMC2208Stepper.h>
#include <Arduino_FreeRTOS.h>

#include "BluetoothControl.h"
#include "FileControl.h"
#include "MotorControl.h"
#include "PrintheadControl.h"

#define RX_PIN 15
#define TX_PIN 14
#define RX1_PIN 19
#define TX1_PIN 18

#define Current 250
#define MicroStep 8

TMC2208Stepper driver = TMC2208Stepper(RX1_PIN, TX1_PIN);
TMC2208Stepper driver2 = TMC2208Stepper(RX_PIN, TX_PIN);

// port assignment
// #define bluetooth_RX PB_11
// #define bluetooth_TX PB_10
// HardwareSerial BluetoothSerial(bluetooth_RX, bluetooth_TX);

TaskHandle_t Task_Main;
TaskHandle_t Task_Status;

const uint8_t nozzlePins[12] = {30, 28, 22, 24, 26, 35, 36, 37, 34, 33, 32, 31};

const int stepPin1 = 3;
const int stepPin2 = 4;
const int stepPin3 = 5;
const int stepPin4 = 6;

const uint8_t dotPause = 5;
const int dotPauseLong = 2000;

const int chipSelect = 53;
File dataFile;

void setup() {
  
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  driver.beginSerial(115200);  
  driver2.beginSerial(115200); 
  driver.push();
  driver2.push(); 

  Serial.print("\nInitialising SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    while(true);
  }
  Serial.println("\ninitialisation done.");

  // BluetoothSerial.begin(460800);

  for(int i = 0; i < 12; i++){
    pinMode(nozzlePins[i], OUTPUT);
    digitalWrite(nozzlePins[i], LOW);
  }

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
  driver2.shaft(true);
  driver2.en_spreadCycle(1);           // 1: spreadCycle 0: stealthChop
  driver2.pwm_autoscale(0);            // 1: if stealthChop is chosen. Otherwise 0
  driver2.mstep_reg_select(1);
  driver2.mres(MicroStep);

  xTaskCreate(MainFunctions, "Main", 2048, NULL, 1, &Task_Main);
  xTaskCreate(BluetoothStatus, "Status", 2048, NULL, 1, &Task_Status);
}


void loop() {

}

void MainFunctions(void *param) {

  (void) param;

  Serial.println("MainFunction Loop reached");
  delay(2000);
  processSDFile();
}

void BluetoothStatus(void *param) {

  (void) param;

  while(1) {
    vTaskDelay(1000/portTICK_PERIOD_MS); // 1 second delay on this split 
    // Bluetooth();
    Serial.println("Bluetooth Message Send");
  }
}