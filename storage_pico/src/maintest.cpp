#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"
#include "pathFinder.h"
#include <iostream>


int main() {
    stdio_init_all();
    sleep_ms(100);
    DcMotor motor(0, 1, 11);
    




    while (true) {
    motor.backwards();
    sleep_ms(3000);
    motor.forwards();
    sleep_ms(3000);
        tight_loop_contents();
    
    }
}
