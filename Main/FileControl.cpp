#include "FileControl.h"

FileControl::FileControl()
  : lineCount(0), forward(true) {}

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

int FileControl::processSDFile() {
  lineCount = 0;

  File dataFile = SD.open("gridData.txt");
  if (!dataFile) {
    Serial.println("Error opening grid file!");
    return lineCount;
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
  }
  dataFile.close();
  return lineCount;
}