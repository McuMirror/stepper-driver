#ifndef __HW_H
#define __HW_H

#include <stdint.h>

#define PWM_PERIOD 2250
#define CL_PERIOD 2250

void hw_init();
void hw_led_on();
void hw_led_off();
void hw_pwm(int16_t m1a, int16_t m1b, int16_t m2a, int16_t m2b, int16_t m3a, int16_t m3b);

#endif
