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

#include "uartClass.h"
#include "Interface.h"
#include "datalogger.h"

#include "../../gripper_pico/src/CurrentSensor.h"

int main() {


    CurrentSensor currentSensor;
    currentSensor.setup();

    clock_t ProgramStart = clock();

    int dataloggingSpan = 300;

    srand(time(0)); //for the random target generator to stress test
    
    stdio_init_all();
    sleep_ms(2000); // Wait for USB serial to initialize

    const uint LED_PIN = 25;
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 1);

    int target = 1; //making it global to use it more than just in one function

    UartClass uartConnection(1, 8, 9, 115200);
    uartConnection.setup();

    Interface mInterface(uartConnection);

    dataLogger log1;

    DcMotor motor(2, 3, 11);
    pathFinder shortcut(0, 3, 8); //in shortcut first number is target position, second number is current position and last number is max number og positions

    ADS7830 ads1(i2c0, 0, 1, 100000, 0x48, 10);
    ads1.init(); //IMPORTANT without nothing is initialised 
    
    
    ldrSensors ldr1(ads1, 1);
    ldrSensors ldr2(ads1, 2);
    ldrSensors ldr3(ads1, 4);
    ldrSensors ldr4(ads1, 5);
    

    drejeBaenkDecoder decoder(ldr1, ldr2, ldr3, ldr4);

    decoder.timeBasedCalibration(motor, 2000);

    log1.reset();

    /*
    ldr1.calibrateValue();
    ldr2.calibrateValue();
    ldr3.calibrateValue();
    ldr4.calibrateValue();
    */

    target = (rand() % 8) +1; // for the random target generator, 8 different targets
    
    int position;
    /*
    if (mInterface.hasCommand()) 
    {
        CmdType cmd = mInterface.getCommand()
        switch (cmd) 
        {
            case CMDType::PING:
            mInterface.sendResponse(cmdType::PING, ResponseType::OK)
            
            ///ping tilbage
            case CMDType::GOTO:
            positon = mInterface.getPosition()
            mInterface.sendResponse(cmdType::GOTO, ResponseType::OK)
            ///Run goto command
            case CMDType::STOP:
            ////
            
    }   
    */
    shortcut.setTarget(target); //setting target via setter in pathFinder for random target generator
    printf("SUCESS!!: New target: %d\n", target);

    log1.appendTarget(target);
    log1.timeTaskStart();

    int oldPosition = target; //Should probably be set to the actual old target

    //float move = 0; not this
    float move = shortcut.getClosestPosition();

    log1.timeTaskStart();

    int limitingCount = 0;

    while (true) {

        //currentSensor.update();
        

        //int currentPos = decoder.decipherRaw();
        int currentPos = decoder.decipherExpected(move);

        //std::array<int, 4> bits = decoder.readBits();
        
        /*
        printf("BITS: %d %d %d %d | POS: %d\n",
            bits[0], bits[1], bits[2], bits[3], currentPos);
        */

        if (currentPos != -1 && currentPos != oldPosition)
        {
            std::cout << "current position is: " << currentPos << std::endl;

            log1.appendReading(currentPos);

            oldPosition = currentPos;
        }

        if(currentPos != -1) {
            shortcut.setCurrentPosition(currentPos);

        }

        

        move = shortcut.getClosestPosition(); //finding closest path to target

        if(move == 0) {
            motor.stop();

            log1.timeTaskEnd();

            log1.storeSuccess();

            printf("SUCCES!!: already at target\n");

            std::cout << "timePassed: " <<((clock() - ProgramStart) / CLOCKS_PER_SEC) << std::endl;
            if (((clock() - ProgramStart) / CLOCKS_PER_SEC) > dataloggingSpan)
            {
                log1.printData();
                sleep_ms(1000);

                //Could be nice if it moved to 4...

                break;
            }

            sleep_ms(500);

            target = (rand() % 8) +1; // for the random target generator, 8 different targets
            shortcut.setTarget(target); //setting target via setter in pathFinder for random target generator
            printf("New target: %d\n", target);
            
            //std::cout << "target is: " << target << std::endl;
            log1.appendTarget(target);
            log1.timeTaskStart();
            
        }
        else if(move > 0) {
            motor.forwards();
            //currentSensor.update();
            

        }
        else if(move < 0) {
            motor.backwards();
            //currentSensor.update();
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

        

/*
        int raw1 = ldr1.readRaw();
        int raw2 = ldr2.readRaw();
        int raw3 = ldr3.readRaw();
        int raw4 = ldr4.readRaw();

        int bit1 = ldr1.interpretValue();
        int bit2 = ldr2.interpretValue();
        int bit3 = ldr3.interpretValue();
        int bit4 = ldr4.interpretValue();

        uint8_t channel1 = ads1.readChannel(0);
        uint8_t channel2 = ads1.readChannel(1);
        uint8_t channel3 = ads1.readChannel(2);
        uint8_t channel4 = ads1.readChannel(3);
        uint8_t channel5 = ads1.readChannel(4);
        uint8_t channel6 = ads1.readChannel(5);
        uint8_t channel7 = ads1.readChannel(6);
        uint8_t channel8 = ads1.readChannel(7);
*/
        //int decoded = decoder.decipher();
/*
       printf(
            "RAW: %3d %3d %3d %3d | BITS: %d %d %d %d | DEC: %d\n",
            raw1, raw2, raw3, raw4,
            bit1, bit2, bit3, bit4,
            decoded
        );
*/
        //std::cout << "target is: " << target << std::endl;
        

//        printf("Channels: %d %d %d %d %d %d %d %d\n", channel1, channel2, channel3, channel4, channel5, channel6, channel7, channel8);
        //currentSensor.update();
        sleep_ms(100);
        tight_loop_contents();
    }   
    return 0;
}
