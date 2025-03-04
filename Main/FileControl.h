#ifndef FILECONTROL_H
#define FILECONTROL_H

#include <Arduino.h>
#include <SD.h>
#include "MotorControl.h"

extern File dataFile;

void makeDot(uint8_t nozzle);

void processSDFile();

#endif
