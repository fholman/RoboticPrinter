#include "BluetoothControl.h"
#include "FileControl.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>

// #include "FS.h"
// #include <SD_MMC.h>
// #include "SPI.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a8" // used to read image data
#define CHARACTERISTIC_UUID2 "64f90866-d4bb-493d-bd01-e532e4e34021" // used to send debug messages, used to receive pre and post image details
#define CHARACTERISTIC_UUID3 "48524cb9-9db9-4ce3-b263-85169799a6f3" // used to send status messages and receive play, pause, and print messages

BLECharacteristic *pCharacteristic2;
BLECharacteristic *pCharacteristic3;

bool deviceConnected = false;

uint8_t batteryPercent = 100;

// need to be clear what each print status means!
// = -1 means there is no print loaded on the SD card
// = 0 means there is a print loaded on the SD card but it is not currently printed so can be overidden
// 1 - 100 means print is in progress (where 1 means just started and 100 means complete)
int16_t printStatus = -1;
uint8_t additionToPrintStatus = 0;

uint16_t widthOfImage;
uint16_t heightOfImage;
uint32_t expectedBytes;
uint32_t countBytes;

// Task handles
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

SemaphoreHandle_t bluetoothMutex = xSemaphoreCreateMutex();


void debugTask(String msg) {
    if (deviceConnected == true) {
        xSemaphoreTake(bluetoothMutex, portMAX_DELAY);
        pCharacteristic2->setValue(msg);
        pCharacteristic2->notify();
        xSemaphoreGive(bluetoothMutex);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 2 second
}

void statusMessages() {
    if (deviceConnected == true) {
        xSemaphoreTake(bluetoothMutex, portMAX_DELAY);
        String msg = String(batteryPercent) + "," + String(printStatus);
        pCharacteristic3->setValue(msg);
        pCharacteristic3->notify();
        Serial.println("Setting Characteristic 3 Value");
        printStatus += additionToPrintStatus;
        batteryPercent -= 1;
        xSemaphoreGive(bluetoothMutex);
    }
}

// Server Callbacks to handle connection and disconnection
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected!");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected!");
    BLEDevice::startAdvertising(); // Restart advertising after disconnection
  }
};

//Callback for handling incoming data
class ImageData : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

    String value = pCharacteristic->getValue();

    xSemaphoreTake(bluetoothMutex, portMAX_DELAY);

    Serial.println(value.length());

    if (value.length() > 0) {
        String allBinaryData = "";
        Serial.println("Received data:");
        for (size_t i = 0; i < value.length(); i++) {
          Serial.printf("Byte %zu: 0x%02X\n", i, (unsigned char)value[i]);
          String binaryValue = hexToBinary((unsigned char)value[i]);
          allBinaryData += binaryValue;
          countBytes += 1;

          if (countBytes % widthOfImage == 0) {
            allBinaryData += "\n";
          }
        }
        // appendFile("/newImage.txt", allBinaryData.c_str());
        allBinaryData = "";
    }

    xSemaphoreGive(bluetoothMutex);
  }
};

String hexToBinary(unsigned char byteValue) {
  String binary = "";
  // Loop through each bit
  for (int i = 7; i >= 0; i--) {
    // Check if the bit is set (1) or not (0) and append '1' or '0'
    binary += (byteValue & (1 << i)) ? '1' : '0';
  }
  return binary;  // Return the 8-bit binary representation as a string
}

//Callback for handling incoming data
class statusOfPrint : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

    xSemaphoreTake(bluetoothMutex, portMAX_DELAY);

    String value = pCharacteristic->getValue();

    Serial.println((unsigned char)value[0]);

    if ((unsigned char)value[0] == 0) {
      additionToPrintStatus = 0;
    }
    else if ((unsigned char)value[0] == 1) {
      additionToPrintStatus = 5;
    }
    else if ((unsigned char)value[0] == 2) {
      printStatus = 0;
    }

    xSemaphoreGive(bluetoothMutex);
  }
};

//Callback for handling incoming data
class imageInfo : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

    xSemaphoreTake(bluetoothMutex, portMAX_DELAY);

    String value = pCharacteristic->getValue();

    if (value.length() == 6) {
      // writeFile("/newImage.txt", "");
      countBytes = 0;
          // Convert first 4 bytes into a 32-bit integer
      expectedBytes = ((uint32_t)(unsigned char)value[0] << 24) |
                            ((uint32_t)(unsigned char)value[1] << 16) |
                            ((uint32_t)(unsigned char)value[2] << 8) |
                            ((uint32_t)(unsigned char)value[3]);

      // Convert last 2 bytes into a 16-bit integer
      widthOfImage = ((uint16_t)(unsigned char)value[4] << 8) |
                            ((uint16_t)(unsigned char)value[5]);

      heightOfImage = expectedBytes / widthOfImage;
    }
    else if (value.length() == 1) {
      if (countBytes == expectedBytes) {
        Serial.println("Success!");
      }
      else {
        Serial.println("Failure");
      }
      Serial.println(countBytes);
      Serial.println(expectedBytes);
      countBytes = 0;
    }

    xSemaphoreGive(bluetoothMutex);
  }
};

void setupBluetooth() {
  BLEDevice::deinit(true);

  BLEDevice::init("RoboPrinter");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);


  BLECharacteristic *pCharacteristic1 =
    pService->createCharacteristic(CHARACTERISTIC_UUID1, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic1->setCallbacks(new ImageData());
  pCharacteristic1->setValue("First Characteristic");

  pCharacteristic2 =
    pService->createCharacteristic(CHARACTERISTIC_UUID2, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic2->addDescriptor(new BLE2902());
  pCharacteristic2->setCallbacks(new imageInfo());
  pCharacteristic2->setValue("Second Characteristic");

  pCharacteristic3 =
    pService->createCharacteristic(CHARACTERISTIC_UUID3, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic3->addDescriptor(new BLE2902());
  pCharacteristic3->setCallbacks(new statusOfPrint());
  pCharacteristic3->setValue("Third Characteristic");


  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::setMTU(400);
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}