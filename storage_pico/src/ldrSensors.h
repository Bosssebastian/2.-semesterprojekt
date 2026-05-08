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

        float initialDrop = 0.5; 
        // initialDrop is how low the values 
        //should get relative to average read value over full marker before it thinks it is over blank area

        float upperThreshold = 0.2; //Top 20% of values

        uint8_t readRaw();
        float readAverage();

        void calibrateValue();

        float readCalibrated();

        

        float TOP_PERCENT = 0.96;

        int interpretValue();

        void saveValue();

        void totalCalibration();

        float vectorAverage(const std::vector<float> inputVector);

    private:
        ADS7830& _ads1;
        uint8_t _channel;
        int _samples;

        uint8_t _calibratedValueLow = 0;
        uint8_t _calibratedValueHigh = 0;
        float _threshold = 0;

        std::vector<float> _savedValues;
        
        

};

#endif