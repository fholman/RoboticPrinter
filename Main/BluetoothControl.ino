void BluetoothStatus(void *param) {

  (void) param;

  vTaskDelay(1000/portTICK_PERIOD_MS); // 1 second delay on this split 
}
