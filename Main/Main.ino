#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// port assignment
#define bluetooth_RX PB_11
#define bluetooth_TX PB_10
HardwareSerial BluetoothSerial(bluetooth_RX, bluetooth_TX);

//Needs pin assignment
#define NOZZLE_01 
#define NOZZLE_02 
#define NOZZLE_03 
#define NOZZLE_04 
#define NOZZLE_05 
#define NOZZLE_06 
#define NOZZLE_07 
#define NOZZLE_08 
#define NOZZLE_09 
#define NOZZLE_10 
#define NOZZLE_11 
#define NOZZLE_12 

#define stepPin1 2
#define dirPin1 3
#define stepPin2 5
#define dirPin2 4
#define stepPin3 7
#define dirPin3 6
#define stepPin4 9
#define dirPin4 8

TaskHandle_t Task_Main;
TaskHandle_t Task_Status;

int nozzles[] = {NOZZLE_06, NOZZLE_07, NOZZLE_08, NOZZLE_09, NOZZLE_12, NOZZLE_11, NOZZLE_10, NOZZLE_03, NOZZLE_02, NOZZLE_01, NOZZLE_04, NOZZLE_05};

int dotPause = ;      //Needs filled !!!

File myFile;

//Sd Card
int sck = 4;
int miso = 5;
int mosi = 6;
int cs = 7;


void setup() {
  
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  Serial.print("\nInitializing SD card...");
  if (!SD.begin(CS Pin Number)) { //Needs changed <------
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");


  BluetoothSerial.begin(460800);

  for(i=0; i<12; i++){
    pinMode(nozzles[i], OUTPUT);
  }

  pinMode(stepPin1,OUTPUT); 
  pinMode(dirPin1,OUTPUT);
  pinMode(stepPin2,OUTPUT); 
  pinMode(dirPin2,OUTPUT);
  pinMode(stepPin3,OUTPUT); 
  pinMode(dirPin3,OUTPUT);
  pinMode(stepPin4,OUTPUT); 
  pinMode(dirPin4,OUTPUT);

  digitalWrite(dirPin1,HIGH); 
  digitalWrite(dirPin2,LOW); 
  digitalWrite(dirPin3,HIGH); 
  digitalWrite(dirPin4,LOW); 

  xTaskCreate(MainFunctions, "Main", 100, NULL, 1, &Task_Main);
  xTaskCreate(BluetoothStatus, "Status", 100, NULL, 2, &Task_Status);
}


void loop() {

}

void MainFunctions(void *param) {

  (void) param;

  // Call functions to begin
  
}
