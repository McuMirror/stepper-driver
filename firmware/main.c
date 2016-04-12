#include "hw.h"
#include "motor.h"
#include "commands.h"
#include "program.h"
#include <stddef.h>

int main() {
    hw_init();
    motor_init();
    hw_led_on();

    for(;;);
}
