#ifndef GUI_H
#define GUI_H
#include "Common.hpp"

typedef struct{
    float PV;
    float OP;
    float Error;
} Datos_recibidos;

uint8_t init_GUI();

void gui_task(void * pvParameters);

#endif