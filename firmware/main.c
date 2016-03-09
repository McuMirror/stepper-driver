#include "hw.h"

int main() {
    hw_init();
    hw_led_on();
    for(;;);
}
