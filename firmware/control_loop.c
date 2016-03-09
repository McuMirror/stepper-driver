#include "control_loop.h"
#include "sine.h"
#include "hw.h"

uint16_t a;

void control_loop() {
    a += 4;
    a &= 1023;
    hw_pwm(cosine(a, 600), sine(a, 600), 0, 0, 0, 0);
}
