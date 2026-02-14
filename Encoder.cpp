#include "Arduino.h"
#include "driver/gpio.h"
#include "Encoder.hpp"


//Definimos el quque a usar para mandar el conteo de pulsos al handle de velocidad
QueueHandle_t encoder_count_queue;

//Definimos el handle del task de enviar el valor del contador
TaskHandle_t encoder_handle = NULL;
// variable volatil para compartir contador entre funciones
volatile int32_t global_contador = 0;

//Mutex para proteger la variable del contador global
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


void IRAM_ATTR leerEncoder() {
    // Leer los pines del encoder
    int estadoA = digitalRead(PIN_A);
    int estadoB = digitalRead(PIN_B);
    
    //Protegemos el calculo del contador
    portENTER_CRITICAL_ISR(&timerMux);
    
    // 3. Lógica de conteo (Tu lógica original estaba bien, pero debía ir aquí)
    if (estadoA == HIGH) {
        if (estadoB == LOW) global_contador++;
        else global_contador--;
    } else {
        if (estadoB == HIGH) global_contador++;
        else global_contador--;
    }
    
    //Salimos de la zona protegida
    portEXIT_CRITICAL_ISR(&timerMux);
}

void encoder_task(void *pvParameters)
{
    //delcaramos variable de contador local
    int32_t contador_local;
    const TickType_t xDelay = 100 / portTICK_PERIOD_MS; 

    for(;;)
    {
        //Leemos el valor del contador global usando el mutex para hacerlo de manera segura
        portENTER_CRITICAL(&timerMux);
        contador_local = global_contador;
        portEXIT_CRITICAL(&timerMux);

        //Enviamos el valor del contador al handler de velocidad a traves de queue
        xQueueOverwrite(encoder_count_queue, &contador_local);

        // Imprimimos valor entregado del contador (para debuggeo)
        // Serial.printf("Contador: %d\n", contador_local);
        vTaskDelay(xDelay);
    }
}

uint8_t init_encoder()
{
    // Queue de tamaño 1 porque solo nos interesa el valor más reciente
    encoder_count_queue = xQueueCreate(1, sizeof(int32_t));

    // Activamos el pullup de los pines de lectura de pulsos
    pinMode(PIN_A, INPUT_PULLUP);
    pinMode(PIN_B, INPUT_PULLUP);

    // configuramos interrupcion que se activa con cambios en el pulso del channel A
    attachInterrupt(digitalPinToInterrupt(PIN_A), leerEncoder, CHANGE);

    // Creamos la tarea con su respectiva funcion y core
    BaseType_t result_encoder = xTaskCreatePinnedToCore(
                            encoder_task,
                            "Encoder_Task",
                            2048,
                            NULL,
                            High_Priority,
                            &encoder_handle,
                            CORE_0); 
    
    //Condicion para asegurarnos que la task se inicializo correctamente
    if(result_encoder == pdPASS){
        return 1;
    }else{
        return 0;
    }
}
