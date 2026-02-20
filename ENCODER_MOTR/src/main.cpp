#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

// Definición de pines 
const int PWM = 4;
const int potenciometro = 34;
const int AIN1 = 26;
const int AIN2 = 27;
const int STBY = 33;

int sensorValue = 0;
byte duty_cycle = 0;
float manipulation = 0;

const int pinA = 18;  //Fase A Encoder
const int pinB = 19;  //Fase B Encoder

// Variables volátiles porque se modifican dentro de una interrupción
volatile long contador = 0;

// Esta función se ejecuta CADA VEZ que el Pin A cambia de estado
void IRAM_ATTR leerEncoder() {
  // Leemos el estado del pin B para saber la dirección
  int estadoB = digitalRead(pinB);
  
  // Si A sube y B está en LOW, giramos en un sentido
  // Si A sube y B está en HIGH, giramos en el otro
  if (digitalRead(pinA) == HIGH) { 
    if (estadoB == LOW) {
      contador++;
    } else {
      contador--;
    }
  } else { 
    // Lógica para cuando A baja 
    if (estadoB == HIGH) {
      contador++;
    } else {
      contador--;
    }
  }
}

void setup() {
  Serial.begin(9600);

  // Activamos las resistencias internas
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  // Configuramos potenciometro como entrada
  pinMode(potenciometro, INPUT);

  // Controlar motor
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWM, OUTPUT);
  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);

  // Configuramos la interrupción en el Pin A
  // CHANGE significa que se activa al subir (0->1) y al bajar (1->0)
  attachInterrupt(digitalPinToInterrupt(pinA), leerEncoder, CHANGE);
}

void loop() {
  // Motor
  sensorValue = analogRead(potenciometro); 
  manipulation = map(sensorValue, 0, 4095, 0, 255);
  duty_cycle = manipulation;
  analogWrite(PWM, duty_cycle);
  Serial.println(sensorValue);


  // Imprimimos todo en una sola línea limpia
  Serial.printf("PWM: %d | Posicion: %ld\n", duty_cycle, contador);

  // Imprimimos el valor solo para verificar
  // static long ultimoValor = 0;
  // if (contador != ultimoValor) {
  //   Serial.printf("Posición: %ld\n", contador);
  //   ultimoValor = contador;
  // }
  delay(200); // Pequeña pausa para no saturar el monitor serie
}