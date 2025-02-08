#include "PrintheadControl.h"

void makeDot(int nozzle, long dotPause){
  int i = 0;
  digitalWrite(nozzles[nozzle], HIGH);
  delay(dotPause);
  digitalWrite(nozzles[nozzle], LOW);
  Serial.println("Nozzle " + String(nozzle) + " Activated");
  delay(1000);
}
