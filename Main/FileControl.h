#ifndef FILECONTROL_H
#define FILECONTROL_H

#include <Arduino.h>
#include <SD.h>
#include "MotorControl.h"
#include <Arduino_FreeRTOS.h>

extern File dataFile;

extern TaskHandle_t Task_Status;

void makeDot(uint8_t nozzle);

void processSDFile();

#endif
