#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <Arduino.h>

#define stepPin1 2
#define dirPin1 3
#define stepPin2 5
#define dirPin2 4
#define stepPin3 7
#define dirPin3 6
#define stepPin4 9
#define dirPin4 8

void horizontalMove(int horizontalSteps);
void verticleMove(int verticleSteps);

#endif
