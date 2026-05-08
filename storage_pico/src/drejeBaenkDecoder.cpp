//DrejeBaenkDecoder.cpp
#include "drejeBaenkDecoder.h"
#include <array>
#include "pico/stdlib.h"


drejeBaenkDecoder::drejeBaenkDecoder(ldrSensors& ldr1, ldrSensors& ldr2, ldrSensors& ldr3, ldrSensors& ldr4)
: _ldr1(ldr1), _ldr2(ldr2), _ldr3(ldr3), _ldr4(ldr4)
{

}

int drejeBaenkDecoder::decipher()
{
    std::array<int, 4> bits = readBits();

    for (int i=0; i < CIFFERTABEL.size(); i++)
    {
        if(CIFFERTABEL[i] == bits)
    {
        return i+1;
        
    }
    }
    return -1;
    
}

std::array<int, 4> drejeBaenkDecoder::readBits()
{
    return std::array<int, 4>
    {
        _ldr1.interpretValue(),
        _ldr2.interpretValue(),
        _ldr3.interpretValue(),
        _ldr4.interpretValue()
    };
}

void drejeBaenkDecoder::saveAllValues()
{
    _ldr1.saveValue();
    _ldr2.saveValue();
    _ldr3.saveValue();
    _ldr4.saveValue();
}


void drejeBaenkDecoder::timeBasedCalibration(DcMotor& motor, uint32_t calibrationTimeMs)
{

    //Clear values, but we only do one calibration at the beginning


    uint32_t startTime = to_ms_since_boot(get_absolute_time());

    motor.forwards();

    while ((to_ms_since_boot(get_absolute_time()) - startTime) < calibrationTimeMs)
    {
        saveAllValues();
        sleep_ms(2);
    }
    motor.stop();

    motor.backwards();

    while ((to_ms_since_boot(get_absolute_time()) - startTime) < calibrationTimeMs * 2)
    {
        saveAllValues();
        sleep_ms(2);
    }
    motor.stop();

    _ldr1.totalCalibration();
    _ldr2.totalCalibration();
    _ldr3.totalCalibration();
    _ldr4.totalCalibration();

}
