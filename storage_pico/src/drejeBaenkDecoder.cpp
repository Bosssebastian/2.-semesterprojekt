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

