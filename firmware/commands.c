#include "commands.h"
#include "hw.h"
#include "motor.h"
#include <stddef.h>

// Halt

static void halt_load(motor_t* m, cmd_data_t* data) {
    m->v = 0;
}

static void halt_step(motor_t* m, cmd_data_t* data) {
}

DECLARE_CMD(halt);

// Absolute move

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

static void zero_abs_load(motor_t* m, cmd_data_t* data) {
    float* z = (float*)data;

    if(z == NULL) {
        m->zero += m->p;
        m->p = 0;
    } else {
        m->zero += m->p - *z;
        m->p = *z;
    }
    m->zero -= (int32_t)m->zero;
}

static void zero_abs_step(motor_t* m, cmd_data_t* data) {
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(zero_abs);

// Zero rel

static void zero_rel_load(motor_t* m, cmd_data_t* data) {
    float* z = (float*)data;

    if(z != NULL) {
        m->zero += *z;
        m->p -= *z;
    }
    m->zero -= (int32_t)m->zero;
}

static void zero_rel_step(motor_t* m, cmd_data_t* data) {
    m->pc++;
    motor_load(m);
}

DECLARE_CMD(zero_rel);
