#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

void TaskBacaSensor(void *pvParameters);
void TaskSendCloud(void *pvParameters);

void setup() {
  Serial.begin(9600);

  xTaskCreatePinnedToCore(
    TaskBacaSensor, "baca_sensor_dari_slave"
    , 
    8000 //stack size
    ,
    NULL, 2 //Priority
    ,
    NULL, ARDUINO_RUNNING_CORE);

  xTaskCreatePinnedToCore(
    TaskSendCloud, "kirim_data_mqtt"
    , 
    8000 //stack size
    ,
    NULL, 2 //Priority
    ,
    NULL, ARDUINO_RUNNING_CORE);
  
}

void loop() {

}

void TaskBacaSensor(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    //Task
    vTaskDelay(100);
  }
}

void TaskSendCloud(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    //Task
    vTaskDelay(100);
  }
}
