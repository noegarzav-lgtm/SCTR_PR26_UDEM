#include "Common.hpp"

static Task_ID info_encoder = {0};
static Task_ID info_speed = {0};
static Task_ID info_pid = {0};
static Task_ID info_gui = {0};
static Task_ID info_pwm = {0};

portMUX_TYPE metricasMux = portMUX_INITIALIZER_UNLOCKED;

void report_info(System_ID task, int32_t jitter, int32_t latencia, int32_t periodo){
    int32_t abs_jitter = abs(jitter);
    int32_t abs_latencia = abs(latencia);
    int32_t abs_periodo = abs(periodo); 

    portENTER_CRITICAL(&metricasMux);

    switch(task){
        case TASK_ENCODER:
            if(abs_jitter > info_encoder.jitter_maximo) {
                info_encoder.jitter_maximo = abs_jitter;
            }
            if(abs_latencia > info_encoder.latencia_maxima) {
                info_encoder.latencia_maxima = abs_latencia;
            }
            info_encoder.periodo_real = abs_periodo;
            break;
        case TASK_SPEED:
            if(abs_jitter > info_speed.jitter_maximo) {
                info_speed.jitter_maximo = abs_jitter;
            }
            if(abs_latencia > info_speed.latencia_maxima) {
                info_speed.latencia_maxima = abs_latencia;
            }
            info_speed.periodo_real = abs_periodo;
            break;
        case TASK_PID:
            if(abs_jitter > info_pid.jitter_maximo) {
                info_pid.jitter_maximo = abs_jitter;
            }
            if(abs_latencia > info_pid.latencia_maxima) {
                info_pid.latencia_maxima = abs_latencia;
            }
            info_pid.periodo_real = abs_periodo;
            break;
        case TASK_GUI:
            if(abs_jitter > info_gui.jitter_maximo) {
                info_gui.jitter_maximo = abs_jitter;
            }
            if(abs_latencia > info_gui.latencia_maxima) {
                info_gui.latencia_maxima = abs_latencia;
            }
            info_gui.periodo_real = abs_periodo;
            break;
        case TASK_PWM:
            if(abs_jitter > info_pwm.jitter_maximo) {
                info_pwm.jitter_maximo = abs_jitter;
            }
            if(abs_latencia > info_pwm.latencia_maxima) {
                info_pwm.latencia_maxima = abs_latencia;
            }
            info_pwm.periodo_real = periodo;
            break;
    }
    portEXIT_CRITICAL(&metricasMux);
}

Task_info get_metricas(){
    Task_info metricas;
    portENTER_CRITICAL(&metricasMux);

    metricas.encoder = info_encoder;
    metricas.speed = info_speed;
    metricas.pid = info_pid;
    metricas.gui= info_gui;
    metricas.pwm= info_pwm;

    info_encoder = {0,0};
    info_speed = {0,0};
    info_pid = {0,0};
    info_gui = {0,0};    
    info_pwm = {0,0};    

    portEXIT_CRITICAL(&metricasMux);
    return metricas;
}