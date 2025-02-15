#include "PrintheadControl.h"

void makeDot(int nozzle, long dotPause){
  digitalWrite(nozzles[nozzle], HIGH);
  delayMicroseconds(dotPause);
  digitalWrite(nozzles[nozzle], LOW);
  Serial.println("Nozzle " + String(nozzle) + " Activated");
  delay(1000);
}
