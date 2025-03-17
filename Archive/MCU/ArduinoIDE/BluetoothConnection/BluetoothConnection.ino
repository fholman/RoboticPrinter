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
#define CHARACTERISTIC_UUID1 "beb5483e-36e1-4688-b7f5-ea07361b26a8" // used to read image data
#define CHARACTERISTIC_UUID2 "64f90866-d4bb-493d-bd01-e532e4e34021" // used to send debug messages
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

// Task handles
TaskHandle_t Task1Handle;
TaskHandle_t Task2Handle;

SemaphoreHandle_t bluetoothMutex = xSemaphoreCreateMutex();

// Task 1: Debug terminal messages
void Task1(void *parameter) {
  while (true) {
    if (deviceConnected == true) {
      xSemaphoreTake(bluetoothMutex, portMAX_DELAY);
      pCharacteristic2->setValue("5");
      pCharacteristic2->notify();
      Serial.println("Setting Value2");
      xSemaphoreGive(bluetoothMutex);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay for 2 second
  }
}

// Task 2: Recurrent Status Messages
void Task2(void *parameter) {
  while (true) {
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

    xSemaphoreTake(bluetoothMutex, portMAX_DELAY);

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

    printStatus = 0;

    xSemaphoreGive(bluetoothMutex);

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

//Callback for handling incoming data
class MyCallbacks1 : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

    xSemaphoreTake(bluetoothMutex, portMAX_DELAY);

    String value = pCharacteristic->getValue();

    //Serial.println(value.length());

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
  pCharacteristic3->setCallbacks(new MyCallbacks1());
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

}

void loop() {
  // put your main code here, to run repeatedly:
}
