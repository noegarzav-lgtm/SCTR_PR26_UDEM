#include <Arduino.h>
#include "Encoder.hpp"
#include "SPEED.hpp"
#include "Common.hpp"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD_RATE);
  uint8_t encoder_init = init_encoder();
  uint8_t speed_init = init_speed();

  if(encoder_init !=1 && speed_init != 1){
    esp_restart();
  }
  else{
    Serial.printf("Setup Complete");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  vTaskDelete(NULL);
}

