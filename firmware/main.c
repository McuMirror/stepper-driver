#include "hw.h"
#include "motor.h"
#include "commands.h"
#include <stddef.h>

#define V 200

int main() {
    motor_init(&motor[0]);
    motor_init(&motor[1]);
    motor_init(&motor[2]);
    hw_init();

    static cmd_functions_t * const myprog[3] = {&cmd_move_rel, &cmd_move_rel, &cmd_halt};
    static data_move_t mymove = {
        .a = 0,
        .b = V / 2,
        .c = 0,
        .end_t = 1,
        .end_p = V / 2,
        .end_v = V
    };
    static data_move_t mymove2 = {
        .a = 0,
        .b = 0,
        .c = V,
        .end_t = 1,
        .end_p = V,
        .end_v = V
    };
    static cmd_data_t * const myprogdata[3] = {&mymove, &mymove2, NULL};

    motor_load_program(&motor[0], myprog, myprogdata);

    hw_led_on();

    for(;;);
}
