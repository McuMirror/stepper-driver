#include "hw.h"

int main() {
    hw_init();
    hw_led_on();
    hw_pwm(-1000, 0, 0, 0, 0, 0);
    for(;;);
}
