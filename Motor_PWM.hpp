#ifndef Motor_PWM_H
#define Motor_PWM_H
#include "Common.hpp"

//Delcarar funciones que usaremos en el handler
uint8_t init_PWM();
void send_PWM(void * pvParameters);


#endif