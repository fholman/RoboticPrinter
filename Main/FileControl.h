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

class FileControl {

  private:
  int lineCount;

  void writeFile(fs::FS &fs, const char *path, const char *message);
  void appendFile(fs::FS &fs, const char *path, const char *message);

  public:
    bool forward;
    // bool runOnce;
    String lines[12];

    FileControl();
    void setupSD();
    int processSDFile();
    void writeFile(const char* path, const char* message);
    void appendFile(const char* path, const char* message);
};

#endif
