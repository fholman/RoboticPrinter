#ifndef main_declarations_h
#define main_declarations_h

// packages
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// port assignment
#define bluetooth_RX PB_11
#define bluetooth_TX PB_10
HardwareSerial BluetoothSerial(bluetooth_RX, bluetooth_TX);

//Needs pin assignment
#define NOZZLE_01
#define NOZZLE_02
#define NOZZLE_03
#define NOZZLE_04
#define NOZZLE_05
#define NOZZLE_06
#define NOZZLE_07
#define NOZZLE_08
#define NOZZLE_09
#define NOZZLE_10
#define NOZZLE_11
#define NOZZLE_12

#define STEP_PAUSE       1000
#define COMMAND_PAUSE       5
#define DOT_PAUSE_LONG   1000
#define DOT_PAUSE_SHORT     5

int i;
char comByte = 0;
char dataMode = 0;
long dotWidth = 11;
long lineHeight = 234;
long stepsDone = 0;

int nozzles[] = {NOZZLE_06, NOZZLE_07, NOZZLE_08, NOZZLE_09, NOZZLE_12, NOZZLE_11, NOZZLE_10, NOZZLE_03, NOZZLE_02, NOZZLE_01, NOZZLE_04, NOZZLE_05};

void motorStep(long, int, int, long);
void makeStep(int, long);

File myFile;

void writeToSD();
void readFromSD();

void read_bluetooth();
void check_bluetooth();

// code here

#endif