#ifndef BLUETOOTHCONTROL_H
#define BLUETOOTHCONTROL_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>
#include "FileControl.h"

class BluetoothControl {

private:

  FileControl& file;

  BLECharacteristic *pCharacteristic2;
  BLECharacteristic *pCharacteristic3;

  bool deviceConnected = false;
  bool isImageReceived = false;

  uint8_t batteryPercent;

  uint32_t expectedBytes;

  // need to be clear what each print status means! Need to have a rethink on this variable
  // = -1 means there is no print loaded on the SD card
  // = 0 means there is a print loaded on the SD card but it is not currently printing so can be overidden
  // 1 - 100 means print is in progress (where 1 means just started and 100 means complete)
  int16_t printStatus = -1;

  // 0 = stopped
  // 1 = paused
  // 2 = play
  uint8_t appStatus = 0;

  uint16_t widthOfImage;
  uint16_t heightOfImage;
  uint32_t countBytes;

  SemaphoreHandle_t bluetoothMutex;

  void setupBluetooth();
  String hexToBinary(unsigned char byteValue);

  static BluetoothControl* instance;


public:
  BluetoothControl(FileControl& f) : file(f) {
    bluetoothMutex = xSemaphoreCreateMutex();
    heightOfImage = file.readHeight();
    setupBluetooth();
  }

  void debugTask(String msg);
  void statusMessages(int status);
  void setAppStatus();
  void setBatteryPercentage();

  bool getDeviceConnected() const { return deviceConnected; }
  bool getIsImageReceived() const { return isImageReceived; }
  int16_t getPrintStatus() const { return printStatus; }
  uint8_t getAppStatus() const { return appStatus; }
  uint16_t getTotalRows() const { return heightOfImage; }

  void updatePrintProgress(int percentage);

  class MyServerCallbacks : public BLEServerCallbacks {
    private:
        BluetoothControl* parent; // Pointer to parent class

    public:
        MyServerCallbacks(BluetoothControl* parentInstance) : parent(parentInstance) {}

        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
  };

  class ImageData : public BLECharacteristicCallbacks {
    private:
      BluetoothControl* parent; // Pointer to parent class
    
    public:
      ImageData(BluetoothControl* parentInstance) : parent(parentInstance) {}

      void onWrite(BLECharacteristic *pCharacteristic) override;
  };

  class statusOfPrint : public BLECharacteristicCallbacks {
    private:
      BluetoothControl* parent; // Pointer to parent class
    
    public:
      statusOfPrint(BluetoothControl* parentInstance) : parent(parentInstance) {}

      void onWrite(BLECharacteristic *pCharacteristic) override;
  };

  class imageInfo : public BLECharacteristicCallbacks {
    private:
      BluetoothControl* parent; // Pointer to parent class
    
    public:
      imageInfo(BluetoothControl* parentInstance) : parent(parentInstance) {}

      void onWrite(BLECharacteristic *pCharacteristic) override;
  };

};

#endif
