/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID2 "64f90866-d4bb-493d-bd01-e532e4e34021"
#define CHARACTERISTIC_UUID3 "48524cb9-9db9-4ce3-b263-85169799a6f3"

BLECharacteristic *pCharacteristic2;
BLECharacteristic *pCharacteristic3;

bool deviceConnected = false;

// Task handles
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

// Task 1: Debug terminal messages
void Task1(void *parameter) {
  while (true) {
    if (deviceConnected == true) {
      pCharacteristic2->setValue("5");
      pCharacteristic2->notify();
      Serial.println("Setting Value2");
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 1 second
  }
}

// Task 2: Recurrent Status Messages
void Task2(void *parameter) {
  while (true) {
    if (deviceConnected == true) {
      pCharacteristic3->setValue("Status");
      pCharacteristic3->notify();
      Serial.println("Setting Characteristic 3 Value");
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS); // Delay for 5 second
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
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

    String value = pCharacteristic->getValue();

    // write value to sd card

    // store [value, value+1, value+2]
    // one storage on flash memory

    //Serial.print(pCharacteristic->getValue());

    Serial.println(value.length());

    if (value.length() > 0) {
        Serial.println("Received data:");
        for (size_t i = 0; i < value.length(); i++) {
            Serial.printf("Byte %zu: 0x%02X\n", i, (unsigned char)value[i]);
        }
    }

    // String value = String(pCharacteristic->getValue().c_str());
    // Serial.println("Receiving data");
    // Serial.println(value);
    // Serial.println(pCharacteristic->getValue());
    // if (value.length() > 0) {
    //     Serial.println("Received data:");
    //     for (size_t i = 0; i < value.length(); i++) {
    //         Serial.printf("Byte %zu: 0x%02X\n", i, (unsigned char)value[i]);
    //     }
    // }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::deinit(true);

  BLEDevice::init("RoboPrinter");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);


  BLECharacteristic *pCharacteristic1 =
    pService->createCharacteristic(CHARACTERISTIC_UUID1, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic1->setCallbacks(new MyCallbacks());
  pCharacteristic1->setValue("First Characteristic");

  pCharacteristic2 =
    pService->createCharacteristic(CHARACTERISTIC_UUID2, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic2->addDescriptor(new BLE2902());
  pCharacteristic2->setValue("Second Characteristic");

  pCharacteristic3 =
    pService->createCharacteristic(CHARACTERISTIC_UUID3, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic3->addDescriptor(new BLE2902());
  pCharacteristic3->setValue("Second Characteristic");


  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::setMTU(255);
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

  // Create Task 1
  xTaskCreate(
    Task1,                  // Function to be executed
    "Debug Terminal Task",       // Name of the task
    2048,                   // Stack size in words
    NULL,                   // Task input parameter
    1,                      // Priority of the task
    &Task1Handle            // Task handle
  );

  // Create Task 2
  xTaskCreate(
    Task2,                  // Function to be executed
    "Status Task",    // Name of the task
    2048,                   // Stack size in words
    NULL,                   // Task input parameter
    1,                      // Priority of the task
    &Task2Handle            // Task handle
  );

  vTaskStartScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
