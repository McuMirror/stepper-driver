#include "motor.h"
#include "hw.h"
#include "commands.h"
#include "buffers.h"
#include <stddef.h>

motor_t motor[3];

cmd_functions_t * const empty_program[1] = {&cmd_halt};
cmd_data_t * const empty_program_data[1] = {NULL};

void motor_init(motor_t* m) {
    m->state = MOTOR_RUN;
    m->p = 0;
    m->v = 0;
    m->zero = 0;
    m->amp = 0;
    m->pc = 0;
    m->program = empty_program;
    m->program_data = empty_program_data;
    motor_load(m);
}

static void handle_streaming(motor_t* m) {
    switch(m->stream) {
        case STREAM_NONE:
            break;
        case STREAM_CURRENT:
            if(buf_num_free(&stream_buffer) < 2 * sizeof(float)) {
                m->error = "Stream overrun";
                motor_error(m);
                break;
            }
            buf_write(&stream_buffer, &m->current_a, 2 * sizeof(float));
            EP3_Check_Ready();
            break;
    }
}

void motor_step(motor_t* m) {
    switch(m->state) {
        case MOTOR_RUN:
            m->program[m->pc]->step(m, m->program_data[m->pc]);
            if(m->v == 0) {
                m->amp = 0.1;
            } else {
                m->amp = 0.4;
            }
            handle_streaming(m);
            break;
        case MOTOR_ERROR:
            m->amp = 0;
            break;
    }
}

void motor_done(motor_t* m) {
    status_buffer_put(m->ix, m->pc, 0);
}

void motor_error(motor_t* m) {
    stream_flush = 1;
    status_buffer_put(m->ix, m->pc, 1);
    m->state = MOTOR_ERROR;
}

void motor_load(motor_t* m) {
    m->program[m->pc]->load(m, m->program_data[m->pc]);
}

void motor_load_program(motor_t* m, cmd_functions_t * const * program, cmd_data_t * const * program_data) {
    hw_stop_control_loop();
    m->program = program;
    m->program_data = program_data;
    m->pc = 0;
    m->state = MOTOR_RUN;
    motor_load(m);
    hw_start_control_loop();
}
