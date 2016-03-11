#ifndef __COMMANDS_H
#define __COMMANDS_H

#include <stdint.h>

#define N_COMMANDS 6

struct motor;

typedef void cmd_data_t;

typedef const struct cmd_functions {
    const uint8_t * data_size;
    void (*load)(volatile struct motor*, cmd_data_t*);
    void (*step)(volatile struct motor*, cmd_data_t*);
} cmd_functions_t;

#define DECLARE_CMD(NAME) cmd_functions_t cmd_ ## NAME = { \
    .load = NAME ## _load, \
    .step = NAME ## _step, \
    .data_size = & NAME ## _data_size \
}

// Commands

// Absolute move

typedef struct data_move {
    float end_t;
    float a;
    float b;
    float c;
    float d;
    float end_p;
    float end_v;
    float t;
} data_move_t;

extern cmd_functions_t cmd_halt;
extern cmd_functions_t cmd_move_abs;
extern cmd_functions_t cmd_move_rel;
extern cmd_functions_t cmd_zero_abs;
extern cmd_functions_t cmd_zero_rel;
extern cmd_functions_t cmd_stream;

extern cmd_functions_t * const command_list[N_COMMANDS]; 

#endif
