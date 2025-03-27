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

  enum PrinterState {
    WAITING_FOR_CONNECTION,
    WAITING_FOR_IMAGE,
    PRINTING,
    PAUSED,
    PRINT_COMPLETE
  };

  PrinterState currentState = WAITING_FOR_CONNECTION;
  // PrinterState currentState = WAITING_FOR_IMAGE;
  int linesRead = 0;
  bool pauseRequested = false;
  int currentProgress = 0;
  int totalPrintRows = bluetooth->getTotalRows();

  while(1) {

    if(bluetooth->getAdditionToPrintStatus() == 0) {  // Pause
      if(currentState == PRINTING || currentState == WAITING_FOR_IMAGE) {
        pauseRequested = true;
        Serial.println("Print Paused");
      }
    }
    else if(bluetooth->getAdditionToPrintStatus() == 5) {  // Play
      if (currentState == PAUSED) {
        currentState = WAITING_FOR_IMAGE;
        Serial.println("Print Resumed");
      }
    }
    else if(bluetooth->getPrintStatus() == 0) {  // Stop
      currentState = PRINT_COMPLETE;
      linesRead = 0;
      fileControl.forward = true;
      Serial.println("Print Stopped");
    }

    switch(currentState) {
      case WAITING_FOR_CONNECTION:
        if(bluetooth->getDeviceConnected()) {
          currentState = WAITING_FOR_IMAGE;
          Serial.println("Bluetooth device connected - Waiting for image");
        }
        break;

      case WAITING_FOR_IMAGE:
        if(!bluetooth->getDeviceConnected()) {
          currentState = WAITING_FOR_CONNECTION;
          Serial.println("Bluetooth device disconnected. Waiting for reconnection.");
        } 
        else if(bluetooth->getIsImageReceived() && pauseRequested) {
            currentState = PAUSED;
            pauseRequested = false;
            Serial.println("Pause until play");
        }
        else if(bluetooth->getIsImageReceived() && !pauseRequested) {
          linesRead = fileControl.processSDFile();
          currentState = PRINTING;
          Serial.println("Start Printing");
        }
        // linesRead = fileControl.processSDFile();
        // currentState = PRINTING;
        break;

      case PRINTING:
        if(linesRead > 0) {
          print(linesRead);

          currentProgress += linesRead;
          int progressPercentage = (currentProgress * 100) / totalPrintRows;
          bluetooth->updatePrintProgress(progressPercentage);

          if(pauseRequested) {
            currentState = PAUSED;
            pauseRequested = false;
            Serial.println("Pause after row");
          }
          linesRead = fileControl.processSDFile();
        } else {
          currentState = PRINT_COMPLETE;
          Serial.println("Print complete");
        }
        break;

      case PAUSED:
        vTaskDelay(100 / portTICK_PERIOD_MS);
        break;

      case PRINT_COMPLETE:      // Get next step from app to restart process
        // if( XXX ) {
        //   currentState = WAITING_FOR_IMAGE;
        //   Serial.println("Ready for next print job.");
        // }
        break;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
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
    for(size_t col = 0; col < fileControl.lines[0].length(); col++) {
      vTaskSuspend(Task_Status);
      for(int row = 0; row < lineCount; row++) {
        if(col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
          Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
          printheadControl.makeDot(row);
          vTaskDelay(500 / portTICK_PERIOD_MS);
        }
      }
      vTaskResume(Task_Status);
      motorControl.horizontalMove();
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  } else {
    motorControl.swapDirection();
    // Read column by column from the last to the first
    for(int col = fileControl.lines[0].length() - 1; col >= 0; col--) {
      vTaskSuspend(Task_Status);
      for(int row = lineCount - 1; row >= 0; row--) {
        if(col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
          Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
          printheadControl.makeDot(row);
          vTaskDelay(500 / portTICK_PERIOD_MS);
        }
      }
      vTaskResume(Task_Status);
      motorControl.horizontalMove();
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
  }
  motorControl.driver2State();
  motorControl.verticleMove();
  vTaskDelay(500 / portTICK_PERIOD_MS);

  // Alternate direction
  fileControl.forward = !fileControl.forward;
}