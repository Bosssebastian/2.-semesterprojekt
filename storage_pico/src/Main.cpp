#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"
#include "pathFinder.h"
#include <iostream>
#include "encoder.h"
#include <stdio.h>
#include "pico/stdio.h"


int main() {
    
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to initialize

    const uint LED_PIN = 25;
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    /*
    DcMotor motor(0, 1, 11);
    motor.forwards();
    sleep_ms(5000);
    motor.forwards();
    sleep_ms(5000);
    pathFinder shortcut(0, 3, 8);
    int target;
    //in shortcut first number is target position, second number is current position and last number
    //is max number og positions
    */

    ADS7830 ads1(i2c0, 0, 1, 100000, 0x48);
    ads1.init();


    while (true) {
        std::cout << "what is the target: \n";
        std::cin >> target;
        shortcut.setTarget(target);
		std::cout << "Need to move: " << shortcut.getClosestPosition() << std::endl;

        uint8_t channel1 = ads1.readChannel(0);
        uint8_t channel2 = ads1.readChannel(1);
        uint8_t channel3 = ads1.readChannel(2);
        uint8_t channel4 = ads1.readChannel(3);
        uint8_t channel5 = ads1.readChannel(4);
        uint8_t channel6 = ads1.readChannel(5);
        uint8_t channel7 = ads1.readChannel(6);
        uint8_t channel8 = ads1.readChannel(7);

        tight_loop_contents();
        printf("Channels: %d %d %d %d %d %d %d %d\n", channel1, channel2, channel3, channel4, channel5, channel6, channel7, channel8);

        sleep_ms(200);

        //tight_loop_contents();
    }
    return 0;
}
