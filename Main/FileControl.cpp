#include "FileControl.h"

void processSDFile() {
  myFile = SD.open("FileName.txt");
  if (!myFile) {
    Serial.println("error opening FileName.txt for reading");
    return;
  }

  // Buffer to hold file lines
  String lines[12];
  int lineCount = 0;
  int dotPause = 6;
  int horizontalSteps = 700;
  int verticleSteps = 700;

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
          horizontalMove(int horizontalSteps);
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
          horizontalMove(int horizontalSteps);
        }
      }
      verticleMove(int verticleSteps);

      // Alternate direction
      forward = !forward;

      // Reset line buffer
      lineCount = 0;
    }
  }

  myFile.close();
  Serial.println("\nProcessing Complete.");
}
