#include <Arduino.h>
#include "driver/gpio.h"

// --- 1. CONFIGURATION ---
// Value obtained from first delivery (speed control with potentiometer)
int PPR = 205;  

// --- Pin Definitions ---
const int PWM_PIN = 4;        
const int POT_PIN = 34;
const int AIN1 = 26;
const int AIN2 = 27;
const int STBY = 33;
const int ENC_A = 18; 
const int ENC_B = 19;

// --- Variables ---
// 'volatile' is required because this variable is modified inside an ISR
volatile long encoderCount = 0; 

// Timing and Calculation Variables
unsigned long lastTimeRPM = 0;
long lastPosition = 0;

// --- INTERRUPT SERVICE ROUTINE (ISR) ---
// This function runs every time the encoder sends a pulse
void IRAM_ATTR readEncoder() {
  int stateB = digitalRead(ENC_B);
  
  // Determine direction based on the state of Channel B when Channel A changes
  if (digitalRead(ENC_A) == HIGH) { 
    if (stateB == LOW)
    {
      encoderCount++;
    } 
    else
    {
      encoderCount--;
    } 
  } 
  else
  { 
    if (stateB == HIGH)
    {
      encoderCount++;
    } 
    else
    {
      encoderCount--; 
    } 
  }
}

void setup() {
  // 1. Initialize Serial Communication
  Serial.begin(9600); 

  // 2. Encoder Setup
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  // Attach the interrupt to Pin A (Trigger on CHANGE: Rising and Falling edges)
  attachInterrupt(digitalPinToInterrupt(ENC_A), readEncoder, CHANGE);

  // 3. Input/Output Setup
  pinMode(POT_PIN, INPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(STBY, OUTPUT);

  // 4. Initial Motor State (Forward direction, Standby OFF)
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
}

void loop() {
  // --- 1. Motor Control Section ---
  int sensorValue = analogRead(POT_PIN); 
  
  // Map 12-bit ADC value (0-4095) to 8-bit ESP32 PWM value (0-255)
  int dutyCycle = map(sensorValue, 0, 4095, 0, 255);
  
  // Control Motor SPEED
  analogWrite(PWM_PIN, dutyCycle);

  // --- 2. RPM Calculation Section (Runs every 100ms) ---
  unsigned long currentTime = millis();
  
  if (currentTime - lastTimeRPM >= 100) {
    
    // --- CRITICAL SECTION START ---
    // We disable interrupts momentarily to safely read the multi-byte variable 'encoderCount'
    noInterrupts();
    long currentPosition = encoderCount;
    interrupts();
    // --- CRITICAL SECTION END ---

    // A. Calculate the change in position (Delta Pulses)
    long pulsesDiff = currentPosition - lastPosition;
    
    // B. Calculate the exact time elapsed since the last calculation
    unsigned long timeDiff = currentTime - lastTimeRPM;

    // C. The Math: RPM = (Delta Pulses / PPR) * (60000ms / Delta Time)
    // We use 60000.0 to force floating-point arithmetic
    double currentRPM = ((double)pulsesDiff / PPR) * (60000.0 / timeDiff);

    // D. Update variables for the next loop
    lastPosition = currentPosition;
    lastTimeRPM = currentTime;

    // --- 3. Serial Output ---
    // Print the PWM input and the calculated RPM
    Serial.printf("PWM: %d | RPM: %.2f\n", dutyCycle, currentRPM);
  }
  delay(100);
}
