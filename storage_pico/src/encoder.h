#ifndef ENCODER_H
//encoder.h

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
            uint32_t baudrate = 100000,
            uint8_t i2cAddress = 0x48,
            uint8_t samples = 10
            

            );


            int CALIBRATION_MS = 3000;
            int SAMPLE_DELAY_MS = 5;
            int SAMPLES_PER_READ = 5;
            

        void init();
        uint8_t readChannel(uint8_t channel);

        float readAverage(uint8_t channel);

    private:
        uint8_t channelCode(uint8_t ch);

        

        i2c_inst_t* _i2cPort;
        uint _sdaPin;
        uint _sclPin;
        uint32_t _baudrate;
        uint8_t _i2cAddress;
        uint8_t _samples;
        

};

#endif