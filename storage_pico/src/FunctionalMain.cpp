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


void goTo(int target)
    {
    currentPos = decoder.decipherExpected(move);
    shortcut.setCurrentPosition(currentPos);
    shortcut.setTarget(target);
    move = shortcut.getClosestPosition();

    while (move != 0)
    {
        int currentPos = decoder.decipherExpected(move);
        
        if(move > 0) {
            motor.forwards();
            //currentSensor.update();
            

        }
        else if(move < 0) {
            motor.backwards();
            //currentSensor.update();
        }
        sleep_ms(100);
        tight_loop_contents();
        
    }
    motor.stop();
    printf("SUCCES!!: already at target\n");
    
        

}

int main() 
{

    sleep_ms(2000); // Wait for USB serial to initialize

    //variable:
    int target = 1; //making it global to use it more than just in one function
    int currentPos = 4;

    //Setups, Inits and Objects
    stdio_init_all();

    UartClass uartConnection(1, 8, 9, 115200); //First number is target position, second number is current position and last number is max number og positions
    uartConnection.setup();

    Interface mInterface(uartConnection);

    CurrentSensor currentSensor;
    currentSensor.setup();

    DcMotor motor(2, 3, 11);

    pathFinder shortcut(0, 3, 8); //in sh

    ADS7830 ads1(i2c0, 0, 1, 100000, 0x48, 10);
    ads1.init(); //IMPORTANT without nothing is initialised 

    ldrSensors ldr1(ads1, 1);
    ldrSensors ldr2(ads1, 2);
    ldrSensors ldr3(ads1, 4);
    ldrSensors ldr4(ads1, 5);

    drejeBaenkDecoder decoder(ldr1, ldr2, ldr3, ldr4);

    
    decoder.timeBasedCalibration(motor, 2000); //Calibration
    
    float move = shortcut.getClosestPosition();

    while (true) {

        if (mInterface.hasCommand()) 
            {
            CmdType cmd = mInterface.getCommand()
            switch (cmd) 
                {
                case CMDType::PING:
                mInterface.sendResponse(cmdType::PING, ResponseType::OK);
                
                ///ping tilbage

                case CMDType::GOTO:
                DesiredPositon = mInterface.getPosition();
                mInterface.sendResponse(cmdType::GOTO, ResponseType::OK);
                goTo(DesiredPositon); ///Run goto command

                case CMDType::STOP:
                
                motor.stop(); ////RUN STOP Command
                }   
            }

        int currentPos = decoder.decipherExpected(move);

        if (currentPos != -1 && currentPos != oldPosition)
        {
            std::cout << "current position is: " << currentPos << std::endl;

            oldPosition = currentPos;
        }

        if(currentPos != -1) {
            shortcut.setCurrentPosition(currentPos);

        }

        
    }   
    return 0;
}
