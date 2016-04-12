#include "control_loop.h"
#include "sine.h"
#include "hw.h"
#include "motor.h"

void control_loop() {
    motor_step(&motor[0]);
    motor_step(&motor[1]);
    motor_step(&motor[2]);
}
