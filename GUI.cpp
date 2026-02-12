#include "Arduino.h"
#include "GUI.hpp"

extern HardwareSerial Serial;
extern QueueHandle_t datos_gui_queue;
QueueHandle_t set_point_queue;
TaskHandle_t gui_handle = NULL;

uint8_t init_GUI(){
    set_point_queue = xQueueCreate(1, sizeof(float));

    BaseType_t result_gui = xTaskCreatePinnedToCore(
        gui_task,
        "Task GUI",
        2048,
        NULL,
        Low_Priority,
        &gui_handle,
        CORE_1
    );

    if(result_gui == pdPASS){
        return 1;
    }
    else{
        return 0;
    }
}

void gui_task(void * pvParameters){
    Datos_recibidos datos_recibidos = {0,0,0};
    float setpoint;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for(;;){
        if(Serial.available()){
            setpoint = Serial.parseFloat();
            xQueueOverwrite(set_point_queue,&setpoint);
        }

        if(xQueueReceive(datos_gui_queue, &datos_recibidos, 0) == pdPASS){
            Serial.print("PV:");
            Serial.print(datos_recibidos.PV);
            Serial.print("OP:");
            Serial.print(datos_recibidos.OP);
            Serial.print("Error:");
            Serial.print(datos_recibidos.Error);
        }
        vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(100));
    }
}