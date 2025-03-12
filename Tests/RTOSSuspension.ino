// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;

int counter1 = 0;
int counter2 = 0;

void setup() {
  Serial.begin(9600);

  xTaskCreate(Task_Print1, "Task 1", 100, NULL, 1, &Task_Handle1);
  xTaskCreate(Task_Print2, "Task 2", 100, NULL, 1, &Task_Handle2);
}

void loop() {
  
}

void Task_Print1(void *param) {

  (void) param;

  TickType_t getTick;
  getTick = xTaskGetTickCount();  //this getTick will get time from system tick from OS
  vTaskDelayUntil(&getTick, 1000 / portTICK_PERIOD_MS);

  while(1) {
    counter2++;
    Serial.println("Task 1");

    vTaskSuspend(Task_Handle2);
    // taskENTER_CRITICAL();
    for(int i = 0; i < 11; i++){
      Serial.print(i);
      Serial.print(" ");
      delay(200);
    }
    Serial.println();
    vTaskResume(Task_Handle2);
    // taskEXIT_CRITICAL();

    // if(counter2 == 3) {
    //   vTaskSuspend(Task_Handle2);
    // } else if(counter2 == 6) {
    //   vTaskResume(Task_Handle2);
    // }
    
    //vTaskDelay(1000/portTICK_PERIOD_MS);
    vTaskDelayUntil(&getTick, 1000 / portTICK_PERIOD_MS);
  }
}

void Task_Print2(void *param) {

  (void) param;

  while(1) {
    counter1++;
    Serial.println("Task 2");
      // if(counter1 == 10) {
      //   vTaskDelete(Task_Handle2);
      // }
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}