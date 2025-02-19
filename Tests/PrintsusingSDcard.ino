#include <SD.h>
#include <SPI.h>

#define stepPin1 6
#define dirPin1 7
#define stepPin2 4
#define dirPin2 5

const int chipSelect = 53;
File dataFile;

// Define the 0/1 grid as a 2D array
const int rows = 36;
const int cols = 22;
int rowIndex = 0;
const char grid[rows][cols + 1];

const uint8_t nozzlePins[12] = {30, 28, 22, 24, 26, 35, 36, 37, 34, 33, 32, 31};

const uint8_t dotPause = 5;

void setup() {
  Serial.begin(9600);

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed.");
    while (true);
  }

  Serial.println("initialization done.");

  File dataFile = SD.open("gridData.txt");

  while (dataFile.available() && rowIndex < rows) {
    String line = dataFile.readStringUntil('\n');
    line.trim();

    if (line.length() == cols) {
      line.toCharArray(grid[rowIndex], cols + 1);
      rowIndex++;
    }
  }
  dataFile.close();

  for (int i = 0; i < rows; i++) {
    Serial.println(grid[i]);
  }

  // Initialize nozzle pins as OUTPUT
  for (int i = 0; i < 12; i++) {
    pinMode(nozzlePins[i], OUTPUT);
    digitalWrite(nozzlePins[i], LOW);
  }

  pinMode(stepPin1,OUTPUT); 
  pinMode(dirPin1,OUTPUT);
  pinMode(stepPin2,OUTPUT); 
  pinMode(dirPin2,OUTPUT);

  digitalWrite(dirPin1,HIGH); 
  digitalWrite(dirPin2,LOW);

  Serial.println("Processing Grid:");
  processGrid();
}

void loop() {
  // Nothing to do here
}

void processGrid() {
  
  String lines[12];
  int lineCount = 0;
  int totalRows = 0;
  
  for (int i = 0; i < rows; i++) {
    
    lines[lineCount++] = grid[i];

    if (lineCount == 12 || i == rows - 1) { 
      
      // Determine direction (alternate between forward and reverse every 12 rows)
      static bool forward = true;

      if (forward) {
        // Read column by column from the first to the last
        for (size_t col = 0; col < cols; col++) {
          for (int row = 0; row < lineCount; row++) {
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
              makeDot((row + 1));
            }
          }
          horizontalMove();
        }
      } else {
        // Read column by column from the last to the first
        for (int col = cols - 1; col >= 0; col--) {
          for (int row = 0; row < lineCount; row++) {
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
              makeDot((row + 1));
            }
          }
          horizontalMove();
        }
      }

      // Alternate direction
      forward = !forward;

      // Update the total row count and reset the line buffer
      totalRows += lineCount;
      lineCount = 0;
    }
  }

  Serial.println("Processing Complete.");
}

void makeDot(uint8_t nozzle){
  digitalWrite(nozzlePins[nozzle], HIGH);
  delayMicroseconds(dotPause);
  digitalWrite(nozzlePins[nozzle], LOW);
  Serial.println("Nozzle " + String(nozzle) + " Activated");
  delay(200);
}

void horizontalMove() {
  Serial.println("Motor move");
  for(int x = 0; x < 8; x++) {
    digitalWrite(stepPin1,HIGH); 
    digitalWrite(stepPin2,HIGH); 
    delayMicroseconds(700);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin1,LOW); 
    digitalWrite(stepPin2,LOW); 
    delay(40); 
  }
}
