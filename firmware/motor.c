#include "motor.h"
#include "hw.h"
#include "commands.h"
#include "buffers.h"
#include <stddef.h>

motor_t motor[3];

void motor_init() {
    motor[0].ix = 0;
    motor[1].ix = 1;
    motor[2].ix = 2;
}

static void handle_streaming(motor_t* m) {
    float currents[2];
    int16_t voltages[2];
    switch(m->stream) {
        case STREAM_NONE:
            break;
        case STREAM_CURRENT:
            currents[0] = m->current_a;
            currents[1] = m->current_b;
            if(!stream_buffer_put(currents, sizeof(currents))) {
                m->error = "Stream overrun";
                motor_error(m);
                break;
            }
            break;
        case STREAM_VOLTAGE:
            voltages[0] = m->voltage_a;
            voltages[1] = m->voltage_b;
            if(!stream_buffer_put(voltages, sizeof(voltages))) {
                m->error = "Stream overrun";
                motor_error(m);
                break;
            }
            break;
    }
}

static void set_amplitude(motor_t* m) {
    switch(m->control_mode) {
        case CONTROL_MODE_CURRENT_OL:
            m->amp = m->km * m->v + m->setp * m->r;
            break;
        case CONTROL_MODE_VOLTAGE:
            m->amp = m->setp;
            break;
        default:
            m->amp = 0;
            break;
    }
}

void motor_step(motor_t* m) {
    switch(m->state) {
        case MOTOR_RUN:
            m->program[m->pc]->step(m, m->program_data[m->pc]);
            set_amplitude(m);
            handle_streaming(m);
            break;
        case MOTOR_STOP:
            set_amplitude(m);
            break;
        case MOTOR_ERROR:
            m->amp = 0;
            break;
    }
}

void motor_halt(motor_t* m) {
    stream_buffer_flush();
    status_buffer_put(m->ix, m->pc, CMD_FINISHED);
    motor_stop(m);
}

void motor_error(motor_t* m) {
    stream_buffer_flush();
    status_buffer_put(m->ix, m->pc, CMD_ERROR);
    m->state = MOTOR_ERROR;
}

void motor_load(motor_t* m) {
    m->program[m->pc]->load(m, m->program_data[m->pc]);
}

void motor_load_program(motor_t* m, cmd_functions_t * const * program, cmd_data_t * const * program_data) {
    run_control_loop = 0;
    m->program = program;
    m->program_data = program_data;
    m->pc = 0;
    m->state = MOTOR_RUN;
    motor_load(m);
    run_control_loop = 1;
    status_buffer_put(m->ix, m->pc, CMD_START);
}

void motor_stop(motor_t* m) {
    m->state = MOTOR_STOP;
}
