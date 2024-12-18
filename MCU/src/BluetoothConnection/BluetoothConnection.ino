/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool deviceConnected = false;

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

  BLEDevice::init("RoboPrinter");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  
  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World says Neil");
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
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}
