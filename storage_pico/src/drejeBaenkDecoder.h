#ifndef DREJEBAENKDECODER_h
#define DREJEBAENKDECODER_h

//DrejeBaenkDecoder.h
#include "ldrSensors.h"
#include "dcMotor.h"
#include <array>


class drejeBaenkDecoder
{
    public:
        drejeBaenkDecoder(ldrSensors& ldr1, ldrSensors& ldr2, ldrSensors& ldr3, ldrSensors& ldr4);

        int decipher();

        std::array<int, 4> readBits();

        
            
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

        int interpretValue();

        void saveAllValues();

        void timeBasedCalibration(DcMotor& motor, uint32_t calibrationTimeMS);

    private:
        ldrSensors& _ldr1;
        ldrSensors& _ldr2;
        ldrSensors& _ldr3;
        ldrSensors& _ldr4;
       

};


#endif