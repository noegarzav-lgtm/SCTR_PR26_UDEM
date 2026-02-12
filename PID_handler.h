#ifndef PID_HANDLER
#define PID_HANDLER

#include "Common.h"

typedef struct{
    float kc;
    float tao_i;
    float tao_d;

    float setPoint;
    float error;

    float proportional;
    float integral;
    float derivative;

    float distanceBuffer[3];

    float pid;

    float dt;
    float pid_min;
    float pid_max;

}pid_controller;

void initControllerPID(float kc, float tao_i, float tao_d);
void calculatePID(float sensorValue);
void setSetPoint(float setPoint);
// void taskPid
// void sendPWM

#endif