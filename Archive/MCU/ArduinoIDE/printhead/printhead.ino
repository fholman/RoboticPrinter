#define NOZZLE_01     26
#define NOZZLE_02     28
#define NOZZLE_03     30
#define NOZZLE_04     32
#define NOZZLE_05     34
#define NOZZLE_06     36
#define NOZZLE_07     38
#define NOZZLE_08     40
#define NOZZLE_09    42
#define NOZZLE_10    44
#define NOZZLE_11    46
#define NOZZLE_12    48

int nozzles[] = {NOZZLE_06, NOZZLE_07, NOZZLE_08, NOZZLE_09, NOZZLE_12, NOZZLE_11, NOZZLE_10, NOZZLE_03, NOZZLE_02, NOZZLE_01, NOZZLE_04, NOZZLE_05};

#define STEP_PAUSE       1000
#define COMMAND_PAUSE       5
#define DOT_PAUSE_LONG   1000
#define DOT_PAUSE_SHORT     5

void makeDot(int nozzle, long dotPause){
  int i = 0;
  digitalWrite(nozzle, HIGH);
  delayMicroseconds(1);
  //for(i=0; i<1; i++);
  digitalWrite(nozzle, LOW);
  delayMicroseconds(dotPause);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for(int i=0; i<12; i++){
    pinMode(nozzles[i], OUTPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("About to Print....5 seconds....");
  delay(5000);
  for(int i = 0; i < 12; i++){
    Serial.println("First Nozzle Print " + String(i) + " " + String(nozzles[i]));
    makeDot(nozzles[i], DOT_PAUSE_LONG);
    delay(10000);
  }
  Serial.println("Finished Printing all Nozzels");
  delay(5000);
}
