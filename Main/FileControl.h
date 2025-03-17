#ifndef FILECONTROL_H
#define FILECONTROL_H

//#include <Arduino.h>
//#include <SD.h>
#include "FS.h"
#include "SD.h"
// #include <SD_MMC.h>
// #include "SPI.h"
#include "MotorControl.h"
#include "PrintheadControl.h"
//#include <Arduino_FreeRTOS.h>


extern TaskHandle_t Task_Status;

void setupSD();

void processSDFile();

void writeFile(const char *path, const char *message);

void writeFile(fs::FS &fs, const char *path, const char *message);

void appendFile(const char *path, const char *message);

void appendFile(fs::FS &fs, const char *path, const char *message);

#endif
