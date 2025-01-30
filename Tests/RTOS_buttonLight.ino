// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>

// print 2 different tasks to serial

void Task_Controlled(void *param);
void Task_Print(void *param);

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  pinMode(8, INPUT_PULLUP);

  xTaskCreate(Task_Controlled, "Task 1", 100, NULL, 1, &Task_Handle1);
  xTaskCreate(Task_Print, "Task 2", 100, NULL, 1, &Task_Handle2);
}

void loop() {
  
}

void Task_Controlled(void *param) {

  (void) param;

  while(1) {
    if(digitalRead(8) == 0) {
      while(digitalRead(8) == 0)
      digitalWrite(13, HIGH);
      Serial.println("Button Pressed");
    } else digitalWrite(13, LOW);
    
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void Task_Print(void *param) {

  (void) param;

  while(1) {
    Serial.println("Task Print");
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}
