#include "FileControl.h"

FileControl::FileControl()
  : lineCount(0), forward(true), filePosition(0) {
    fileMutex = xSemaphoreCreateMutex();
  }

void FileControl::setupSD() {
  SPI.begin(sck, miso, mosi, cs);
  if(!SD.begin(cs)) {
      // Serial.println("Card Mount Failed");
      while (1);
  }
  if(SD.cardType() == CARD_NONE) {
      // Serial.println("No SD card attached");
      while (1);
  }
}

void FileControl::resetFilePosition() {
  filePosition = 0;
}

bool FileControl::isFileNotEmpty() {
    xSemaphoreTake(fileMutex, portMAX_DELAY);
    File file = SD.open("/image.txt");  // Open the file

    if (!file) {
        // Serial.println("Failed to open grid.txt or file does not exist.");
        xSemaphoreGive(fileMutex);
        return false;  // File does not exist or cannot be opened
    }

    if (file.available()) {  
      char firstChar = file.read();  // Read first character
      file.close();
      
      if (firstChar != -1) {  // If file has content
        xSemaphoreGive(fileMutex);
        return true;
      }
    }

    file.close();
    // Serial.println("grid.txt is empty.");
    xSemaphoreGive(fileMutex);
    return false;  // File exists but is empty
}

int FileControl::readHeight() {
  return readHeight(SD);
}

int FileControl::readHeight(fs::FS &fs) {
  File file = fs.open("/imageHeight.txt");
  if (!file) {
    return 0;
  }

  String heightStr = file.readStringUntil('\n');
  file.close();

  return heightStr.toInt();
}

void FileControl::writeFile(const char* path, const char* message) {
    writeFile(SD, path, message);
}

void FileControl::writeFile(fs::FS& fs, const char* path, const char* message) {
  xSemaphoreTake(fileMutex, portMAX_DELAY);
  // Serial.printf("Writing file: %s\n", path);
  File file = fs.open(path, FILE_WRITE);
  if(!file) {
      // Serial.println("Failed to open file for writing");
      return;
  }
  file.print(message);
  // file.print(message) ? Serial.println("File written") : Serial.println("Write failed");
  file.close();
  xSemaphoreGive(fileMutex);
}

/*
Only for appending
*/
void FileControl::openFile(fs::FS& fs, const char* path, int mode) {
  xSemaphoreTake(fileMutex, portMAX_DELAY);
  if (mode == 0) {
    fileToAppend = fs.open(path, FILE_APPEND);
  }
  else if (mode == 1) {
    fileToAppend = fs.open(path, FILE_READ);
  }
  xSemaphoreGive(fileMutex);
}

void FileControl::openFile(const char* path, int mode) {
  openFile(SD, path, mode);
}

void FileControl::closeFile() {
  xSemaphoreTake(fileMutex, portMAX_DELAY);
  fileToAppend.close();
  xSemaphoreGive(fileMutex);
}

void FileControl::appendFile(const char* path, const char* message) {
  appendFile(SD, path, message);
}

void FileControl::appendFile(fs::FS& fs, const char* path, const char* message) {
  xSemaphoreTake(fileMutex, portMAX_DELAY);
  // Serial.printf("Appending to file: %s\n", path);
  if(!fileToAppend) {
      // Serial.println("Failed to open file for appending");
      return;
  }
  fileToAppend.print(message);
  // fileToAppend.print(message) ? Serial.println("Message appended") : Serial.println("Append failed");
  xSemaphoreGive(fileMutex);
}

int FileControl::processSDFile() {
  // Serial.println("Getting 12 lines from file");
  lineCount = 0;

  closeFile();

  openFile("/image.txt", 1);

  xSemaphoreTake(fileMutex, portMAX_DELAY);

  if(!fileToAppend) {
    xSemaphoreGive(fileMutex);
    return 0;
  }

  fileToAppend.seek(filePosition);

  // Read file line by line
  while(fileToAppend.available() && lineCount < 12) {
    if(fileToAppend.available()) {
      String line = fileToAppend.readStringUntil('\n');  // Read one line from the file
      line.trim();  // Trim any extra spaces or newline characters

      // Skip any empty lines
      if(line.length() == 0) {
        continue;
      }

      lines[lineCount] = line;  // Store the line in the lines buffer
      lineCount++;
    }
  }

  filePosition = fileToAppend.position();

  xSemaphoreGive(fileMutex);

  closeFile();

  return lineCount;
}