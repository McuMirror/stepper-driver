#ifndef __PROGRAM_H
#define __PROGRAM_H

#include "commands.h"
#include <stdint.h>

#define PROGRAM_MEMORY_SIZE 1024
#define MAX_PROGRAM_LENGTH 128

uint8_t program_mem[6 * PROGRAM_MEMORY_SIZE];
int16_t program_mem_ptr[6];

cmd_functions_t* program_functions[6 * MAX_PROGRAM_LENGTH];
cmd_data_t* program_data[6 * MAX_PROGRAM_LENGTH];
int16_t program_ptr[6];

uint8_t program_pp[3];
uint8_t program_ready[3];

typedef enum program_error {
    PROGRAM_SUCCESS,
    PROGRAM_NOT_STARTED,
    PROGRAM_NOT_FINISHED,
    PROGRAM_INVALID_MOTOR,
    PROGRAM_INVALID_COMMAND,
    PROGRAM_TOO_LONG,
    PROGRAM_BAD_DATA,
    PROGRAM_NOT_ENOUGH_MEMORY,
} program_error_t;

program_error_t program_start(uint8_t motor_n);
program_error_t program_instruction(uint8_t motor_n, uint8_t command_n, void* data, uint8_t length);
program_error_t program_end(uint8_t motor_n);
program_error_t program_load(uint8_t motor_n);
program_error_t program_immediate(uint8_t motor_n, uint8_t command_n, void* data, uint8_t length);

#endif
