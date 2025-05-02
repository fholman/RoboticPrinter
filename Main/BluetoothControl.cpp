#include "BluetoothControl.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a8" // used to read image data
#define CHARACTERISTIC_UUID2 "64f90866-d4bb-493d-bd01-e532e4e34021" // used to send debug messages, used to receive pre and post image details
#define CHARACTERISTIC_UUID3 "48524cb9-9db9-4ce3-b263-85169799a6f3" // used to send status messages and receive play, pause, and print messages

String BluetoothControl::hexToBinary(unsigned char byteValue) {
  char binary[9]; // 8 bits + null terminator
  for (int i = 7; i >= 0; i--) {
    binary[7 - i] = (byteValue & (1 << i)) ? '1' : '0';
  }
  binary[8] = '\0';
  return String(binary); // Construct once
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
    // setBatteryPercentage();
    // Serial.println("Status Message");
    String msg = String(batteryPercent) + "," + String(printStatus) + "," + String(status);
    pCharacteristic3->setValue(msg);
    pCharacteristic3->notify();
    xSemaphoreGive(bluetoothMutex);
  }
}

void BluetoothControl::setAppStatus() {
  xSemaphoreTake(bluetoothMutex, portMAX_DELAY);
  appStatus = 3;
  xSemaphoreGive(bluetoothMutex);
}

void BluetoothControl::setBatteryPercentage() {
  // Uncomment gamma parts for battery percentage curves
  const int voltagePin = A10;
  const float Vref = 3.3;  // Can read voltage of 3.3V rail with voltage divider then divide by 1024. Should be 3.3V but could use 5V like arduino systems use
  const float Vmax = 12.6;
  const float Vmin = 9;

  // float gammaMax = 3.0;
  // float gammaMin = 0.5;

  int adcValue = 0;
  float voltage = 0;
  float batteryPercentage = 0;

  adcValue = analogRead(voltagePin); // Voltage from 0 to boards
  voltage = ((adcValue * Vref) / 1000) * 11;  // Divide by 1000 for Volts - * 11 for voltage divider

  batteryPercentage = ((voltage - Vmin) / (Vmax - Vmin)) * 100;

  // float gamma = gammaMin + (gammaMax - gammaMin) * (batteryPercentage / 100);
  // float trueBatteryPercentage = pow((batteryPercentage / 100), gamma) * 100;

  batteryPercent = batteryPercentage; // trueBatteryPercentage  // Value will be float value between 0.00 and 100.00
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

    if (value.length() > 0) {
      // Serial.println("Received data:");
      for (size_t i = 0; i < value.length(); i++) {
        // Serial.printf("Byte %zu: 0x%02X\n", i, (unsigned char)value[i]);
        String binaryValue = parent->hexToBinary((unsigned char)value[i]);
        parent->allBinaryData += binaryValue;
        parent->countBytes += 1;

        if (parent->countBytes % parent->widthOfImage == 0) {
          parent->allBinaryData += "\n";
        }
      }

      parent->file.appendFile("/image.txt", parent->allBinaryData.c_str());
      //parent->myTimer = ( parent->myTimer * (parent->counter - 1) + (micros() - parent->startTime) ) / parent->counter;
      parent->allBinaryData = "";
    }
  }
}

void BluetoothControl::statusOfPrint::onWrite(BLECharacteristic *pCharacteristic) {
  if (parent->printStatus > -1) {
    xSemaphoreTake(parent->bluetoothMutex, portMAX_DELAY);

    String value = pCharacteristic->getValue();

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

      parent->startTime = micros();
    }
    else if (value.length() == 1) {
      xSemaphoreGive(parent->bluetoothMutex);
      parent->debugTask("Bytes : " + String(parent->expectedBytes));
      parent->myTimer = micros() - parent->startTime;
      parent->debugTask("Time : " + String(parent->myTimer));
      parent->debugTask("Counter : " + String(parent->counter));
      size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);  // You can specify other capabilities
      parent->debugTask("Heap : " + String(freeHeap));
      // Serial.println("Expected");
      // Serial.println(parent->expectedBytes);
      // Serial.println("Counted");
      // Serial.println(parent->countBytes);
      if (parent->countBytes == parent->expectedBytes) {
        parent->isImageReceived = true;
        parent->printStatus = 0;
        parent->file.writeFile("/imageHeight.txt", String(parent->heightOfImage).c_str());
        parent->debugTask("SUCCESS!");
      }
      else {
        parent->debugTask("FAILED!");
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
  xSemaphoreTake(bluetoothMutex, portMAX_DELAY);
  printStatus = percentage;
  xSemaphoreGive(bluetoothMutex);
}

void BluetoothControl::setupBluetooth() {
  BLEDevice::deinit(true);

  BLEDevice::init("RoboPrinter");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks(this));
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic1 =
    pService->createCharacteristic(CHARACTERISTIC_UUID1, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
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
  BLEDevice::setMTU(450);
  // Serial.println("MTU SET");
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  // Serial.println("Characteristic defined! Now you can read it in your phone!");
}