//main

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
#include "ldrSensors.h"

#include "drejeBaenkDecoder.h"


int main() {
    srand(time(0)); //for the random target generator to stress test
    
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to initialize

    const uint LED_PIN = 25;
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    int target = 1; //making it global to use it more than just in one function


    DcMotor motor(4, 5, 6);
    pathFinder shortcut(0, 3, 8); //in shortcut first number is target position, second number is current position and last number is max number og positions

    ADS7830 ads1(i2c0, 0, 1, 100000, 0x48, 10);
    ads1.init(); //IMPORTANT without nothing is initialised 
    
    
    ldrSensors ldr1(ads1, 1);
    ldrSensors ldr2(ads1, 2);
    ldrSensors ldr3(ads1, 4);
    ldrSensors ldr4(ads1, 5);
    

    drejeBaenkDecoder decoder(ldr1, ldr2, ldr3, ldr4);

    ldr1.calibrateValue();
    ldr2.calibrateValue();
    ldr3.calibrateValue();
    ldr4.calibrateValue();

    target = (rand() % 8) +1; // for the random target generator, 8 different targets
    shortcut.setTarget(target); //setting target via setter in pathFinder for random target generator
    printf("SUCESS!!: New target: %d\n", target);

    while (true) {

        int currentPos = decoder.decipher();

        if(currentPos != -1) {
            shortcut.setCurrentPosition(currentPos);

        }

        float move = shortcut.getClosestPosition(); //finding closest path to target

        if(move == 0){
            motor.stop();
            printf("SUCCES!!: already at target\n");

            sleep_ms(500);

            target = (rand() % 8) +1; // for the random target generator, 8 different targets
            shortcut.setTarget(target); //setting target via setter in pathFinder for random target generator
            printf("New target: %d\n", target);
        }
        else if(move > 0) {
            motor.forwards();


        }
        else if(move < 0) {
            motor.backwards();
            
        }


        //std::cout << "what is the target: \n";
        //std::cin >> target;
        //shortcut.setTarget(target);
		//std::cout << "Need to move: " << shortcut.getClosestPosition() << std::endl;

        /*
        uint8_t channel1 = ads1.readChannel(0);
        uint8_t channel2 = ads1.readChannel(1);
        uint8_t channel3 = ads1.readChannel(2);
        uint8_t channel4 = ads1.readChannel(3);
        uint8_t channel5 = ads1.readChannel(4);
        uint8_t channel6 = ads1.readChannel(5);
        uint8_t channel7 = ads1.readChannel(6);
        uint8_t channel8 = ads1.readChannel(7);

        tight_loop_contents();
        */
       /*
       int channel1 = ldr1.readAverage();
       int channel2 = ldr2.readAverage();
       int channel3 = ldr3.readAverage();
       int channel4 = ldr4.readAverage();
       int channel5 = ldr5.readAverage();
        */
       /*
       int channel1 = ldr1.readRaw();
       int channel2 = ldr2.readRaw();
       int channel3 = ldr3.readRaw();
       int channel4 = ldr4.readRaw();


        printf("Channels: %d %d %d %d %d \n", channel1, channel2, channel3, channel4);
        */

        


        int raw1 = ldr1.readAverage();
        int raw2 = ldr2.readAverage();
        int raw3 = ldr3.readAverage();
        int raw4 = ldr4.readAverage();

        int bit1 = ldr1.interpretValue();
        int bit2 = ldr2.interpretValue();
        int bit3 = ldr3.interpretValue();
        int bit4 = ldr4.interpretValue();

        int decoded = decoder.decipher();

     /*   printf(
            "RAW: %3d %3d %3d %3d | BITS: %d %d %d %d | DEC: %d\n",
            raw1, raw2, raw3, raw4,
            bit1, bit2, bit3, bit4,
            decoded
        );*/
        //std::cout << "target is: " << target << std::endl;
        if(currentPos != -1) {
            std::cout << "current position is: " << currentPos << std::endl;

        }
        sleep_ms(100);
        tight_loop_contents();
    }   
    return 0;
}
