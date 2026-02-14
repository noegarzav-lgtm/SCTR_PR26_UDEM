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


// Variables de PID digital
extern byte DUTY_CYCLE;

//Parametros de la ESP32
constexpr int BAUD_RATE = 115200;

//Parametros del PID
constexpr int PID_MAX = 255;
constexpr int PID_MIN = 1;


//Prioridad
constexpr uint8_t High_Priority = 3;
constexpr uint8_t Medium_Priority = 2;
constexpr uint8_t Low_Priority = 1;

// CORE Assignation
#define CORE_0      0
#define CORE_1      1



#endif