#ifndef ENCODER_H
#define ENCODER_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cstdint> 
#include <array>
#include <cmath>
#include <vector>

class ADS7830
{
    public:
        ADS7830(
            i2c_inst_t* i2cPort = i2c0,
            uint sdaPin = 0,
            uint sclPin = 1,
            uint32_t baurate = 100000,
            uint8_t i2cAddress = 0x48

            );

            int CALIBRATION_MS = 3000;
            int SAMPLE_DELAY_MS = 5;
            int SAMPLES_PER_READ = 5;
            float TOP_PERCENT = 0.50;
            
            //Ciffertabel kunne cære graycode eller bit-mønster
            std::array<std::array<int, 4>, 8> CIFFERTABEL = {{
                {1, 1, 0, 0},  // -> 1
                {1, 0, 0, 1},  // -> 2
                {1, 0, 0, 0},  // -> 3
                {1, 1, 1, 1},  // -> 4
                {1, 0, 1, 0},  // -> 5
                {1, 1, 1, 0},  // -> 6
                {1, 1, 0, 1},  // -> 7
                {1, 0, 1, 1},  // -> 8
        }};

        void init();
        uint8_t readChannel(uint8_t channel);

        int getPosition(); 

    private:
        uint8_t channelCode(uint8_t ch);

        i2c_inst_t* _i2cPort;
        uint _sdaPin;
        uint _sclPin;
        uint32_t _baurate;
        uint8_t _i2cAddress;

};

#endif