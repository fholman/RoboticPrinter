#ifndef BLUETOOTHCONTROL_H
#define BLUETOOTHCONTROL_H

#include <Arduino.h>

void debugTask(String msg);
void statusMessages();
String hexToBinary(unsigned char byteValue);
void setupBluetooth();

#endif
