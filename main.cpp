#include <Arduino.h>
#include "Encoder.hpp"
#include "Common.hpp"

extern int BAUD_RATE = 9600;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUD_RATE);
  uint8_t encoder_init = init_encoder();

  if(encoder_init !=1){
    esp_restart();
  }
  Serial.printf("Setup Complete");
}

void loop() {
  // put your main code here, to run repeatedly:
  vTaskDelete(NULL);
}

