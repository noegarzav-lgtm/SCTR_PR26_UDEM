#ifndef GUI_H
#define GUI_H
#include "Common.hpp"

//Declaramos struct que recibe los datos que manda el handle del PID
typedef struct{
    float PV;
    float OP;
    float Error;
    float SP;
} Datos_recibidos;

typedef struct{
    float kc;
    float tao_i;
    float tao_d;
}PID_Ks_GUI;

//Declaramso funciones a usar en el handle
uint8_t init_GUI();

void gui_task(void * pvParameters);

#endif