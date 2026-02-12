#include "PID.hpp"
#include "Arduino.h"

extern HardwareSerial Serial;
QueueHandle_t PID_queue;
QueueHandle_t datos_gui_queue;
extern QueueHandle_t speed_queue;
extern QueueHandle_t set_point_queue;
TaskHandle_t PID_handle = NULL;

uint8_t init_PID(){
    PID_queue = xQueueCreate(1, sizeof(float));
    BaseType_t result_PID = xTaskCreatePinnedToCore(
        get_PID,
        "Calculo PID",
        2048,
        NULL,
        Medium_Priority,
        &PID_handle,
        CORE_1
    );

    if(result_PID == pdPASS){
        return 1;
    }
    else{
        return 0;
    }
}

void get_PID(void * pvParameters){
    PID_Ks PID_Ks = {0 ,0, 0}; //aqui se ajustan los valores del PID
    PID_vals PID_vals = {0,0,0};
    Datos_GUI Datos_GUI = {0,0,0};
    float dt = 10; //Periodo de muestreo (deadline)
    float PID = 0;
    float set_point = 0;
    float error = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    int rpm_actual = 0;
    float rpm_buffer[3];

    for(;;){
        //xQueueReceive(set_point_queue, &set_point, 0);
        if(xQueueReceive(speed_queue,&rpm_actual, portMAX_DELAY) == pdPASS){
            error = set_point - rpm_actual;
            //Calculo proporcional
            PID_vals.proporcional = error * PID_Ks.Kc;
            //Calculo integral
            PID_vals.integral = PID_vals.integral + error*dt*PID_Ks.Kc/PID_Ks.tao_i;
            if(PID_vals.integral > PID_MAX){
                PID_vals.integral = PID_MAX;
            }

            if(PID_vals.integral < -PID_MAX){
                PID_vals.integral = -PID_MAX;
            }
            //Calculo derivativa
            rpm_buffer[2] = rpm_buffer[1];
            rpm_buffer[1] = rpm_buffer[0];
            rpm_buffer[0] = rpm_actual;
            PID_vals.derivativa = (rpm_buffer[0] - rpm_buffer[2])*PID_Ks.Kc*PID_Ks.tao_d/(2*dt);

            //Calculo PID
            PID = PID_vals.proporcional + PID_vals.integral + PID_vals.derivativa;
            if(PID > PID_MAX){
                PID = PID_MAX;
            }

            if(PID < PID_MIN){
                PID = PID_MIN;
            }
            //Guardamos datos en el struct a enviar al GUI
            Datos_GUI.PV = rpm_actual;
            Datos_GUI.OP = PID;
            Datos_GUI.Error = error;
            xQueueOverwrite(datos_gui_queue,&Datos_GUI);
            xQueueOverwrite(PID_queue, &PID);
            Serial.printf("PID: ", PID);
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
    }
}