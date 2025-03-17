#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

//#include <Arduino.h>
#include <TMC2208Stepper.h>

extern const int stepPin1;
extern const int stepPin2;
extern const int stepPin3;
extern const int stepPin4;

extern TMC2208Stepper driver;
extern TMC2208Stepper driver2;

void horizontalMove();
void verticleMove();

#endif
