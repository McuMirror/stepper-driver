#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>
#include "commands.h"

enum {
    CMD_START,
    CMD_FINISHED,
    CMD_ERROR
}; // status printouts

enum {
    MOTOR_STOP,
    MOTOR_RUN,
    MOTOR_ERROR
}; // motor.state

enum {
    STREAM_NONE,
    STREAM_CURRENT
}; // motor.stream

enum {
    CONTROL_MODE_CURRENT_OL,
    CONTROL_MODE_VOLTAGE
}; // motor.control_mode

typedef volatile struct motor {
    uint8_t ix;
    uint8_t state;
    float p;
    float v;
    float amp;
    float zero;
    float current_a;
    float current_b;
    uint8_t pc;
    uint8_t stream;
    uint8_t control_mode;
    float setp;
    float km;
    float r;
    const char * error;
    cmd_functions_t * const * program;
    cmd_data_t * const * program_data;
} motor_t;

extern motor_t motor[3];

void motor_step(motor_t* motor);

void motor_load_program(motor_t* m, cmd_functions_t * const * program, cmd_data_t * const * program_data);

void motor_load(motor_t* m);
void motor_halt(motor_t* m);
void motor_error(motor_t* m);
void motor_stop(motor_t* m);
  
#endif
