#include "Arduino.h" 
#include "Speed.hpp"
#include "Encoder.hpp" 

// Funciones externas como queues que declaramos en otras partes
extern HardwareSerial Serial;
extern QueueHandle_t encoder_count_queue; 

//Task y Queue de este handle
TaskHandle_t speed_task_handle = NULL;
QueueHandle_t speed_queue;

uint8_t init_speed(){
    // Creamos la queue con tamaño de la variable que vamos a mandar
    speed_queue = xQueueCreate(1, sizeof(int32_t));
    

    //Creamos la task del handler, con su correpondiente funcion, prioridad, y core del esp
    BaseType_t result_speed = xTaskCreatePinnedToCore(
        get_speed, 
        "Speed_Calculation", 
        4096, 
        NULL, 
        High_Priority,
        &speed_task_handle, 
        CORE_0
    );
    //usamos condicion para asegurarnos que el task se inicialice correctamente
    if(result_speed == pdPASS){
        return 1;
    }
    else{
        return 0;
    }
}

void get_speed(void *pvParameters){
    // Variables para calcular la velocidad
    int32_t rpm = 0;
    int32_t contador_actual = 0;
    int32_t contador_anterior = 0;
    int32_t delta = 0;
    const int PPR_EFECTIVOS = 205; 
    const uint8_t TIEMPO_MUESTREO_MS = 5; 
    
    // Bandera para evitar salto brusco en salida 
    bool primer_calculo = true; 

    //Nuestro temporizador para el bloqueo
    const int TARGET_US = 10000; // 10ms
    
    int64_t t_inicio = 0;
    int64_t t_fin = 0;
    int64_t t_anterior = esp_timer_get_time();
    
    // Inicialización del tiempo para vTaskDelayUntil
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for(;;){
        //Bloqueo del task para que se active solamente en su tiempo de periodo
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TIEMPO_MUESTREO_MS));

        t_inicio = esp_timer_get_time();
        int32_t periodo_real = (int32_t)(t_inicio - t_anterior);
        int32_t jitter = (int32_t)(periodo_real - TARGET_US);
        t_anterior = t_inicio;

        //Con un if, nos aseguramos que el calculo de la velocidad suceda solo cuando se recibe un dato
        if(xQueueReceive(encoder_count_queue, &contador_actual, 0) == pdPASS){
            
            //Condicion para igualar ambos estados del contador en el primer calculo para evitar errores
            if(primer_calculo){
                contador_anterior = contador_actual;
                primer_calculo = false;
            }

            //Obtenemos la diferencia entre los valores de contador anterior y actual
            delta = contador_actual - contador_anterior;
            
            // Actualizamos el valor recibido en el contador anterior para el proximo calculo
            contador_anterior = contador_actual;

            //Calculamos nuestro numerador y denominados.
            //Hecho por separado por la diferencia de tamaños y facil de debuggear
            int64_t numerador = (int64_t)delta * 6000;
            int32_t denominador = PPR_EFECTIVOS * TIEMPO_MUESTREO_MS;
            
            //Calculamos velocidad usando nuestro numerador y denominador ya calculados
            //Al ya ser el resultado, se castea el tipo de dato pues podemos usar uno de menor tamaño ahora
            rpm = (int16_t)(numerador / denominador);

            // Usamos QueueOverWrite para mandar la velocidad al handel del PID (se usa OverWrite por tener queues de tamaño de 1 en el arreglo)
            xQueueOverwrite(speed_queue, &rpm);
            
            // Revisión de que se reciben los pulsos y calculan las variables correctamente (para debuggeo)
            //Serial.printf("Pulsos: %d | Delta: %d | RPM: %d\n", contador_actual, delta, rpm);
        }
        else {
        }
        t_fin = esp_timer_get_time();
        int32_t latency = (int32_t)(t_fin - t_inicio);

        report_info(TASK_SPEED,jitter,latency, periodo_real);
    }
}