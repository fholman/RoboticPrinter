#include "PrintheadControl.h"

void PrintheadControl::printheadSetUp(){
  gpio_reset_pin(GPIO_NUM_39);
  gpio_reset_pin(GPIO_NUM_40);

  for(int i = 0; i < 12; i++){
    pinMode(nozzlePins[i], OUTPUT);
    digitalWrite(nozzlePins[i], LOW);
  }
}

void PrintheadControl::makeDot(uint8_t nozzle){
  digitalWrite(nozzlePins[nozzle], HIGH);
  delayMicroseconds(dotPause);
  digitalWrite(nozzlePins[nozzle], LOW);
  delayMicroseconds(dotPauseLong);
}