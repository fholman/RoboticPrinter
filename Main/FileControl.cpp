#include "FileControl.h"

// // Grid and other code below swap between for testing without SD card
// const int rows = 27;
// const int cols = 15;    //REMEMBER TO CHANGE !!
// int rowIndex = 0;
// const char grid[rows][cols + 1]; // = { or ;
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

FileControl::FileControl(MotorControl* motorPtr)
  : motor(motorPtr), lineCount(0), forward(true) {}

void FileControl::setupSD() {
    if (!SD.begin()) {
        Serial.println("Card Mount Failed");
        while (1);
    }
    if (SD.cardType() == CARD_NONE) {
        Serial.println("No SD card attached");
        while (1);
    }
}

void FileControl::writeFile(const char* path, const char* message) {
    writeFile(SD, path, message);
}

void FileControl::writeFile(fs::FS& fs, const char* path, const char* message) {
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.print(message) ? Serial.println("File written") : Serial.println("Write failed");
    file.close();
}

void FileControl::appendFile(const char* path, const char* message) {
    appendFile(SD, path, message);
}

void FileControl::appendFile(fs::FS& fs, const char* path, const char* message) {
    Serial.printf("Appending to file: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    file.print(message) ? Serial.println("Message appended") : Serial.println("Append failed");
    file.close();
}

void FileControl::processSDFile() {
  int lineCount = 0;

  File dataFile = SD.open("gridData.txt");
  if (!dataFile) {
    Serial.println("Error opening grid file!");
    return;
  }

  Serial.println("Processing dataFile.txt:");

  // Read file line by line
  while (dataFile.available() || lineCount > 0) {
    if (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');  // Read one line from the file
      line.trim();  // Trim any extra spaces or newline characters

      // Skip any empty lines
      if (line.length() == 0) {
        continue;
      }

      lines[lineCount] = line;  // Store the line in the lines buffer
      lineCount++;
    }
  // for (int i = 0; i < rows; i++) {
   
  //   lines[lineCount++] = grid[i];

  // Read the file line by line
  // while (dataFile.available()) {
  //   String currentLine = dataFile.readStringUntil('\n');
  //   lines[lineCount++] = currentLine;

    // If we've collected 12 lines or reached the end of the file
    // if (lineCount == 12 || !dataFile.available()) {
    // if (lineCount == 12 || i == rows - 1) {
    if (lineCount == 12 || !dataFile.available()) {
      MotorControl.driver2State();

      if (forward) {
        MotorControl.swapDirection();
        // Read column by column from the first to the last
        for (size_t col = 0; col < lines[0].length(); col++) {
          taskENTER_CRITICAL(&myMutex);
          vTaskSuspend(Task_Status);
          for (int row = 0; row < lineCount; row++) {
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
              makeDot(row);
              //delay(500);
            }
          }
          vTaskResume(Task_Status);
          taskEXIT_CRITICAL(&myMutex);
          horizontalMove();
          //delay(2000);
        }
      } else {
        MotorControl.swapDirection();
        // Read column by column from the last to the first
        for (int col = lines[0].length() - 1; col >= 0; col--) {
          for (int row = lineCount - 1; row >= 0; row--) {
            vTaskSuspend(Task_Status);
            if (col < lines[row].length() && lines[row][col] == '1') {
              Serial.println("1 found in col " + String(col + 1) + " and row " + String(row + 1));
              makeDot(row);
              //delay(500);
            }
            vTaskResume(Task_Status);
          }
          horizontalMove();
          //delay(2000);
        }
      }
      MotorControl.driver2State();
      verticleMove();
      //delay(2000);

      // Alternate direction
      forward = !forward;

      // Reset line buffer
      lineCount = 0;
    }
  }

  dataFile.close();
  Serial.println("\nProcessing Complete.");
}