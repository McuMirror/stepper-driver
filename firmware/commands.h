#ifndef __COMMANDS_H
#define __COMMANDS_H

#include <stdint.h>

#define N_COMMANDS 8

struct motor;

typedef void cmd_data_t;

typedef const struct cmd_functions {
    const char * name;
    const uint8_t * data_size;
    const char * data_descriptor;
    void (*load)(volatile struct motor*, cmd_data_t*);
    void (*step)(volatile struct motor*, cmd_data_t*);
} cmd_functions_t;

#define DECLARE_CMD(NAME) cmd_functions_t cmd_ ## NAME = { \
    .name = #NAME, \
    .data_size = & NAME ## _data_size, \
    .data_descriptor = NAME ## _data_descriptor, \
    .load = NAME ## _load, \
    .step = NAME ## _step, \
}

// Commands

// Absolute move

typedef struct data_move {
    float end_t;
    float end_p;
    float end_v;
    float a;
    float b;
    float c;
    float d;
    float t;
} data_move_t;

extern cmd_functions_t * const command_list[N_COMMANDS]; 

#endif
