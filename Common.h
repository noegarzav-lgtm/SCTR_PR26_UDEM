#ifndef COMMON_H
#define COMMON_H

#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

//Librería de FreeRTOS
#include "freertos/FreeRTOS.h"

//Control PWM, salida del controlador digital
#define PWM 4

//Pines de driver de motores - TB6612FNG 
#define AIN1 26
#define AIN2 27
#define STBY 33

//Pines de encoder
#define PIN_A 18
#define PIN_B 19

//Contador de pulsos del encoder
extern volatile long CONTADOR = 0;

// Variables de PID digital
extern byte DUTY_CYCLE = 0;
extern float MANIPULACION = 0;
extern float ERROR = 0;
extern float PV = 0;
extern float SET_POINT = 0;

//Parametros de la ESP32
extern int BAUD_RATE = 9600;

//Parametros del PID
extern int PID_MAX = 255;
extern int PID_MIN = 1;
extern float distanceBuffer[3] = {0,0,0};
extern float PROPORCIONAL = 0;
extern float INTEGRAL = 0;
extern float DERIVATIVA = 0;
extern float PID = 0;


#endif