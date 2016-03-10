#ifndef __MOTOR_H
#define __MOTOR_H

#include <stdint.h>
#include "commands.h"

typedef struct motor {
    float p;
    float v;
    float amp;
    float zero;
    cmd_functions_t * const * program;
    cmd_data_t * const * program_data;
    int16_t pc;
} motor_t;

extern motor_t motor[3];

void motor_init(motor_t* motor);
void motor_step(motor_t* motor);
void motor_load(motor_t* m);
void motor_load_program(motor_t* m, cmd_functions_t * const * program, cmd_data_t * const * program_data);
 
#endif
