#ifndef PID_H
#define PID_H

#include "Common.hpp"

typedef struct{
    float Kc;
    float tao_i;
    float tao_d;
}PID_Ks;

typedef struct{
    float proporcional;
    float integral;
    float derivativa;
}PID_vals;

typedef struct{
    float PV;
    float OP;
    float Error;
}Datos_GUI;

uint8_t init_PID();
void get_OP(void * pvParameters);

#endif
