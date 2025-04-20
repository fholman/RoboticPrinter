#include <Arduino.h>
#include <cstdint>
#include <SPI.h>
#include <SD.h>
#include <TMC2208Stepper.h>
#include <freertos/FreeRTOS.h>
#include "esp_heap_caps.h"

#include "BluetoothControl.h"
#include "FileControl.h"
#include "MotorControl.h"
#include "PrintheadControl.h"

TaskHandle_t Task_Main;
TaskHandle_t Task_Status;
TaskHandle_t Task_Status_Message;

bool printDone = false;
int totalPrintRows;

const int ledPinBlue = 8;
const int ledPinGreen = 16;

FileControl fileControl;
PrintheadControl printheadControl;
MotorControl motorControl(21, 13, 11, 9);
BluetoothControl* bluetooth;

uint16_t maxSpeed = 5;
uint16_t minSpeed = 40;

SemaphoreHandle_t debugMutex;

String debugMessage = "Starting System ... ";

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

uint16_t testSpeed = 15;

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

  pinMode(ledPinBlue, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  digitalWrite(ledPinBlue, LOW);
  digitalWrite(ledPinGreen, LOW);

  delay(5000);

  // xTaskCreate(MainFunctions, "Main", 8192, NULL, 2, &Task_Main);
  // xTaskCreate(BluetoothStatus, "Status", 8192, NULL, 1, &Task_Status);
  // xTaskCreate(BluetoothDebug, "Status_Message", 8192, NULL, 1, &Task_Status_Message);

  // xTaskCreate(TestFunctions, "Main", 4096, NULL, 1, &Task_Main);

  // xTaskCreatePinnedToCore(TestFunctions, "Main", 4096, NULL, 2, &Task_Main, 1);

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

    motorControl.driverOn();

    // 3 horizontal movements to start moving

    motorControl.swapDirection();

    for (int i=0; i<10; i++) {
      motorControl.horizontalMove(40);  
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void MainFunctions(void *param) {
  (void) param;

  int linesRead = 0;
  int currentProgress = 0;

  currentState = STOPPED;

  // Serial.println("Right here");

  while(1) {
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
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
        digitalWrite(ledPinGreen, LOW);
        // motorControl.driverOff();
        break;

      case PRINTING:
        stateOfApp = 1;
        digitalWrite(ledPinGreen, HIGH);
        // Serial.println("Printing");
        linesRead = fileControl.processSDFile();
        // Serial.println("HERE");
        if(linesRead > 0) {
          // Serial.println("First Row");

          // vTaskDelay(1000 / portTICK_PERIOD_MS);
          // UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(Task_Main); // NULL = current task
          // sendMessage("Main task free stack left after processing : " + String(highWaterMark));
          // vTaskDelay(1000 / portTICK_PERIOD_MS);
          // size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);  // You can specify other capabilities
          // sendMessage("Free Heap after processing : " + String(freeHeap));
          // vTaskDelay(1000 / portTICK_PERIOD_MS);

          print(linesRead);

          currentProgress += linesRead;
          
          int progressPercentage = (currentProgress * 100) / totalPrintRows;
          // Serial.println(progressPercentage);
          // Serial.println(totalPrintRows);
          bluetooth->updatePrintProgress(progressPercentage);
        } else {
          currentState = STOPPED;
          motorControl.setShaftState();
          fileControl.resetFilePosition();
          // Serial.println("Print complete");
        }
        break;
    }

    // UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(NULL); // NULL = current task
    // sendMessage("Main task free stack left : " + String(highWaterMark));
    // // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);  // You can specify other capabilities
    // sendMessage("Free Heap : " + String(freeHeap));

    vTaskDelay(50 / portTICK_PERIOD_MS); // should only have one 50ms delay here
  }
}

/*
Will use this task to check for print status and messages from the app
*/
void BluetoothStatus(void *param) {

  (void) param;

  while(1) {
    if (bluetooth->getDeviceConnected()) {
      digitalWrite(ledPinBlue, HIGH);
      bluetooth->statusMessages(stateOfApp);

      if(bluetooth->getAppStatus() == 0) {  // stopped
        currentState = STOPPED;
        fileControl.resetFilePosition();
        sendMessage("Device has been stopped ... ");
      }
      else if(bluetooth->getAppStatus() == 1) {  // Paused
        currentState = PAUSED;
        sendMessage("Device has been paused ... ");
      }
      else if(bluetooth->getAppStatus() == 2) {  // Play
        totalPrintRows = bluetooth->getTotalRows();
        // if (bluetooth->getPrintStatus() == 0) motorControl.setShaftState();
        bluetooth->updatePrintProgress(1);
        currentState = PRINTING;
        sendMessage("Device has begun printing ...");
      }

      bluetooth->setAppStatus();
      vTaskDelay(1000/portTICK_PERIOD_MS);
    } else {
      digitalWrite(ledPinBlue, HIGH);
      vTaskDelay(1000/portTICK_PERIOD_MS);
      digitalWrite(ledPinBlue, LOW);
      vTaskDelay(1500/portTICK_PERIOD_MS);
    }
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
  }
}

void sendMessage(String msg) {
  if (bluetooth->getDeviceConnected()) {
    debugMessage = msg;
    xSemaphoreGive(debugMutex);
  }
}

void print(int lineCount){

  motorControl.verticalMove(testSpeed);

  motorControl.driverOn();

  sendMessage("Device is printing a row");
  // vTaskDelay(1000 / portTICK_PERIOD_MS);

  if (fileControl.forward) {
    motorControl.swapDirection();
    motorControl.horizontalMove(testSpeed);
    // Read column by column from the first to the last
    for(size_t col = 0; col < fileControl.lines[0].length(); col++) {
      // UBaseType_t highWaterMark = uxTaskGetStackHighWaterMark(Task_Main); // NULL = current task
      // vTaskDelay(1000 / portTICK_PERIOD_MS);
      // sendMessage("Main task free stack left : " + String(highWaterMark));
      // vTaskDelay(1000 / portTICK_PERIOD_MS);
      // sendMessage("Image Width : " + String(fileControl.lines[0].length()));
      // vTaskDelay(1000 / portTICK_PERIOD_MS);
      // size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);  // You can specify other capabilities
      // sendMessage("Free Heap : " + String(freeHeap));
      // delay(1000000);
      //vTaskSuspend(Task_Status);
      for (int i=0; i<2; i++) {
        for(int row = 0; row < lineCount; row++) {
          if(col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
            printheadControl.makeDot(row);
          }
          else {
            delayMicroseconds(8);
          }
        }
        delayMicroseconds(800);
      }
      if (currentState == PAUSED) { 
        stateOfApp = 0;
        motorControl.driverOff();
        while (currentState == PAUSED) {
          digitalWrite(ledPinGreen, HIGH);
          vTaskDelay(250 / portTICK_PERIOD_MS);
          digitalWrite(ledPinGreen, LOW);
          vTaskDelay(250 / portTICK_PERIOD_MS);
        }
        digitalWrite(ledPinGreen, HIGH);
        motorControl.driverOn();
        stateOfApp = 1;
      }
      if (currentState == STOPPED) {
        motorControl.driverOff();
        return;
      }

      //vTaskResume(Task_Status);
      motorControl.horizontalMove(testSpeed);
    }
  } else {
    motorControl.swapDirection();
    motorControl.horizontalMove(testSpeed);
    // Read column by column from the last to the first
    int length = fileControl.lines[0].length();
    for(int col = length - 1; col >= 0; col--) {
      //vTaskSuspend(Task_Status);
      for (int i=0; i<2; i++) {
        for(int row = lineCount - 1; row >= 0; row--) {
          if(col < fileControl.lines[row].length() && fileControl.lines[row][col] == '1') {
            printheadControl.makeDot(row);
          }
          else {
            delayMicroseconds(854);
          }
        }
      }
      if (currentState == PAUSED) { 
        stateOfApp = 0;
        motorControl.driverOff();
        while (currentState == PAUSED) {
          digitalWrite(ledPinGreen, HIGH);
          vTaskDelay(250 / portTICK_PERIOD_MS);
          digitalWrite(ledPinGreen, LOW);
          vTaskDelay(250 / portTICK_PERIOD_MS);
        }
        digitalWrite(ledPinGreen, HIGH);
        motorControl.driverOn();
        stateOfApp = 1;
      }
      if (currentState == STOPPED) {
        motorControl.driverOff();
        return;
      }

      //vTaskResume(Task_Status);
      motorControl.horizontalMove(testSpeed);
    }
  }
  motorControl.driverOff();

  // motorControl.verticalMove();

  sendMessage("Device has finished printing a row");

  // Alternate direction
  fileControl.forward = !fileControl.forward;
}