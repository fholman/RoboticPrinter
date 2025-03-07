#include "FileControl.h"

// // Grid and other code below swap between for testing without SD card
const int rows = 36;
const int cols = 22;
int rowIndex = 0;
const char grid[rows][cols + 1]; // = {
//   "110000010110010",
//   "101110100100101",
//   "101011110100011",
//   "011111110110010",
//   "010110010101110",
//   "001100000100110",
//   "100100000010110",
//   "011000011010011",
//   "000100010111100",
//   "000101011011110",
//   "111000011010100",
//   "001011101010110",
  
//   "011101010001100",
//   "101011010100011",
//   "110101101011000",
//   "001010111001010",
//   "101101111100011",
//   "100000001101100",
//   "001100000010111",
//   "100100101010011",
//   "110110000011101",
//   "000010011010000",
//   "001011101101100",
//   "101010111011110",
  
//   "000010011010000",
//   "001011101101100",
//   "101010111011110"
// };

void processSDFile() {
  // Buffer to hold file lines
  String lines[12];
  int lineCount = 0;
  static bool forward = true;

  Serial.println("Processing dataFile.txt:");

  // //Code to assign all lines in file to matching grid size
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

  // File dataFile = SD.open("gridData.txt");
  // if (!dataFile) {
  //   Serial.println("Error opening grid file!");
  //   return;
  // }

  for (int i = 0; i < rows; i++) {
    
    lines[lineCount++] = grid[i];

  // Read the file line by line
  // while (dataFile.available()) {
  //   String currentLine = dataFile.readStringUntil('\n');
  //   lines[lineCount++] = currentLine;

    // If we've collected 12 lines or reached the end of the file
    // if (lineCount == 12 || !dataFile.available()) {
    if (lineCount == 12 || i == rows - 1) { 

      if (forward) {
        driver2.shaft(true);
        // Read column by column from the first to the last
        for (size_t col = 0; col < lines[0].length(); col++) {
          for (int row = 0; row < lineCount; row++) {
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
              makeDot(row);
            }
          }
          horizontalMove();
        }
      } else {
        driver2.shaft(false);
        // Read column by column from the last to the first
        for (int col = lines[0].length() - 1; col >= 0; col--) {
          for (int row = lineCount - 1; row >= 0; row--) {
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
              makeDot(row);
            }
          }
          horizontalMove();
        }
      }
      verticleMove();

      // Alternate direction
      forward = !forward;

      // Reset line buffer
      lineCount = 0;
    }
  }

  dataFile.close();
  Serial.println("\nProcessing Complete.");
}
