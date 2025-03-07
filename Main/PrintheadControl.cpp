#include "PrintheadControl.h"

void makeDot(uint8_t nozzle){
  digitalWrite(nozzlePins[nozzle], HIGH);
  delayMicroseconds(dotPause);
  digitalWrite(nozzlePins[nozzle], LOW);
  Serial.println("Nozzle " + String(nozzle) + " Activated");
  delayMicroseconds(dotPauseLong);
}