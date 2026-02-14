#include "Arduino.h"
#include "GUI.hpp"

extern HardwareSerial Serial;
// Declaramos queue externo que nos manda los datos desde el handle del PID
extern QueueHandle_t datos_gui_queue;
// Declaramos queue que mandara el setpoint desde el GUI hacia el PID
QueueHandle_t set_point_queue;

// Declaramos Task handle para nuestra tarea del GUI
TaskHandle_t gui_handle = NULL;

uint8_t init_GUI()
{
    Serial.setTimeout(10);
    // Declaramos queue con valor de la variable a mandar
    set_point_queue = xQueueCreate(1, sizeof(float));
    // Declaramos nuestro task con su respectiva funcion y core
    BaseType_t result_gui = xTaskCreatePinnedToCore(
        gui_task,
        "Task GUI",
        2048,
        NULL,
        Low_Priority,
        &gui_handle,
        CORE_1);

    // Condicion para asegurarnos que el task se inicializo correctamente
    if (result_gui == pdPASS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void gui_task(void *pvParameters)
{
    // Declaramos una variable con el struct a recibir los datos
    Datos_recibidos datos_recibidos = {0, 0, 0};
    // Declaramos variable con valor del setpoint que se recibe por UART
    float setpoint;
    // Creamos nuestro temporizador que realiza bloquea el task a su periodo
    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {

        // Revisamos constantemente si se mando el setpoint por puerto serial
        if (Serial.available() > 0)
        {
            String input = Serial.readStringUntil('\n');
            input.trim();

            if (input.length() > 0)
            {
                float new_sp = input.toFloat();
                xQueueOverwrite(set_point_queue, &new_sp);
            }
        }

        // Condicionamos para que la recepcion de datos
        if (xQueueReceive(datos_gui_queue, &datos_recibidos, 0) == pdPASS)
        {
            // Si recibimos datos, usamos el puerto serial para mandarlos de manera que se puedan decodificar en el .py
            Serial.print(datos_recibidos.PV);
            Serial.print(",");
            Serial.print(datos_recibidos.OP);
            Serial.print(",");
            Serial.print(datos_recibidos.Error);
            Serial.println();
        }
        // Bloqueamos la tarea a su periodo de 100ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}