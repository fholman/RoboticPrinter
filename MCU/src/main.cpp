#include "main_declarations.h"


void motorStep(long stepWidth, int dirPin, int stepPin, long stepPause){
  if(stepWidth < 0){
    digitalWrite(dirPin, HIGH);
    stepWidth = -stepWidth;
  }
  else{
    digitalWrite(dirPin, LOW);
  }
  for(long i=0; i<stepWidth; i++){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(COMMAND_PAUSE);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(stepPause);
  }
}

void makeDot(int nozzle, long dotPause){
  int i = 0;
  digitalWrite(nozzle, HIGH);
  delayMicroseconds(1);
  //for(i=0; i<1; i++);
  digitalWrite(nozzle, LOW);
  delayMicroseconds(dotPause);
}


void setup() {
  Serial.begin(9600);
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

  dataMode = 0;
  stepsDone = 0;

  pinMode(LED_1, OUTPUT);
  pinMode(SWITCH_01, INPUT);
  digitalWrite(SWITCH_01, HIGH); // Activate pull-up resistor

  pinMode(DIR_X, OUTPUT);
  pinMode(STEP_X, OUTPUT);
  pinMode(DIR_Y, OUTPUT);
  pinMode(STEP_Y, OUTPUT);

  for(i=0; i<12; i++){
    pinMode(nozzles[i], OUTPUT);
  }

  motorStep(100, DIR_X, STEP_X, STEP_PAUSE);
  motorStep(-100, DIR_X, STEP_X, STEP_PAUSE);
  motorStep(100, DIR_Y, STEP_Y, STEP_PAUSE);
  motorStep(-100, DIR_Y, STEP_Y, STEP_PAUSE);

  // start serial port at 115200 bps:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  check_bluetooth();

  comByte = 0;
  if (Serial.available() > 0){//if valid, read from serial:
    //get incoming byte:
    comByte = Serial.read();
    Serial.print('R');   // send 'R' to initiate next data from computer

    if(dataMode == 0){

      if(comByte == 'd'){
        dataMode = 1;
      }

      if(comByte == 'r'){// Return line
        motorStep(-stepsDone, DIR_X, STEP_X, STEP_PAUSE/10);
        stepsDone = 0;
      }
      if(comByte == 'n'){// New line
        motorStep(-lineHeight, DIR_Y, STEP_Y, STEP_PAUSE/10);
        //motorStep(-dotWidth, DIR_X, STEP_X, STEP_PAUSE/10);//only why printhead wasn't othogonal
        stepsDone = 0;
      }
      if(comByte == 'N'){// Forward paper for one dot
        motorStep(-lineHeight/12, DIR_Y, STEP_Y, STEP_PAUSE/10);
        stepsDone = 0;
      }


      if(comByte == 'T'){// Test nozzles
        for(int i = 0; i < 12; i++){
          makeDot(nozzles[i], DOT_PAUSE_LONG);
          delay(2000);
        }
      }

      if(comByte == 't'){// Test nozzles
        for(int i = 0; i < 12; i++){
          for(int j = 0; j < 100; j++){
            makeDot(nozzles[i], DOT_PAUSE_LONG);
            motorStep(dotWidth, DIR_X, STEP_X, DOT_PAUSE_LONG/dotWidth + 10);
          }
        }
        motorStep(-lineHeight, DIR_Y, STEP_Y, STEP_PAUSE/4);
        for(int i = 0; i < 12; i++){
          for(int j = 0; j < 100; j++){
            makeDot(nozzles[i], DOT_PAUSE_LONG);
            motorStep(-dotWidth, DIR_X, STEP_X, DOT_PAUSE_LONG/dotWidth + 10);
          }
        }
      }

      if(comByte == 'x'){
        motorStep(100, DIR_X, STEP_X, STEP_PAUSE);
      }
      if(comByte == 'X'){
        motorStep(-100, DIR_X, STEP_X, STEP_PAUSE);
      }
      if(comByte == 'y'){
        motorStep(100, DIR_Y, STEP_Y, STEP_PAUSE);
      }
      if(comByte == 'Y'){
        motorStep(-100, DIR_Y, STEP_Y, STEP_PAUSE);
      }
    }
    else{
      if(dataMode == 1){
        for(int i = 0; i < 8; i++){
          if(comByte & (1<<i)){
            makeDot(nozzles[i], DOT_PAUSE_SHORT);
          }
        }
      }
      if(dataMode == 2){
        for(int i = 0; i < 4; i++){
          if(comByte & (1<<i)){
            makeDot(nozzles[i+8], DOT_PAUSE_SHORT);
          }
        }
      }
      dataMode++;
      if(dataMode == 3){
        motorStep(dotWidth, DIR_X, STEP_X, DOT_PAUSE_LONG/dotWidth + 10);
        stepsDone+=dotWidth;
        dataMode = 0;
      }
    }
  }
}

void writeToSD(String data) {
  // Open the file for writing
  myFile = SD.open("FileName.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to FileName.txt");
    // myFile.println("testing 1, 2, 3."); For Testing
    myFile.println(data);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening FileName.txt for writing");
  }
}

void readFromSD() {
  // Open the file for reading
  myFile = SD.open("FileName.txt");
  if (myFile) {
    Serial.println("Reading from FileName.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening FileName.txt for reading");
  }
}

// put function definitions here:
void read_bluetooth() {
  String data = "";
  while (BluetoothSerial.available()) {
    char c = BluetoothSerial.read();
    if (c == '\n') { // Check for newline character as end of message
      writeToSD(data); // Write the collected data to SD card
      data = ""; // Clear the data string for the next message
    } else {
      data += c; // Append character to data string
    }
  }
}

void check_bluetooth() {
  if (BluetoothSerial.available()) {
    char c = BluetoothSerial.read();
    if (c == 2) {
      read_bluetooth(c);
    }
  }
}

void processSDFile() {
  myFile = SD.open("FileName.txt");
  if (!myFile) {
    Serial.println("error opening FileName.txt for reading");
    return;
  }

  // Buffer to hold file lines
  String lines[12];
  int lineCount = 0;

  Serial.println("Processing FileName.txt:");

  // Read the file line by line
  while (myFile.available()) {
    String currentLine = myFile.readStringUntil('\n');
    lines[lineCount++] = currentLine;

    // If we've collected 12 lines or reached the end of the file
    if (lineCount == 12 || !myFile.available()) {
      // Determine direction (alternate between forward and reverse every 12 rows)
      static bool forward = true;

      if (forward) {
        // Read column by column from the first to the last
        for (size_t col = 0; col < lines[0].length(); col++) {
          for (int row = 0; row < lineCount; row++) {
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.print(lines[row + 1][col + 1]); // Process the character
              makeDot(lines[row + 1], dotPause);
            }
          }
        }
      } else {
        // Read column by column from the last to the first
        for (int col = lines[0].length() - 1; col >= 0; col--) {
          for (int row = lineCount - 1; row >= 0; row--) {
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.print(lines[row + 1][col + 1]); // Process the character
              makeDot(lines[row + 1], dotPause);
            }
          }
        }
      }

      // Alternate direction
      forward = !forward;

      // Reset line buffer
      lineCount = 0;
    }
  }

  myFile.close();
  Serial.println("\nProcessing Complete.");
}