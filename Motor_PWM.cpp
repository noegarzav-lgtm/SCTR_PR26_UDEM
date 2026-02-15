#include "Motor_PWM.hpp"
#include "Arduino.h"

extern HardwareSerial Serial;
//Declaramos el queue del handler del PID que nos mandara el valor de manipulacion
extern QueueHandle_t PID_queue;

//Declaramos el task handle para nuestra aplicacion del pwm al motor 
TaskHandle_t PWM_handle = NULL;

uint8_t init_PWM(){
    //Declaramos nuestro task del PWM con su respectivo core y funcion a usar
    BaseType_t result_PWM = xTaskCreatePinnedToCore(
        send_PWM,
        "PWM_To_Motor",
        2048,
        NULL,
        Medium_Priority,
        &PWM_handle,
        CORE_1
    );
    //Usamos condicion para asegurarnos que el task se haya inicializado correctamente 
    if(result_PWM == pdPASS){
        return 1;
    }
    else{
        return 0;
    }
}

void send_PWM(void * pvParameters){
    //Configuramos los pines usados en el Puente-H para poder prender el motor
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(STBY, OUTPUT);
    pinMode(PWM, OUTPUT);
    //Variable que recibe la manipulacion por el queue
    float PWM_val = 0;
    //Nuestro temporizador para el bloqueo
    const int TARGET_US = 10000; // 10ms
    
    int64_t t_inicio = 0;
    int64_t t_fin = 0;
    int64_t t_anterior = esp_timer_get_time();


    TickType_t xLastWakeTime = xTaskGetTickCount();
    //Activamos los pines del motor
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2,LOW);
    digitalWrite(STBY,HIGH);

    for(;;){
        //Bloqueo del task para que se realice cada 10ms
        vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(10));
        t_inicio = esp_timer_get_time();
        int32_t periodo_real = (int32_t)(t_inicio - t_anterior);
        int32_t jitter = (int32_t)(periodo_real - TARGET_US);
        t_anterior = t_inicio;
        
        //Usamos condicion para que se aplique el PWM solo cuando se recibe manipulacion
        if(xQueueReceive(PID_queue, &PWM_val, 0) == pdPASS){
            //Usamos analogWrite para aplicar la señal PWM (manipulacion) en su respectivo pin
            analogWrite(PWM, PWM_val);
            //Impresion de que si se realizo la actividad (para debuggeo)
            //Serial.printf("Señal PWM mandada!");
        }

        t_fin = esp_timer_get_time();
        int32_t latency = (int32_t)(t_fin - t_inicio);

        report_info(TASK_PWM,jitter,latency,periodo_real);

    }
}