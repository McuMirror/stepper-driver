#include "program.h"
#include "motor.h"
#include <stddef.h>

program_error_t program_start(uint8_t motor_n) {
    if(motor_n >= 3) return PROGRAM_INVALID_MOTOR;
    uint8_t buffer_n = motor_n * 2 + 1 - program_pp[motor_n];

    program_ptr[buffer_n] = 0;
    program_mem_ptr[buffer_n] = 0;
    program_ready[motor_n] = 0;
    return PROGRAM_SUCCESS;
}

program_error_t program_instruction(uint8_t motor_n, uint8_t command_n, void* data, uint8_t length) {
    if(motor_n >= 3) return PROGRAM_INVALID_MOTOR;
    if(command_n >= N_COMMANDS) return PROGRAM_INVALID_COMMAND;

    uint8_t buffer_n = motor_n * 2 + 1 - program_pp[motor_n];
    void* data_mem = &((uint8_t *)program_mem)[buffer_n * PROGRAM_MEMORY_SIZE];
    cmd_functions_t** cmds_functions = &program_functions[buffer_n * MAX_PROGRAM_LENGTH];
    cmd_data_t** cmds_data = &program_data[buffer_n * MAX_PROGRAM_LENGTH];
    uint16_t mem_ptr = program_mem_ptr[buffer_n];
    uint8_t ptr = program_ptr[buffer_n];

    if(program_ready[motor_n]) return PROGRAM_NOT_STARTED;
    if(ptr == MAX_PROGRAM_LENGTH) return PROGRAM_TOO_LONG;
    cmd_functions_t* f = command_list[command_n];
    if(*f->data_size < length) return PROGRAM_BAD_DATA;
    if(mem_ptr + *f->data_size >= PROGRAM_MEMORY_SIZE) return PROGRAM_NOT_ENOUGH_MEMORY;

    uint16_t size = *f->data_size;
    if((size & 3) != 0) size = (size & (uint16_t)(~3)) + 4;
    uint16_t i;
    for(i = 0; i < size; i++) {
        ((uint8_t *)data_mem)[mem_ptr + i] = (i < length) ? ((uint8_t*)data)[i] : 0;
    }
    program_mem_ptr[buffer_n] += i;

    cmds_functions[ptr] = f;
    cmds_data[ptr] = &((uint8_t *)data_mem)[mem_ptr];
    program_ptr[buffer_n]++;
    return PROGRAM_SUCCESS;
}

program_error_t program_end(uint8_t motor_n) {
    if(motor_n >= 3) return PROGRAM_INVALID_MOTOR;

    if(program_ready[motor_n]) return PROGRAM_NOT_STARTED;
    program_ready[motor_n] = 1;
    return PROGRAM_SUCCESS;
}

program_error_t program_load(uint8_t motor_n) {
    if(motor_n >= 3) return PROGRAM_INVALID_MOTOR;
    if(!program_ready[motor_n]) return PROGRAM_NOT_FINISHED;

    program_pp[motor_n] = 1 - program_pp[motor_n];
    uint8_t buffer_n = motor_n * 2 + program_pp[motor_n];
    cmd_functions_t** cmds_functions = &program_functions[buffer_n * MAX_PROGRAM_LENGTH];
    cmd_data_t** cmds_data = &program_data[buffer_n * MAX_PROGRAM_LENGTH];
    motor_load_program(&motor[motor_n], cmds_functions, cmds_data);

    buffer_n = motor_n * 2 + 1 - program_pp[motor_n];
    program_ready[motor_n] = 0;
    program_ptr[buffer_n] = 0;
    program_mem_ptr[buffer_n] = 0;
    program_ready[motor_n] = 0;
    return PROGRAM_SUCCESS;
}

program_error_t program_immediate(uint8_t motor_n, uint8_t command_n, void* data, uint8_t length) {
    program_error_t r;
    if((r = program_start(motor_n)) != PROGRAM_SUCCESS) return r;
    if((r = program_instruction(motor_n, command_n, data, length)) != PROGRAM_SUCCESS) return r;
    if((r = program_instruction(motor_n, 0, NULL, 0)) != PROGRAM_SUCCESS) return r;
    if((r = program_end(motor_n)) != PROGRAM_SUCCESS) return r;
    if((r = program_load(motor_n)) != PROGRAM_SUCCESS) return r;
    return PROGRAM_SUCCESS;
}


