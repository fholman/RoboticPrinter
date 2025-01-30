void makeDot(int nozzle, long dotPause){
  int i = 0;
  digitalWrite(nozzle, HIGH);
  delayMicroseconds(1);
  //for(i=0; i<1; i++);
  digitalWrite(nozzle, LOW);
  delayMicroseconds(dotPause);
}
