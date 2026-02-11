#include "Encoder.hpp"
#include "Arduino.h"
#include "driver/gpio.h"

extern HardwareSerial Serial;
QueueHandle_t encoder_queue;
QueueHandle_t encoder_count_queue;
TaskHandle_t encoder_handle = NULL;


uint8_t init_encoder()
{
    encoder_queue = xQueueCreate(1, sizeof(bool));
    encoder_count_queue = xQueueCreate(1, sizeof(int32_t));

    BaseType_t result_encoder = xTaskCreatePinnedToCore(
                                        encoder_function,
                                        "Encoder_Pulses",
                                        2048,
                                        NULL,
                                        1,
                                        &encoder_handle,
                                        CORE_0); 

    if(result_encoder == pdPASS)
    {
        return 1;
    }
    else{
        return 0;
    }
}

void IRAM_ATTR leerEncoder() {
    bool flag = true;

    xQueueSendFromISR(encoder_queue, &flag, NULL);
}

void encoder_function(void *Parameter)
{
    bool flag;
    int32_t CONTADOR=0;
    attachInterrupt(digitalPinToInterrupt(PIN_A), leerEncoder, CHANGE);

    for(;;)
    {
        if (xQueueReceive(encoder_queue, &flag, portMAX_DELAY) == pdPASS)
        {
            int estadoA = digitalRead(PIN_A);
            int estadoB = digitalRead(PIN_B);

            if (estadoA == HIGH) {
                if (estadoB == LOW)
                    CONTADOR++;
                else
                    CONTADOR--;
            } else {
                if (estadoB == HIGH)
                    CONTADOR++;
                else
                    CONTADOR--;
            }
            Serial.print(CONTADOR);
            xQueueOverwrite(encoder_count_queue,&CONTADOR);
        }
    }
}
