#include "control_loop.h"
#include "sine.h"
#include "hw.h"

uint16_t a;
uint16_t b;
uint16_t c;

void control_loop() {
    a++;
    a = a & 1023;
    b = cosine(a, PWM_PERIOD);
    c = sine(a, PWM_PERIOD);
    hw_pwm(cosine(a, PWM_PERIOD), sine(a, PWM_PERIOD), 0, 0, 0, 0);
}
