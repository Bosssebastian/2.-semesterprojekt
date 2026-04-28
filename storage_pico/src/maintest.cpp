#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"
#include "pathFinder.h"
#include <iostream>


int main() {
    stdio_init_all();
    DcMotor motor(0, 1, 11);
    motor.forwards();
    sleep_ms(3000);
    motor.stop();
    sleep_ms(2000);
    motor.backwards();
    sleep_ms(3000);
    motor.stop();


    while (true) {
        tight_loop_contents();
    }
}
