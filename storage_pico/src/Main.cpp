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

/* DEBUGGING WITH INPUT-STREAM
std::string readUsbLine()
{
    static std::string buffer;

    int ch = getchar_timeout_us(0);

    while (ch != PICO_ERROR_TIMEOUT)
    {
        char c = static_cast<char>(ch);

        if (c == '\n' || c == '\r')
        {
            std::string line = buffer;
            buffer.clear();
            return line;
        }

        buffer += c;

        // TEMP TEST FIX:
        // Accept "goto X" immediately, even without newline.
        if (buffer.length() == 6 && buffer.rfind("goto ", 0) == 0)
        {
            std::string line = buffer;
            buffer.clear();
            return line;
        }

        if (buffer == "stop")
        {
            std::string line = buffer;
            buffer.clear();
            return line;
        }

        ch = getchar_timeout_us(0);
    }

    return "";
}
*/


int main() 
{

    stdio_init_all();

    sleep_ms(2000); // Wait for USB serial to initialize

    //variable:
    int target = 1; //making it global to use it more than just in one function
    int currentPos = 4;
    int desiredPosition;
    int oldDirection;

    //Setups, Inits and Objects
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
    
    

    currentPos = decoder.decipherExpected(0);

    shortcut.setCurrentPosition(currentPos);

    if (currentPos == -1)
    {
        printf("ERROR: No valid initial encoder position\n");
    }
    else
    {
        shortcut.setCurrentPosition(currentPos);
        printf("DEBUG initial currentPos=%d\n", currentPos);
    }

    float move = shortcut.getClosestPosition();

    auto goTo = [&]()
    {
        //Offset by +2 from sensor to delivery-zone
        desiredPosition = ((desiredPosition + 2 - 1 + 8) % 8) + 1;
        
        shortcut.setTarget(desiredPosition);
        move = shortcut.getClosestPosition();
        /*
        currentPos = decoder.decipherExpected(move);
        shortcut.setCurrentPosition(currentPos);
        shortcut.setTarget(desiredPosition);
        move = shortcut.getClosestPosition();
        */
        while (move != 0)
        {
            
            currentPos = decoder.decipherExpected(move);
            
            shortcut.setCurrentPosition(currentPos);

            move = shortcut.getClosestPosition();

            //printf("DEBUG GOTO target=%d current=%d move=%.2f\n", desiredPosition, currentPos, move);

            if(move > 0) {
                oldDirection = 1;
                motor.forwards();
                //currentSensor.update();
                

            }
            else if(move < 0) {
                oldDirection = -1;
                motor.backwards();
                //currentSensor.update();
            }
            else 
            {
                motor.stop();
                break;
            }
            sleep_ms(100);
            tight_loop_contents();
            
        }
        if (oldDirection == 1)
        {
            motor.forwards();
            sleep_ms(200);
        }
        else {
            motor.backwards();
            sleep_ms(300);
        }
        motor.stop();
        printf("TARGET REACHED!");
    };

    printf("USB input test started\n");

    while (true) {

        
        if (mInterface.hasCommand()) 
            {
            CmdType cmd = mInterface.getCommand();
            switch (cmd) 
                {
                case CmdType::PING:
                mInterface.sendResponse(CmdType::PING, ResponseType::OK, "");
                break;
                ///ping tilbage

                case CmdType::GOTO:
                desiredPosition = mInterface.getPosition();
                mInterface.sendResponse(CmdType::GOTO, ResponseType::OK, "");
                goTo(); ///Run goto command
                break;

                case CmdType::STOP:
                motor.stop(); ////RUN STOP Command
                break;
                }   
            }
        
            
        /*
        std::string input = readUsbLine();

        if (!input.empty())
        {
            //printf("You wrote: %s\n", input.c_str());

            if (input.rfind("goto ", 0) == 0)
            {
                desiredPosition = std::atoi(input.substr(5).c_str());

                if (desiredPosition >= 1 && desiredPosition <= 8)
                {
                    goTo();
                }
                else
                {
                    //printf("Invalid target. Use goto 1 to goto 8\n");
                }
            }
            else if (input == "stop")
            {
                motor.stop();
            }
        }
        */
    }   
    return 0;
}

