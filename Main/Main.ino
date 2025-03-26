#include <Arduino.h>
#include <cstdint>
#include <SPI.h>
#include <SD.h>
#include <TMC2208Stepper.h>
#include <freertos/FreeRTOS.h>

#include "BluetoothControl.h"
#include "FileControl.h"
#include "MotorControl.h"
#include "PrintheadControl.h"

TaskHandle_t Task_Main;
TaskHandle_t Task_Status;

const int chipSelect = 53;
bool printDone = false;

FileControl fileControl;
PrintheadControl printheadControl;
MotorControl motorControl(3, 4, 5, 6);
BluetoothControl* bluetooth;

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

void setup() {
  
  Serial.begin(115200);
  // while (!Serial) {
  //   ;
  // }

  fileControl.setupSD();
  printheadControl.printheadSetUp();
  motorControl.motorSetUp();

  bluetooth = new BluetoothControl();

  xTaskCreate(MainFunctions, "Main", 2048, NULL, 1, &Task_Main);
  xTaskCreate(BluetoothStatus, "Status", 2048, NULL, 1, &Task_Status);
}


void loop() {

}

void MainFunctions(void *param) {

  (void) param;

  Serial.println("MainFunction Loop reached");

  while(!printDone){
    int linesRead = fileControl.processSDFile();
    if (linesRead > 0) {
      // Now process those 12 lines and control the motor
      print(linesRead);
    } else {
      Serial.println("End of print.");
      printDone = true;
    }
  }
}

void BluetoothStatus(void *param) {

  (void) param;

  while(1) {
    bluetooth->statusMessages();
    bluetooth->debugTask("Hello");
    
    vTaskDelay(5000/portTICK_PERIOD_MS);
  }
}

void print(int lineCount){
  motorControl.driver2State();

  if (fileControl.forward) {
    motorControl.swapDirection();
    // Read column by column from the first to the last
    for (size_t col = 0; col < fileControl.lines[0].length(); col++) {
      taskENTER_CRITICAL(&myMutex);
      vTaskSuspend(Task_Status);
      for (int row = 0; row < lineCount; row++) {
        if (col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
          Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
          printheadControl.makeDot(row);
          //delay(500);
        }
      }
      vTaskResume(Task_Status);
      taskEXIT_CRITICAL(&myMutex);
      motorControl.horizontalMove();
      //delay(2000);
    }
  } else {
    MotorControl.swapDirection();
    // Read column by column from the last to the first
    for (int col = fileControl.lines[0].length() - 1; col >= 0; col--) {
      for (int row = lineCount - 1; row >= 0; row--) {
        vTaskSuspend(Task_Status);
        if (col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
          Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
          printheadControl.makeDot(row);
          //delay(500);
        }
        vTaskResume(Task_Status);
      }
      motorControl.horizontalMove();
      //delay(2000);
    }
  }
  motorControl.driver2State();
  motorControl.verticleMove();
  //delay(2000);

  // Alternate direction
  fileControl.forward = !fileControl.forward;

  // Reset line buffer
  lineCount = 0;
}