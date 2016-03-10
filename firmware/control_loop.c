#include "control_loop.h"
#include "sine.h"
#include "hw.h"
#include "motor.h"

void control_loop() {
    motor[0].current_a = (float)adc_buffer[0] * CURRENT_SCALE;
    motor[0].current_b = (float)adc_buffer[1] * CURRENT_SCALE;
    motor[1].current_a = (float)adc_buffer[2] * CURRENT_SCALE;
    motor[1].current_b = (float)adc_buffer[3] * CURRENT_SCALE;
    motor[2].current_a = (float)adc_buffer[4] * CURRENT_SCALE;
    motor[2].current_b = (float)adc_buffer[5] * CURRENT_SCALE;

    motor_step(&motor[0]);
    motor_step(&motor[1]);
    motor_step(&motor[2]);

    hw_pwm(cosine(motor[0].p + motor[0].zero, motor[0].amp), sine(motor[0].p + motor[0].zero, motor[0].amp),
           cosine(motor[1].p + motor[1].zero, motor[1].amp), sine(motor[1].p + motor[1].zero, motor[1].amp),
           cosine(motor[2].p + motor[2].zero, motor[2].amp), sine(motor[2].p + motor[2].zero, motor[2].amp));
}
