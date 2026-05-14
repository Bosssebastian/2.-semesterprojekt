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
    _calibratedValueLow = readAverage();
}

float ldrSensors::readCalibrated()
{

    return (_calibratedValueLow - readAverage() );
}

int ldrSensors::interpretValue()
{   
    float currentValue = readAverage();

    return interpretRawValue(currentValue);
    /*
    float currentValue = readAverage();
    float threshold = _calibratedValueHigh * initialDrop;
    if (abs(readAverage()) < _calibratedValueLow * TOP_PERCENT)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    */
}

int ldrSensors::interpretRawValue(float value)
{
    if (value > _threshold)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void ldrSensors::saveValue()
{
    //1. Turning untill all LDR's drop value
    //2. Reading values for each LDR

    _savedValues.push_back(readAverage());

    //Saving top and bottom x% of values and average those for calibratedValueLow and calibratedValueHigh
    //This can be done by having an array of a certain length for high and low, and then appending if read value is in top/bottom 20%
    //Or that we save all read values and then filter afterwards
    //So throwing away the middle 60% (100-2x)% and then making a vector for high and low or diretly taking the average of the top/bottom 20%
    //3. Write these threasholds for each LDR to a file

}

float ldrSensors::vectorAverage(const std::vector<float>& inputVector)
{

    if (inputVector.empty())
    {   
        return 0;
    }

    float average = 0;

    for (int i = 0; i < inputVector.size(); i++)
    {
        average += inputVector[i];
    }
    average = average / inputVector.size();

    return average;
}

//Should this method be in another class? Surely this is run on a specifik LDR object, so we do not want it to be run more 
//times than neccesary, but then there could be difference, then it need to continue to spin untill all sensors see background
void ldrSensors::totalCalibration() 
{

    if (_savedValues.empty())
    {
        return;
    }

    float maxValue = _savedValues[0];
    float minValue = _savedValues[0];

    for (int i = 1; i < _savedValues.size(); i++)
    {
        if (_savedValues[i] > maxValue)
        {
            maxValue = _savedValues[i];
        }

        if (_savedValues[i] < minValue)
        {
            minValue = _savedValues[i];
        }

    }

    float highLimit = maxValue - ((maxValue - minValue) * upperThreshold);
    float lowLimit = minValue + ((maxValue - minValue) * upperThreshold);

    std::vector<float> highValues;
    std::vector<float> lowValues;

    for (int i=0; i < _savedValues.size(); i++)
    {
        if (_savedValues[i] > highLimit)
        {
            highValues.push_back(_savedValues[i]);
        }
        if (_savedValues[i] < lowLimit)
        {
            lowValues.push_back(_savedValues[i]);
        }
    }

    _calibratedValueHigh = vectorAverage(highValues);
    _calibratedValueLow = vectorAverage(lowValues);

    _threshold = ( _calibratedValueHigh + _calibratedValueLow ) / 2;

    /* OLD CODE
    int currentValue = readAverage(); //readAverage is just measuring 10 ssamples and taking the average, this is perceved as 1 reading

    //While loop need at least one element in savedValues
    saveValue(); //Savevalue just saves the current reading for each LDR


    //Hopefully, it will read values untill the readings drop below threshold of initial drop
    //motor.forwards();
    while (currentValue > (vectorAverage(_savedValues) * initialDrop)) 
    {
        saveValue();
        sleep_ms(2);
    }
    //motor.stop();

    
    for (int i = 0; i < _savedValues.size(); i++) //Storing max value in calibratedValueHigh
    {
        if (_savedValues[i] > _calibratedValueHigh)
        {
            _calibratedValueHigh = _savedValues[i]; //using calibratedValueHigh as placeholder
        }
    }

    _calibratedValueHigh = _calibratedValueHigh * (1 - upperThreshold);

    for (int i = 0; i < _savedValues.size(); i++) //Deleting all elements under top 20%
    {
        if (_savedValues[i] < _calibratedValueHigh)
        {
            _savedValues.erase(_savedValues.begin() + i);
        }
    }

    //Finally setting the correct _calibratedValueHigh to the average of the remaining vector
    _calibratedValueHigh = vectorAverage(_savedValues); 
    
    */

}
