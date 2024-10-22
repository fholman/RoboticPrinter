#ifndef main_declarations_h
#define main_declarations_h

// packages
#include <Arduino.h>

// port assignment
#define bluetooth_RX PB_11
#define bluetooth_TX PB_10
HardwareSerial BluetoothSerial(bluetooth_RX, bluetooth_TX);

int read_bluetooth(char);
void check_bluetooth();

// code here

#endif