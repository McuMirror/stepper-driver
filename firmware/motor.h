#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>
#include "commands.h"

typedef enum stream {
    STREAM_NONE,
    STREAM_CURRENT
} stream_t;

typedef struct motor {
    enum {
        MOTOR_RUN,
        MOTOR_ERROR
    } state;
    float p;
    float v;
    float amp;
    float zero;
    float current_a;
    float current_b;
    cmd_functions_t * const * program;
    cmd_data_t * const * program_data;
    uint8_t pc;
    uint8_t ix;
    stream_t stream;
    const char * error;
} motor_t;

extern motor_t motor[3];

void motor_init(motor_t* motor);
void motor_step(motor_t* motor);

void motor_load_program(motor_t* m, cmd_functions_t * const * program, cmd_data_t * const * program_data);

void motor_load(motor_t* m);
void motor_done(motor_t* m);
void motor_error(motor_t* m);
  
#endif
