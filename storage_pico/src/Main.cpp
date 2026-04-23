#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"


int main() {
    stdio_init_all();
    DcMotor motor(0, 1, 11);
    motor.forwards();
    pathFinder shortcut(0, 0, 8);
    int target;
    //in shortcut first number is target position, second number is current position and last number
    //is max number og positions



    while (true) {
        std::cout << "what is the target: \n";
        std::cin >> target;
        shortcut.setTarget(target);
		std::cout << "Need to move: " << shortcut.getClosestPosition() << std::endl;


        tight_loop_contents();
    }
}
