#include <Arduino.h>
#include <cstdint>
//#include <SPI.h>
// #include <SD.h>
#include <TMC2208Stepper.h>
//#include <Arduino_FreeRTOS.h>

#include "BluetoothControl.h"
#include "FileControl.h"
#include "MotorControl.h"
#include "PrintheadControl.h"

TaskHandle_t Task_Main;
TaskHandle_t Task_Status;

const int chipSelect = 53;
//File dataFile;

MotorControl motor(12, 13, 14, 15);
FileControl fileHandler(&motor);

MotorControl motorControl(3, 4, 5, 6);

void setup() {
  
  Serial.begin(115200);
  // while (!Serial) {
  //   ;
  // }

  setupSD();

  Serial.println("Right here");

  xTaskCreate(MainFunctions, "Main", 2048, NULL, 1, &Task_Main);
  xTaskCreate(BluetoothStatus, "Status", 2048, NULL, 1, &Task_Status);
}


void loop() {

}

void MainFunctions(void *param) {

  (void) param;

  // Serial.println("MainFunction Loop reached");
  // delay(2000);
  //processSDFile();
  while(1) {}
}

void BluetoothStatus(void *param) {

  (void) param;

  Serial.println("Getting Instance");

  BluetoothControl* btControl = BluetoothControl::getInstance();

  Serial.println("Got Instance");

  while(1) {
    // statusMessages();
    
    vTaskDelay(5000/portTICK_PERIOD_MS);
  }
}