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
bool printDone = false;

// FileControl fileHandler();
// MotorControl motorControl(3, 4, 5, 6);
BluetoothControl* bluetooth;

void setup() {
  
  Serial.begin(115200);
  // while (!Serial) {
  //   ;
  // }

  //setupSD();

  bluetooth = new BluetoothControl();

  //xTaskCreate(MainFunctions, "Main", 2048, NULL, 1, &Task_Main);
  xTaskCreate(BluetoothStatus, "Status", 2048, NULL, 1, &Task_Status);
}


void loop() {

}

void MainFunctions(void *param) {

  // (void) param;

  // Serial.println("MainFunction Loop reached");

  // while(!printDone){
  //   int linesRead = fileHandler.processSDFile();
  //   if (linesRead > 0) {
  //     // Now process those 12 lines and control the motor
  //     print(linesRead);
  //   } else {
  //     Serial.println("End of print.");
  //     printDone = true;
  //   }
  // }
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
  if (lineCount == 12 || !dataFile.available()) {
    motorControl.driver2State();

    if (fileHandler.forward) {
      motorControl.swapDirection();
      // Read column by column from the first to the last
      for (size_t col = 0; col < fileHandler.lines[0].length(); col++) {
        taskENTER_CRITICAL(&myMutex);
        vTaskSuspend(Task_Status);
        for (int row = 0; row < lineCount; row++) {
          if (col < fileHandler.lines[row].length() && fileHandler.lines[row][col] == '1') {
            Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
            PrintheadControl.makeDot(row);
            //delay(500);
          }
        }
        vTaskResume(Task_Status);
        taskEXIT_CRITICAL(&myMutex);
        horizontalMove();
        //delay(2000);
      }
    } else {
      MotorControl.swapDirection();
      // Read column by column from the last to the first
      for (int col = fileHandler.lines[0].length() - 1; col >= 0; col--) {
        for (int row = lineCount - 1; row >= 0; row--) {
          vTaskSuspend(Task_Status);
          if (col < fileHandler.lines[row].length() && fileHandler.lines[row][col] == '1') {
            Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
            PrintheadControl.makeDot(row);
            //delay(500);
          }
          vTaskResume(Task_Status);
        }
        motorControl.horizontalMove();
        //delay(2000);
      }
    }
    motorControl.driver2State();
    verticleMove();
    //delay(2000);

    // Alternate direction
    fileHandler.forward = !fileHandler.forward;

    // Reset line buffer
    lineCount = 0;
  }
}