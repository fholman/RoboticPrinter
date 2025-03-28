#include "PrintheadControl.h"

void PrintheadControl::printheadSetUp(){
  for(int i = 0; i < 12; i++){
    pinMode(nozzlePins[i], OUTPUT);
    digitalWrite(nozzlePins[i], LOW);
  }
  //Serial.println("Printhead set up");
}

void PrintheadControl::makeDot(uint8_t nozzle){
  digitalWrite(nozzlePins[nozzle], HIGH);
  delayMicroseconds(dotPause);
  digitalWrite(nozzlePins[nozzle], LOW);
  //Serial.println("Nozzle " + String(nozzle) + " Activated");
  delayMicroseconds(dotPauseLong);
}