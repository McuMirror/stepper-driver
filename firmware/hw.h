#ifndef __HW_H
#define __HW_H

#include <stdint.h>

#define SYSTEM_CLOCK 72000000
#define PWM_PERIOD 2250
#define PWM_DT ((float)(PWM_PERIOD) / (SYSTEM_CLOCK))

#define CURRENT_SCALE ((float)(3.3 / (4096 * 4.7 * 0.1)))

void hw_init();
void hw_led_on();
void hw_led_off();
void hw_start_adc_conversion();
 
extern int16_t adc_buffer[6];
extern uint8_t run_control_loop;

#endif
