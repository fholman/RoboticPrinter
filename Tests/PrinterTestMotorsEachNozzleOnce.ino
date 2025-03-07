#define stepPin1 1
#define dirPin1 2
#define stepPin2 3
#define dirPin2 4

// Define the 0/1 grid as a 2D array
// const int rows = 27;
// const int cols = 15;
const int rows = 12;
const int cols = 12;

const char grid[rows][cols + 1] = {
  "000000000001",
  "000000000010",
  "000000000100",
  "000000001000",
  "000000010000",
  "000000100000",
  "000001000000",
  "000010000000",
  "000100000000",
  "001000000000",
  "010000000000",
  "100000000000"
  // "110000010110010",
  // "001110100100101",
  // "101011110100011",
  // "011111110110010",
  // "010110010101110",
  // "001100000100110",
  // "100100000010110",
  // "011000011010011",
  // "000100010111100",
  // "000101011011110",
  // "111000011010100",
  // "001011101010110",
  
  // "011101010001100",
  // "101011010100011",
  // "110101101011000",
  // "001010111001010",
  // "101101111100011",
  // "100000001101100",
  // "001100000010111",
  // "100100101010011",
  // "110110000011101",
  // "000010011010000",
  // "001011101101100",
  // "101010111011110",
  
  // "000010011010000",
  // "001011101101100",
  // "101010111011110"
};

const int nozzlePins[12] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

void setup() {
  Serial.begin(9600);

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
  int dotPause = 0.003;
  
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
              makeDot((row + 1), dotPause);
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
              makeDot((row + 1), dotPause);
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

void makeDot(int nozzle, long dotPause){
  digitalWrite(nozzlePins[nozzle], HIGH);
  delay(dotPause);
  digitalWrite(nozzlePins[nozzle], LOW);
  Serial.println("Nozzle " + String(nozzle) + " Activated");
  delay(1000);
}

void horizontalMove() {
  Serial.println("Motor move");
  for(int x = 0; x < 100; x++) {
    digitalWrite(stepPin1,HIGH); 
    digitalWrite(stepPin2,HIGH); 
    delayMicroseconds(700);    // by changing this time delay between the steps we can change the rotation speed
    digitalWrite(stepPin1,LOW); 
    digitalWrite(stepPin2,LOW); 
    delay(20); 
  }
}
