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
TaskHandle_t Task_Status_Message;

bool printDone = false;
int totalPrintRows;

FileControl fileControl;
PrintheadControl printheadControl;
MotorControl motorControl(21, 13, 11, 9);
BluetoothControl* bluetooth;

uint16_t maxSpeed = 5;
uint16_t minSpeed = 40;

SemaphoreHandle_t debugMutex;

String debugMessage = "Starting System ... ";

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

enum PrinterState {
  PRINTING,
  PAUSED,
  STOPPED
};

PrinterState currentState;
int stateOfApp = 2;

void setup() {
  
  // Serial.begin(115200);

  fileControl.setupSD();
  printheadControl.printheadSetUp();
  motorControl.motorSetUp();

  bluetooth = new BluetoothControl(fileControl);

  debugMutex = xSemaphoreCreateBinary();

  delay(15000);

  // xTaskCreate(MainFunctions, "Main", 8192, NULL, 2, &Task_Main);
  // xTaskCreate(BluetoothStatus, "Status", 8192, NULL, 1, &Task_Status);
  // xTaskCreate(BluetoothDebug, "Status_Message", 8192, NULL, 1, &Task_Status_Message);

  // xTaskCreate(TestFunctions, "Main", 4096, NULL, 1, &Task_Main);

  xTaskCreatePinnedToCore(MainFunctions, "Main", 4096, NULL, 2, &Task_Main, 1);
  xTaskCreatePinnedToCore(BluetoothStatus, "Status", 4096, NULL, 1, &Task_Status, 0);
  xTaskCreatePinnedToCore(BluetoothDebug, "Status_Message", 4096, NULL, 1, &Task_Status_Message, 0);
}


void loop() {}

/*
For testing on prototype nothing else
*/
void TestFunctions(void *param) {
  (void) param;

  while(1) {


    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void MainFunctions(void *param) {
  (void) param;

  int linesRead = 0;
  int currentProgress = 0;

  currentState = PRINTING;

  // Serial.println("Right here");

  while(1) {
    sendMessage("Looping Again");
    // Serial.println("Main Loop");
    switch(currentState) {
      case STOPPED:
        stateOfApp = 2;
        // Serial.println("Stopped");
        if (bluetooth->getDeviceConnected()) {
          if (!fileControl.isFileNotEmpty() || bluetooth->getTotalRows() == 0) { 
            bluetooth->updatePrintProgress(-1);
            currentProgress = -1;
          }
          else {
            bluetooth->updatePrintProgress(0);
            currentProgress = 0;
          }

          linesRead = 0;
          sendMessage("Stopped Section");
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
        // motorControl.driverOff();
        break;
      
      case PAUSED:
        stateOfApp = 0;
        vTaskDelay(500 / portTICK_PERIOD_MS);
        // Serial.println("Paused");
        // motorControl.driverOff();
        break;

      case PRINTING:
        stateOfApp = 1;
        // Serial.println("Printing");
        linesRead = fileControl.processSDFile();
        // Serial.println("HERE");
        if(linesRead > 0) {
          // Serial.println("First Row");

          print(linesRead);

          currentProgress += linesRead;
          
          int progressPercentage = (currentProgress * 100) / totalPrintRows;
          // Serial.println(progressPercentage);
          // Serial.println(totalPrintRows);
          bluetooth->updatePrintProgress(progressPercentage);
        } else {
          currentState = STOPPED;
          fileControl.resetFilePosition();
          // Serial.println("Print complete");
        }
        break;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/*
Will use this task to check for print status and messages from the app
*/
void BluetoothStatus(void *param) {

  (void) param;

  while(1) {
    if (bluetooth->getDeviceConnected()) {
      bluetooth->statusMessages(stateOfApp);

      if(bluetooth->getAppStatus() == 0) {  // stopped
        currentState = STOPPED;
        fileControl.resetFilePosition();
      }
      else if(bluetooth->getAppStatus() == 1) {  // Paused
        currentState = PAUSED;
      }
      else if(bluetooth->getAppStatus() == 2) {  // Play
        totalPrintRows = bluetooth->getTotalRows();
        motorControl.setShaftState();
        currentState = PRINTING;
      }

      bluetooth->setAppStatus();
    }
    
    vTaskDelay(2500/portTICK_PERIOD_MS);
  }
}

/*
Will use this task to check for print status and messages from the app
*/
void BluetoothDebug(void *param) {

  (void) param;

  while(1) {
    xSemaphoreTake(debugMutex, portMAX_DELAY);
    bluetooth->debugTask(debugMessage);
    // Serial.println("Sending debug message : " + debugMessage);
  }
}

void sendMessage(String msg) {
  if (bluetooth->getDeviceConnected()) {
    debugMessage = msg;
    xSemaphoreGive(debugMutex);
  }
}

void print(int lineCount){

  motorControl.driverOn();

  if (fileControl.forward) {
    motorControl.swapDirection();
    // Read column by column from the first to the last
    for(size_t col = 0; col < fileControl.lines[0].length(); col++) {
      //vTaskSuspend(Task_Status);
      for (int i=0; i<2; i++) {
        for(int row = 0; row < lineCount; row++) {
          if(col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
            printheadControl.makeDot(row);
          }
        }
      }
      //vTaskResume(Task_Status);
      motorControl.horizontalMove(40);
    }
  } else {
    motorControl.swapDirection();
    // Read column by column from the last to the first
    int length = fileControl.lines[0].length();
    for(int col = length - 1; col >= 0; col--) {
      //vTaskSuspend(Task_Status);
      for (int i=0; i<2; i++) {
        for(int row = lineCount - 1; row >= 0; row--) {
          if(col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
            printheadControl.makeDot(row);
          }
        }
      }
      //vTaskResume(Task_Status);
      motorControl.horizontalMove(40);
    }
  }
  motorControl.driverOff();

  motorControl.verticalMove();

  // Alternate direction
  fileControl.forward = !fileControl.forward;
}