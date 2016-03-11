#include "commands.h"
#include "motor.h"
#include "circular_buffer.h"
#include "buffers.h"
#include "hw.h"
#include <stddef.h>

// Halt

static void halt_load(motor_t* m, cmd_data_t* data) {
    m->v = 0;
}

static void halt_step(motor_t* m, cmd_data_t* data) {
}

const uint8_t halt_data_size = 0;

DECLARE_CMD(halt);

// Absolute move

const uint8_t move_abs_data_size = sizeof(data_move_t);

static void move_abs_load(motor_t* m, cmd_data_t* data) {
    data_move_t* d = (data_move_t*)data;

    d->t = 0;
}

static void move_abs_step(motor_t* m, cmd_data_t* data) {
    data_move_t* d = (data_move_t*)data;

    m->p = d->d + d->t * (d->c + d->t * (d->b + d->t * d->a));
    m->v = d->c + d->t * (2 * d->b + d->t * 3 * d->a);
    d->t += PWM_DT;
    if(d->t > d->end_t) {
        m->p = d->end_p;
        m->v = d->end_v;
        motor_done(m);
        m->pc++;
        motor_load(m);
    }
}

DECLARE_CMD(move_abs);

// Relative move

const uint8_t move_rel_data_size = sizeof(data_move_t);

static void move_rel_load(motor_t* m, cmd_data_t* data) {
    data_move_t* d = (data_move_t*)data;

    d->t = 0;
    d->d = m->p;
}

static void move_rel_step(motor_t* m, cmd_data_t* data) {
    data_move_t* d = (data_move_t*)data;

    m->p = d->d + d->t * (d->c + d->t * (d->b + d->t * d->a));
    m->v = d->c + d->t * (2 * d->b + d->t * 3 * d->a);
    d->t += PWM_DT;
    if(d->t > d->end_t) {
        m->p = d->d + d->end_p;
        m->v = d->end_v;
        motor_done(m);
        m->pc++;
        motor_load(m);
    }
}

DECLARE_CMD(move_rel);

// Zero abs

const uint8_t zero_abs_data_size = sizeof(float);

static void zero_abs_load(motor_t* m, cmd_data_t* data) {
    float* z = (float*)data;

    m->zero += m->p - *z;
    m->p = *z;
    m->zero -= (int32_t)m->zero;
}

static void zero_abs_step(motor_t* m, cmd_data_t* data) {
    motor_done(m);
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(zero_abs);

// Zero rel

const uint8_t zero_rel_data_size = sizeof(float);

static void zero_rel_load(motor_t* m, cmd_data_t* data) {
    float* z = (float*)data;

    m->zero += *z;
    m->p -= *z;
    m->zero -= (int32_t)m->zero;
}

static void zero_rel_step(motor_t* m, cmd_data_t* data) {
    motor_done(m);
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(zero_rel);

// Stream

const uint8_t stream_data_size = sizeof(stream_t);

static void stream_load(motor_t* m, cmd_data_t* data) {
    stream_t* stream = (stream_t*)data;

    m->stream = *stream;
    stream_buffer_flush();
}

static void stream_step(motor_t* m, cmd_data_t* data) {
    motor_done(m);
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(stream);

// Command list

cmd_functions_t * const command_list[N_COMMANDS] = { 
    &cmd_halt,
    &cmd_move_abs,
    &cmd_move_rel,
    &cmd_zero_abs,
    &cmd_zero_rel,
    &cmd_stream
};
