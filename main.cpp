#include <Arduino.h>
#include "Encoder.hpp"
#include "SPEED.hpp"
#include "PID.hpp"
#include "Common.hpp"
#include "Motor_PWM.hpp"
#include "GUI.hpp"

void setup() {
  //inicializamos nuestros task
  Serial.begin(BAUD_RATE);
  uint8_t encoder_init = init_encoder();
  uint8_t speed_init = init_speed();
  uint8_t pid_init = init_PID();
  uint8_t PWM_init = init_PWM();
  uint8_t GUI_init = init_GUI();
  

  if(encoder_init !=1 && speed_init != 1 && pid_init != 1 && PWM_init != 1 && GUI_init !=1){
    esp_restart();
  }
  else{
    Serial.printf("Setup Complete");
  }
}

void loop() {
  vTaskDelete(NULL);
}

