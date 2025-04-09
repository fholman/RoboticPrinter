#include "BluetoothControl.h"

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

void BluetoothControl::statusMessages(int status) {
  if (deviceConnected == true) {
    xSemaphoreTake(bluetoothMutex, portMAX_DELAY);
    setBatteryPercentage();
    // Serial.println("Status Message");
    String msg = String(batteryPercent) + "," + String(printStatus) + "," + String(status);
    pCharacteristic3->setValue(msg);
    pCharacteristic3->notify();
    xSemaphoreGive(bluetoothMutex);
  }
}

void BluetoothControl::setAppStatus() {
  appStatus = 3;
}

void BluetoothControl::setBatteryPercentage() {
  int pin = A10;
  float Vref = 3.3;
  float R1 = 1000000.0;
  float R2 = 100000.0;
  float Vmin = 3.5;  // Min battery voltage
  float Vmax = 4.2;  // Max battery voltage
  float gammaMax = 3.0;
  float gammaMin = 0.5;

  int adcValue = analogRead(pin);
  float Vout = (adcValue / 1023.0) * Vref;  // Output voltage after voltage divider
  float Vbattery = Vout * (R1 + R2) / R2;  // Reversed voltage divider equation

  float gamma = gammaMin + (gammaMax - gammaMin) * (Vbattery - Vmin) / (Vmax - Vmin);
  float batteryPercentage = pow((Vbattery - Vmin) / (Vmax - Vmin), gamma) * 100;

  if (batteryPercentage < 0) batteryPercentage = 0;
  if (batteryPercentage > 100) batteryPercentage = 100;

  batteryPercent = batteryPercentage;
}

void BluetoothControl::MyServerCallbacks::onConnect(BLEServer* pServer) {
  parent->deviceConnected = true;
  // Serial.println("Device connected!");
}

void BluetoothControl::MyServerCallbacks::onDisconnect(BLEServer* pServer) {
  parent->deviceConnected = false;
  // Serial.println("Device disconnected!");
  BLEDevice::startAdvertising(); // Restart advertising after disconnection
}

void BluetoothControl::ImageData::onWrite(BLECharacteristic *pCharacteristic) {
  if (parent->printStatus < 1) {
    String value = pCharacteristic->getValue();

    // Serial.println(value.length());

    if (value.length() > 0) {
      String allBinaryData = "";
      // Serial.println("Received data:");
      for (size_t i = 0; i < value.length(); i++) {
        // Serial.printf("Byte %zu: 0x%02X\n", i, (unsigned char)value[i]);
        String binaryValue = parent->hexToBinary((unsigned char)value[i]);
        allBinaryData += binaryValue;
        parent->countBytes += 1;

        if (parent->countBytes % parent->widthOfImage == 0) {
          allBinaryData += "\n";
        }
      }
      parent->file.appendFile("/image.txt", allBinaryData.c_str());
      allBinaryData = "";
    }

    // Serial.println("End Of Statement");

    // xSemaphoreGive(parent->bluetoothMutex);
  }
}

void BluetoothControl::statusOfPrint::onWrite(BLECharacteristic *pCharacteristic) {
  if (parent->printStatus > -1) {
    xSemaphoreTake(parent->bluetoothMutex, portMAX_DELAY);

    String value = pCharacteristic->getValue();

    //Serial.println((unsigned char)value[0]);

    if ((unsigned char)value[0] == 0) { // pause
      parent->appStatus = 1;
    }
    else if ((unsigned char)value[0] == 1) { // play
      parent->appStatus = 2;
    }
    else if ((unsigned char)value[0] == 2) { // stop
      parent->appStatus = 0;
      parent->printStatus = 0;
    }

    xSemaphoreGive(parent->bluetoothMutex);
  }
}

void BluetoothControl::imageInfo::onWrite(BLECharacteristic *pCharacteristic) {
  if (parent->printStatus < 1) {
    // xSemaphoreTake(parent->bluetoothMutex, portMAX_DELAY);

    String value = pCharacteristic->getValue();

    parent->file.closeFile();

    if (value.length() == 6) {
      parent->updatePrintProgress(-1);
      parent->statusMessages(2);
      xSemaphoreTake(parent->bluetoothMutex, portMAX_DELAY);
      parent->file.writeFile("/image.txt", "");
      parent->countBytes = 0;
          // Convert first 4 bytes into a 32-bit integer
      parent->expectedBytes = ((uint32_t)(unsigned char)value[0] << 24) |
                            ((uint32_t)(unsigned char)value[1] << 16) |
                            ((uint32_t)(unsigned char)value[2] << 8) |
                            ((uint32_t)(unsigned char)value[3]);

      // Convert last 2 bytes into a 16-bit integer
      parent->widthOfImage = ((uint16_t)(unsigned char)value[4] << 8) |
                            ((uint16_t)(unsigned char)value[5]);

      parent->heightOfImage = parent->expectedBytes / parent->widthOfImage;

      parent->file.openFile("/image.txt", 0);
    }
    else if (value.length() == 1) {
      xSemaphoreGive(parent->bluetoothMutex);
      // Serial.println("Expected");
      // Serial.println(parent->expectedBytes);
      // Serial.println("Counted");
      // Serial.println(parent->countBytes);
      if (parent->countBytes == parent->expectedBytes) {
        parent->isImageReceived = true;
        parent->printStatus = 0;
        parent->file.writeFile("/imageHeight.txt", String(parent->heightOfImage).c_str());
        // Serial.println("Success!");
      }
      else {
        // Serial.println("Failure");
        parent->file.writeFile("/image.txt", "");
        parent->file.writeFile("/imageHeight.txt", "0");
      }
      // Serial.println(parent->countBytes);
      // Serial.println(expectedBytes);
      parent->countBytes = 0;
    }

    // xSemaphoreGive(parent->bluetoothMutex);
  }
}

void BluetoothControl::updatePrintProgress(int percentage) {
  printStatus = percentage;
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
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::setMTU(500);
  // Serial.println("MTU SET");
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  // Serial.println("Characteristic defined! Now you can read it in your phone!");
}