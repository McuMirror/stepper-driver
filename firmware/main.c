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

    static volatile int i;
    for(i = 0; i < 2000000; i++);

    static cmd_functions_t * const myprog[4] = {&cmd_stream, &cmd_move_rel, &cmd_stream, &cmd_halt};

    static stream_t stream_current = STREAM_CURRENT;
    static stream_t stream_nothing = STREAM_NONE;

    static data_move_t mymove = {
        .a = 0,
        .b = 0,
        .c = V,
        .end_t = 1,
        .end_p = V,
        .end_v = V
    };
    static cmd_data_t * const myprogdata[4] = {&stream_current, &mymove, &stream_nothing, NULL};

    motor_load_program(&motor[0], myprog, myprogdata);
    //motor_load_program(&motor[1], myprog, myprogdata);
    //motor_load_program(&motor[2], myprog, myprogdata);

    hw_led_on();

    for(;;);
}
