#include "Motor_PWM.hpp"
#include "Arduino.h"

extern HardwareSerial Serial;
extern QueueHandle_t PID_queue;
TaskHandle_t PWM_handle = NULL;

uint8_t init_PWM(){
    BaseType_t result_PWM = xTaskCreatePinnedToCore(
        send_PWM,
        "PWM_To_Motor",
        2048,
        NULL,
        Medium_Priority,
        &PWM_handle,
        CORE_1
    );

    if(result_PWM == pdPASS){
        return 1;
    }
    else{
        return 0;
    }
}

void send_PWM(void * pvParameters){
    float PWM_val = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2,LOW);

    for(;;){
        if(xQueueReceive(PID_queue, &PWM_val, portMAX_DELAY) == pdPASS){
            analogWrite(PWM, PWM_val);
            Serial.printf("Señal PWM mandada!");
        }
        vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(10));
    }
}