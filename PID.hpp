#ifndef PID_H
#define PID_H

#include "Common.hpp"

//Declaramos un struct que usaremos para guardar los valores de sintonizacion del PID
typedef struct{
    float Kc;
    float tao_i;
    float tao_d;
}PID_Ks;

//Declaramos un struct donde guardemos los valores de cada parte del PID
typedef struct{
    float proporcional;
    float integral;
    float derivativa;
}PID_vals;

//Declaramos un ultimo struct donde guardemos la info que mandaremos al GUI
typedef struct{
    float PV;
    float OP;
    float Error;
}Datos_GUI;

//Declaramos funciones a utilizar en el hanlder 
uint8_t init_PID();
void get_PID(void * pvParameters);

#endif
