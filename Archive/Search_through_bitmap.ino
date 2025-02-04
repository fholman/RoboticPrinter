// Define the 0/1 grid as a 2D array
const int rows = 27;
const int cols = 15;
const char grid[rows][cols + 1] = {
  "110000010110010",
  "101110100100101",
  "101011110100011",
  "011111110110010",
  "010110010101110",
  "001100000100110",
  "100100000010110",
  "011000011010011",
  "000100010111100",
  "000101011011110",
  "111000011010100",
  "001011101010110",
  
  "011101010001100",
  "101011010100011",
  "110101101011000",
  "001010111001010",
  "101101111100011",
  "100000001101100",
  "001100000010111",
  "100100101010011",
  "110110000011101",
  "000010011010000",
  "001011101101100",
  "101010111011110",
  
  "000010011010000",
  "001011101101100",
  "101010111011110"
};

void setup() {
  Serial.begin(9600);
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
            if (lines[row][col] == '1') {
              // Log the location of a '1'
              Serial.print("1 found at (Row: ");
              Serial.print(totalRows + row + 1);
              Serial.print(", Col: ");
              Serial.print(col + 1);
              Serial.println(")");
            }
          }
        }
      } else {
        // Read column by column from the last to the first
        for (int col = cols - 1; col >= 0; col--) {
          for (int row = 0; row < lineCount; row++) {
            if (lines[row][col] == '1') {
              // Log the location of a '1'
              Serial.print("1 found at (Row: ");
              Serial.print(totalRows + row + 1);
              Serial.print(", Col: ");
              Serial.print(col + 1);
              Serial.println(")");
            }
          }
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
