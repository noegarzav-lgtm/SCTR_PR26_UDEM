#include "PID_Handler.h"

static pid_controller controller;
extern QueueHandle_t speed_queue;

uint8_t initControllerPID(float kc, float tao_i, float tao_d)
{
    controller.kc = kc;
    controller.tao_i = tao_i;
    controller.tao_d = tao_d;

    controller.setPoint = 0;
    controller.error = 0;

    controller.proportional = 0;
    controller.integral = 0;
    controller.derivative = 0;

    controller.distanceBuffer[0] = 0;
    controller.distanceBuffer[1] = 0;
    controller.distanceBuffer[2] = 0;

    controller.pid = 0;

    controller.dt = 0.01;          
    controller.pid_min = 0.0;
    controller.pid_max = 255.0;    

    return 1;
}


void setSetPoint(float setPoint)
{
    controller.setPoint = setPoint;
}


void calculatePID(float sensorValue)
{
    
    float distance = sensorValue;     
  
    controller.error = controller.setPoint - distance;

    
    controller.proportional = controller.error * controller.kc;

    controller.distanceBuffer[2] = controller.distanceBuffer[1];
    controller.distanceBuffer[1] = controller.distanceBuffer[0];
    controller.distanceBuffer[0] = distance;

    controller.derivative =
        (controller.distanceBuffer[0] - controller.distanceBuffer[2])
        * controller.kc * controller.tao_d
        / (2.0f * controller.dt);

   
    controller.integral += controller.error * controller.kc * controller.dt / controller.tao_i;

    if (controller.integral > controller.pid_max)
        controller.integral = controller.pid_max;

    else if (controller.integral < -controller.pid_max)
        controller.integral = -controller.pid_max;

   
    controller.pid = controller.proportional + controller.derivative + controller.integral;

    if (controller.pid > controller.pid_max)
        controller.pid = controller.pid_max;

    else if (controller.pid < controller.pid_min)
        controller.pid = controller.pid_min;
}
