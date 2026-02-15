#include "PID.hpp"
#include "Arduino.h"

extern HardwareSerial Serial;
// Declaramos el queue que viene desde el handler de la velocidad y el handler del GUI para el setpoint
extern QueueHandle_t speed_queue;
extern QueueHandle_t set_point_queue;
extern QueueHandle_t pid_ks_queue;

//Declaramos queues para mandar el PID al handler del motor y los datos al GUI
QueueHandle_t PID_queue;
QueueHandle_t datos_gui_queue;

//Declaramos el task handle de nuestra tarea de calculo de PID
TaskHandle_t PID_handle = NULL;

uint8_t init_PID(){
    //Creamos las queues para mandar la manipulacion y datos del GUI, cada una con tamaño de el dato a enviar
    PID_queue = xQueueCreate(1, sizeof(float));
    datos_gui_queue = xQueueCreate(1, sizeof(Datos_GUI));

    //Creamos el task del PID con su respectiva funcion y CORE
    BaseType_t result_PID = xTaskCreatePinnedToCore(
        get_PID,
        "Calculo PID",
        4096,
        NULL,
        Medium_Priority,
        &PID_handle,
        CORE_1
    );

    //Condicion para revisar que el task se inicializo correctamente
    if(result_PID == pdPASS){
        return 1;
    }else{
        return 0;
    }
}

void get_PID(void * pvParameters){
    //Declaramos nuestro struct de los valores de sintonizacion, aqui cambias la sintonizacion
    PID_Ks PID_Ks = {0,0,0}; 
    
    //Declaramos otros structs para usar y mandar variables
    PID_vals PID_vals = {0, 0, 0};
    Datos_GUI Datos_GUI = {0, 0, 0, 0};
    
    //declaramos el tiempo de muestreo, que definimos de 10 ms en base al periodo
    const float dt = 0.01; 
    
    // Variables para calcular la manipulacion
    float PID_Output = 0;
    float set_point = 0;
    float error_actual = 0;
    float error_anterior = 0; 

    // Variables para calcular la jitter y latencia
    int64_t t_inicio = 0;
    int64_t t_fin = 0;
    int64_t t_anterior = esp_timer_get_time();
    
    //Nuestro temporizador para el bloqueo
    const int TARGET_US = 10000; // 10ms
    //Declaramos dos rpm actual, uno para recibir en el tamño de dato correcto, el otro para usar en calculo
    int32_t rpm_actual_int = 0; 
    float rpm_actual = 0;      
    
    // Declaramos reloj para llevar cuenta del tiempo con ticks
    TickType_t xLastWakeTime = xTaskGetTickCount();

    for(;;){
        // Bloqueo de tarea para realizarla con periodo de 10ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));

        t_inicio = esp_timer_get_time();
        int32_t periodo_real = (int32_t)(t_inicio - t_anterior);
        int32_t jitter = (int32_t)(periodo_real - TARGET_US);
        t_anterior = t_inicio;

        //usamos QueueRecieve para ver si recibimos el setpoint nuevo desde el GUI
        xQueueReceive(set_point_queue, &set_point, 0);
        xQueueReceive(pid_ks_queue, &PID_Ks,0);

        //Usamos if para asegurarnos de calcular el PID solo cuando se recibe la velocidad
        if(xQueueReceive(speed_queue, &rpm_actual_int,0) == pdPASS){
            //casteamos el valor de los rpm
            rpm_actual = (float)rpm_actual_int;

            //calculamos el error usando el setpoin y nuestra velocidad actual
            error_actual = set_point - rpm_actual;

            //Calculamos la proporcional, que simplemente multiplica el error con nuestra constante proporcional
            PID_vals.proporcional = PID_Ks.Kc * error_actual;

            //Calculamos nuestra integral, con una condicion especial para evitar que dividamos entre 0 y se rompa el sistema
            if(PID_Ks.tao_i != 0.0){
                PID_vals.integral = PID_vals.integral + ( (PID_Ks.Kc / PID_Ks.tao_i) * error_actual * dt );
            } else {
                PID_vals.integral = 0;
            }

            //nos aseguramos que el valor de la integral este dentro de nuestros valores de PWM posibles
            if(PID_vals.integral > PID_MAX) PID_vals.integral = PID_MAX;
            if(PID_vals.integral < -PID_MAX) PID_vals.integral = -PID_MAX;


            //calculamos la derivada que usa dos estados del error para predecir cambios en el error
            PID_vals.derivativa = PID_Ks.Kc * PID_Ks.tao_d * ( (error_actual - error_anterior) / dt );
            
            // Actualizamos error anterior para el calculo
            error_anterior = error_actual;


            //Sumamos nuestras tres partes para obtener nuestra manipulacion
            PID_Output = PID_vals.proporcional + PID_vals.integral + PID_vals.derivativa;

            //Nos aseguramos que este valor no sea mayor que nuestros limites de PWM 
            if(PID_Output > PID_MAX) PID_Output = PID_MAX;
            if(PID_Output < PID_MIN) PID_Output = PID_MIN;

            
            //Mandamos  velocidad, error, y manipulacion al GUI
            Datos_GUI.PV = rpm_actual;
            Datos_GUI.OP = PID_Output;
            Datos_GUI.Error = error_actual;
            Datos_GUI.SP = set_point;
            xQueueOverwrite(datos_gui_queue, &Datos_GUI);

            //Mandamos manipulacion al handler del motor
            xQueueOverwrite(PID_queue, &PID_Output);

            //Imprimimos todos nuestros valores clave para aseguranos que funcione correctamente (para debuggeo)
            //Serial.printf("SP: %.1f | PV: %.1f | Err: %.1f | PID: %.1f\n", set_point, rpm_actual, error_actual, PID_Output);
        }
        t_fin = esp_timer_get_time();
        int32_t latency = (int32_t)(t_fin - t_inicio);

        report_info(TASK_PID,jitter,latency,periodo_real);
    }
}