#ifndef PRINTHEADCONTROL_H
#define PRINTHEADCONTROL_H

#include <Arduino.h>

class PrintheadControl {

  private:
    const uint8_t nozzlePins[12] = {30, 28, 22, 24, 26, 35, 36, 37, 34, 33, 32, 31};

    const uint8_t dotPause = 5;
    const int dotPauseLong = 2000;

  public:
    void printheadSetUp();
    void makeDot(uint8_t nozzle);

}

#endif
