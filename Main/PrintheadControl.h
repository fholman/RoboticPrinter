#ifndef PRINTHEADCONTROL_H
#define PRINTHEADCONTROL_H

#include <Arduino.h>

extern const uint8_t nozzlePins[12];

extern const uint8_t dotPause;
extern const int dotPauseLong;

void makeDot(uint8_t nozzle);

#endif
