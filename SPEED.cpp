#include "Speed.h"

extern QueueHandle_t encoder_count_queue;
TaskHandle_t speed_task_handle = NULL;
QueueHandle_t speed_queue = NULL;

uint8_t init_speed(){
    speed_queue = xQueueCreate(1,sizeof(int32_t));
    BaseType_t result_speed = xTaskCreatePinnedToCore(
        get_speed,
        "Speed_Calculation",
        2048,
        NULL,
        1,
        &speed_task_handle,
        CORE_1
    );
    if (result_speed == pdPASS){
        return 1;
    }
    else{
        return 0;
    }
}

void get_speed(void *pvParameters){
   int PPR = 205;
   int32_t contador_actual = 0;
   int32_t contador_anterior = 0;
   int32_t delta = 0;

   int rpm = 0;

   for(;;){
    if(xQueueReceive(encoder_count_queue,&contador_actual,portMAX_DELAY)==pdPASS){
        
        delta = contador_actual - contador_anterior;
        contador_anterior = contador_actual;
        
        rpm = (delta*60000)/(PPR*ms);
        xQueueOverwrite(speed_queue,&rpm);
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
   }


}

