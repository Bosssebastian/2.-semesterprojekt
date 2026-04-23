#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"


int main() {
    stdio_init_all();
    DcMotor motor(0, 1, 11);
    motor.forwards();


    while (true) {



        tight_loop_contents();
    }
}
