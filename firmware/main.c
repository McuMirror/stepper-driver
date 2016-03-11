#include "hw.h"
#include "motor.h"
#include "commands.h"
#include "program.h"
#include <stddef.h>

#define V 200

int main() {
    motor_init(&motor[0]);
    motor_init(&motor[1]);
    motor_init(&motor[2]);
    hw_init();

    static volatile int i;
    //for(i = 0; i < 20000000; i++);

    static data_move_t mymove = {
        .a = 0,
        .b = 0,
        .c = V,
        .end_t = 1,
        .end_p = V,
        .end_v = V
    };

    //program_start(0);
    //program_instruction(0, 2, &mymove, sizeof(mymove)); // move_rel
    //program_instruction(0, 0, NULL, 0); // halt
    //program_end(0);
    //program_load(0);
    //program_immediate(0, 2, &mymove, sizeof(mymove));
    //for(i = 0; i < 2000000; i++);
    //program_immediate(0, 2, &mymove, sizeof(mymove));

    //static cmd_data_t * const myprogdata[4] = {&stream_current, &mymove, &stream_nothing, NULL};
    //static cmd_data_t * const myprogdata2[2] = {&mymove2, NULL};

    //motor_load_program(&motor[0], myprog, myprogdata);
    //motor_load_program(&motor[1], myprog2, myprogdata2);
    //motor_load_program(&motor[2], myprog2, myprogdata2);

    hw_led_on();

    for(;;);
}
