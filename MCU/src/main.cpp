#include "main_declarations.h"

void setup() {
  // put your setup code here, to run once:
  BluetoothSerial.begin(460800);
}

void loop() {
  // put your main code here, to run repeatedly:
  check_bluetooth();
}

// put function definitions here:
int read_bluetooth(char c) {
  while (c != 3) {
    if (BluetoothSerial.available()) {
      char c = BluetoothSerial.read();
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