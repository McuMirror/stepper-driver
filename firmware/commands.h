#ifndef __COMMANDS_H
#define __COMMANDS_H

struct motor;

typedef void cmd_data_t;

typedef const struct cmd_functions {
    void (*load)(struct motor*, cmd_data_t*);
    void (*step)(struct motor*, cmd_data_t*);
} cmd_functions_t;

#define DECLARE_CMD(NAME) cmd_functions_t cmd_ ## NAME = { \
    .load = NAME ## _load, \
    .step = NAME ## _step \
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

#endif
