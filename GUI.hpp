#ifndef GUI_H
#define GUI_H
#include "Common.hpp"

//Declaramos struct que recibe los datos que manda el handle del PID
typedef struct{
    float PV;
    float OP;
    float Error;
} Datos_recibidos;

//Declaramso funciones a usar en el handle
uint8_t init_GUI();

void gui_task(void * pvParameters);

#endif