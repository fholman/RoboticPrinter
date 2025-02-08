#include "BluetoothControl.h"

void BluetoothStatus(void *param) {

  (void) param;

  while(1) {
    vTaskDelay(1000/portTICK_PERIOD_MS); // 1 second delay on this split 
  }
}
