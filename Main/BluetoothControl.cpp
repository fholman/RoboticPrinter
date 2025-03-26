#include "BluetoothControl.h"
#include "FileControl.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a8" // used to read image data
#define CHARACTERISTIC_UUID2 "64f90866-d4bb-493d-bd01-e532e4e34021" // used to send debug messages, used to receive pre and post image details
#define CHARACTERISTIC_UUID3 "48524cb9-9db9-4ce3-b263-85169799a6f3" // used to send status messages and receive play, pause, and print messages

String BluetoothControl::hexToBinary(unsigned char byteValue) {
  String binary = "";
  // Loop through each bit
  for (int i = 7; i >= 0; i--) {
    // Check if the bit is set (1) or not (0) and append '1' or '0'
    binary += (byteValue & (1 << i)) ? '1' : '0';
  }
  return binary;  // Return the 8-bit binary representation as a string
}

void BluetoothControl::debugTask(String msg) {
    if (deviceConnected == true) {
        xSemaphoreTake(bluetoothMutex, portMAX_DELAY);
        pCharacteristic2->setValue(msg);
        pCharacteristic2->notify();
        xSemaphoreGive(bluetoothMutex);
    }
}

void BluetoothControl::statusMessages() {
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

void BluetoothControl::MyServerCallbacks::onConnect(BLEServer* pServer) {
  parent->deviceConnected = true;
  Serial.println("Device connected!");
}

void BluetoothControl::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
  parent->deviceConnected = false;
  Serial.println("Device disconnected!");
  BLEDevice::startAdvertising(); // Restart advertising after disconnection
}

void BluetoothControl::ImageData::onWrite(BLECharacteristic *pCharacteristic) {
  String value = pCharacteristic->getValue();

  xSemaphoreTake(parent->bluetoothMutex, portMAX_DELAY);

  Serial.println(value.length());

  if (value.length() > 0) {
      String allBinaryData = "";
      Serial.println("Received data:");
      for (size_t i = 0; i < value.length(); i++) {
        Serial.printf("Byte %zu: 0x%02X\n", i, (unsigned char)value[i]);
        String binaryValue = parent->hexToBinary((unsigned char)value[i]);
        allBinaryData += binaryValue;
        parent->countBytes += 1;

        if (parent->countBytes % parent->widthOfImage == 0) {
          allBinaryData += "\n";
        }
      }
      // appendFile("/newImage.txt", allBinaryData.c_str());
      allBinaryData = "";
  }

  xSemaphoreGive(parent->bluetoothMutex);
}

void BluetoothControl::statusOfPrint::onWrite(BLECharacteristic *pCharacteristic) {
  xSemaphoreTake(parent->bluetoothMutex, portMAX_DELAY);

  String value = pCharacteristic->getValue();

  Serial.println((unsigned char)value[0]);

  if ((unsigned char)value[0] == 0) { // pause
    Serial.println("Got a zero");
    parent->additionToPrintStatus = 0;
  }
  else if ((unsigned char)value[0] == 1) { // play
    Serial.println("Got a one");
    parent->additionToPrintStatus = 5;
  }
  else if ((unsigned char)value[0] == 2) { // stop
    Serial.println("Got a two");
    parent->printStatus = 0;
  }

  xSemaphoreGive(parent->bluetoothMutex);
}



void BluetoothControl::imageInfo::onWrite(BLECharacteristic *pCharacteristic) {
  xSemaphoreTake(parent->bluetoothMutex, portMAX_DELAY);

    String value = pCharacteristic->getValue();

    uint32_t expectedBytes;

    if (value.length() == 6) {
      // writeFile("/newImage.txt", "");
      parent->countBytes = 0;
          // Convert first 4 bytes into a 32-bit integer
      expectedBytes = ((uint32_t)(unsigned char)value[0] << 24) |
                            ((uint32_t)(unsigned char)value[1] << 16) |
                            ((uint32_t)(unsigned char)value[2] << 8) |
                            ((uint32_t)(unsigned char)value[3]);

      // Convert last 2 bytes into a 16-bit integer
      parent->widthOfImage = ((uint16_t)(unsigned char)value[4] << 8) |
                            ((uint16_t)(unsigned char)value[5]);

      parent->heightOfImage = expectedBytes / parent->widthOfImage;
    }
    else if (value.length() == 1) {
      if (parent->countBytes == expectedBytes) {
        Serial.println("Success!");
      }
      else {
        Serial.println("Failure");
      }
      Serial.println(parent->countBytes);
      Serial.println(expectedBytes);
      parent->countBytes = 0;
    }

    xSemaphoreGive(parent->bluetoothMutex);
}

void BluetoothControl::setupBluetooth() {
  BLEDevice::deinit(true);

  BLEDevice::init("RoboPrinter");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks(this));
  BLEService *pService = pServer->createService(SERVICE_UUID);


  BLECharacteristic *pCharacteristic1 =
    pService->createCharacteristic(CHARACTERISTIC_UUID1, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic1->setCallbacks(new ImageData(this));
  pCharacteristic1->setValue("First Characteristic");

  pCharacteristic2 =
    pService->createCharacteristic(CHARACTERISTIC_UUID2, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic2->addDescriptor(new BLE2902());
  pCharacteristic2->setCallbacks(new imageInfo(this));
  pCharacteristic2->setValue("Second Characteristic");

  pCharacteristic3 =
    pService->createCharacteristic(CHARACTERISTIC_UUID3, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic3->addDescriptor(new BLE2902());
  pCharacteristic3->setCallbacks(new statusOfPrint(this));
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