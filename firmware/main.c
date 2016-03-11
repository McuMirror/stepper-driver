#include "hw.h"
#include "motor.h"
#include "commands.h"
#include "program.h"
#include <stddef.h>

int main() {
    motor_init(&motor[0]);
    motor_init(&motor[1]);
    motor_init(&motor[2]);
    hw_init();
    hw_led_on();

    for(;;);
}
