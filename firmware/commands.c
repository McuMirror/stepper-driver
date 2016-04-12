#include "commands.h"
#include "motor.h"
#include "circular_buffer.h"
#include "buffers.h"
#include "hw.h"
#include <stddef.h>

// Halt

static const char halt_data_descriptor[] = "";

static void halt_load(motor_t* m, cmd_data_t* data) {
    m->v = 0;
}

static void halt_step(motor_t* m, cmd_data_t* data) {
    motor_halt(m);
}

const uint8_t halt_data_size = 0;

DECLARE_CMD(halt);

// Absolute move

static const char move_abs_data_descriptor[] = "\
    arg float end_t; \
    arg float end_p; \
    arg float end_v; \
    arg float a; \
    arg float b; \
    arg float c; \
    arg float d; \
    internal float t; \
";

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
        m->pc++;
        motor_load(m);
    }
}

DECLARE_CMD(move_abs);

// Relative move

static const char move_rel_data_descriptor[] = "\
    arg float end_t; \
    arg float end_p; \
    arg float end_v; \
    arg float a; \
    arg float b; \
    arg float c; \
    internal float d; \
    internal float t; \
";

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
        m->pc++;
        motor_load(m);
    }
}

DECLARE_CMD(move_rel);

// Zero abs

const uint8_t zero_abs_data_size = sizeof(float);

static const char zero_abs_data_descriptor[] = "\
    arg float position; \
";

static void zero_abs_load(motor_t* m, cmd_data_t* data) {
    float* z = (float*)data;

    m->zero += m->p - *z;
    m->p = *z;
    m->zero -= (int32_t)m->zero;
}

static void zero_abs_step(motor_t* m, cmd_data_t* data) {
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(zero_abs);

// Zero rel

const uint8_t zero_rel_data_size = sizeof(float);

static const char zero_rel_data_descriptor[] = "\
    arg float increment; \
";

static void zero_rel_load(motor_t* m, cmd_data_t* data) {
    float* z = (float*)data;

    m->zero += *z;
    m->p -= *z;
    m->zero -= (int32_t)m->zero;
}

static void zero_rel_step(motor_t* m, cmd_data_t* data) {
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(zero_rel);

// Stream

const uint8_t stream_data_size = sizeof(uint8_t);

static const char stream_data_descriptor[] = "\
    arg enum8 {STREAM_NONE, STREAM_CURRENT, STREAM_VOLTAGE} stream; \
";

static void stream_load(motor_t* m, cmd_data_t* data) {
    uint8_t* stream = (uint8_t*)data;

    m->stream = *stream;
    stream_buffer_flush();
}

static void stream_step(motor_t* m, cmd_data_t* data) {
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(stream);

// Control mode

typedef struct data_control_mode {
    uint8_t mode;
    float setp;
} data_control_mode_t;

const uint8_t control_mode_data_size = sizeof(data_control_mode_t);

static const char control_mode_data_descriptor[] = "\
    arg enum8 {CONTROL_MODE_CURRENT_OL, CONTROL_MODE_VOLTAGE} mode; \
    arg float setp; \
";

static void control_mode_load(motor_t* m, cmd_data_t* data) {
    data_control_mode_t* control_mode = (data_control_mode_t*)data;

    m->control_mode = control_mode->mode;
    m->setp = control_mode->setp;
}

static void control_mode_step(motor_t* m, cmd_data_t* data) {
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(control_mode);

// Tuning

typedef struct data_tuning {
    float km;
    float r;
} data_tuning_t;

const uint8_t tuning_data_size = sizeof(data_tuning_t);

static const char tuning_data_descriptor[] = "\
    arg float km; \
    arg float r; \
";

static void tuning_load(motor_t* m, cmd_data_t* data) {
    data_tuning_t* tuning = (data_tuning_t*)data;

    m->km = tuning->km;
    m->r = tuning->r;
}

static void tuning_step(motor_t* m, cmd_data_t* data) {
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(tuning);

// Command list

cmd_functions_t * const command_list[N_COMMANDS] = { 
    &cmd_halt,
    &cmd_move_abs,
    &cmd_move_rel,
    &cmd_zero_abs,
    &cmd_zero_rel,
    &cmd_stream,
    &cmd_control_mode,
    &cmd_tuning,
};
