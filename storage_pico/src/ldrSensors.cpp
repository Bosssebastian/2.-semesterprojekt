#include <ldrSensors.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <cstdint> 
#include <stdint.h>
#include <array>
#include <cmath>
#include <vector>

ldrSensors::ldrSensors(ADS7830& ads1, uint8_t channel, int samples)
        : _ads1(ads1), _channel(channel), _samples(samples)
        {

        }

float ldrSensors::readAverage()
{
    float total = 0;

    for (int i = 0; i < _samples; i++)
    {
        total += ldrSensors::readRaw();
        sleep_ms(2);
    }

    return total / _samples;
}

uint8_t ldrSensors::readRaw()
{
    return _ads1.readChannel(_channel); 
}

void ldrSensors::calibrateValue()
{
    _calibratedValue = readAverage();
}

float ldrSensors::readCalibrated()
{

    return (_calibratedValue - readAverage() );
}

int ldrSensors::interpretValue()
{   
    if (abs(readAverage()) < _calibratedValue * TOP_PERCENT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}