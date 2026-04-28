#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"
#include "pathFinder.h"
#include <iostream>
#include "encoder.h"
#include <stdio.h>
#include "pico/stdio.h"
#include <cstdlib> //for random target
#include <ctime> //for random target

int main() {
    srand(time(NULL)); //for the random target generator to stress test
    
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to initialize

    const uint LED_PIN = 25;
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);


    DcMotor motor(4, 5, 6);
    pathFinder shortcut(0, 3, 8); //in shortcut first number is target position, second number is current position and last number is max number og positions
    ADS7830 encoder(i2c0, 0, 1, 100000, 0x48);
    encoder.init();


    while (true) {

        float move = shortcut.getClosestPosition(); //finding closest path to target

        if(move == 0){
            motor.stop();
            printf("already at target\n");

            sleep_ms(500);

        int target = (rand() % 8) +1; // for the random target generator, 8 different targets
        shortcut.setTarget(target); //setting target via setter in pathFinder for random target generator
        printf("New target: %d\n", target);
        }
        else if(move > 0) {
            motor.forwards();
        }
        else {
            motor.backwards();
        }

        int currentPos = encoder.getPosition();
        shortcut.setCurrentPosition(currentPos);
        //std::cout << "what is the target: \n";
        //std::cin >> target;
        //shortcut.setTarget(target);
		//std::cout << "Need to move: " << shortcut.getClosestPosition() << std::endl;

        uint8_t channel1 = encoder.readChannel(0);
        uint8_t channel2 = encoder.readChannel(1);
        uint8_t channel3 = encoder.readChannel(2);
        uint8_t channel4 = encoder.readChannel(3);
        uint8_t channel5 = encoder.readChannel(4);
        uint8_t channel6 = encoder.readChannel(5);
        uint8_t channel7 = encoder.readChannel(6);
        uint8_t channel8 = encoder.readChannel(7);

        printf("Channels: %d %d %d %d %d %d %d %d\n", channel1, channel2, channel3, channel4, channel5, channel6, channel7, channel8);


        sleep_ms(100);
        tight_loop_contents();
    }
    return 0;
}
