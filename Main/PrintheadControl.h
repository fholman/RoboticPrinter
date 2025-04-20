#ifndef PRINTHEADCONTROL_H
#define PRINTHEADCONTROL_H

#include <Arduino.h>

class PrintheadControl {

  private:
    const uint8_t nozzlePins[12] = {6, 5, 1, 2, 4, 38, 39, 40, 37, 36, 35, 7};

    const uint8_t dotPause = 4;
    const int dotPauseLong = 4;

  public:
    void printheadSetUp();
    void makeDot(uint8_t nozzle);

};

#endif
