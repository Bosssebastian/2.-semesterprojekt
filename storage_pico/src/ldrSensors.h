#ifndef LDRSENSORS_H
//ldrSensors.h

#define LDRSENSORS_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cstdint> 
#include <stdint.h>
#include <array>
#include <cmath>
#include <vector>
#include "encoder.h"

class ldrSensors
{
    public:
        ldrSensors(
            ADS7830& ads1,
            uint8_t channel,
            int samples = 5
        );

        uint8_t readRaw();
        float readAverage();

        void calibrateValue();

        float readCalibrated();

        float TOP_PERCENT = 0.96;

        int interpretValue();

    private:
        ADS7830& _ads1;
        uint8_t _channel;
        int _samples;
        uint8_t _calibratedValue;
        

};

#endif