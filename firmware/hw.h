#ifndef __HW_H
#define __HW_H

#include <stdint.h>

#define SYSTEM_CLOCK 72000000
#define PWM_PERIOD 2250
#define PWM_DT ((float)(PWM_PERIOD) / (SYSTEM_CLOCK))

void hw_init();
void hw_led_on();
void hw_led_off();
void hw_start_adc_conversion();
void hw_pwm(int16_t m1a, int16_t m1b, int16_t m2a, int16_t m2b, int16_t m3a, int16_t m3b);
void hw_start_control_loop();
void hw_stop_control_loop();
 
#endif
