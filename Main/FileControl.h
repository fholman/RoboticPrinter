#ifndef FILECONTROL_H
#define FILECONTROL_H

//#include <Arduino.h>
//#include <SD.h>
#include "FS.h"
#include "SD.h"
// #include <SD_MMC.h>
#include "SPI.h"
#include "MotorControl.h"
#include "PrintheadControl.h"
//#include <Arduino_FreeRTOS.h>

class FileControl {

  private:
    int lineCount;
    File fileToAppend;
    int filePosition;

    const int cs = 41;
    const int sck = 47;
    const int miso = 48;
    const int mosi = 42;

    SemaphoreHandle_t fileMutex;

    void openFile(fs::FS& fs, const char* path, int mode);
    void writeFile(fs::FS &fs, const char *path, const char *message);
    int readHeight(fs::FS &fs);
    void appendFile(fs::FS &fs, const char *path, const char *message);

  public:
    bool forward;
    // bool runOnce;
    String lines[12];

    FileControl();
    void resetFilePosition();
    void setupSD();
    int processSDFile();
    bool isFileNotEmpty();
    int readHeight();
    void openFile(const char* path, int mode);
    void closeFile();
    void writeFile(const char* path, const char* message);
    void appendFile(const char* path, const char* message);
};

#endif
