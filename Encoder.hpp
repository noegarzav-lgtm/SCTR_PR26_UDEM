#ifndef ENCODER_H
#define ENCODER_H

#include "Common.hpp"
//Definimos las funciones a usar en el handler
uint8_t init_encoder();
//Al usar interrupciones para contar los pulsos, se ocupa una funcion especifica que se active con las interrupciones
void IRAM_ATTR leerEncoder();
void encoder_function(void * parameter);

#endif
// #endif COMMON_H