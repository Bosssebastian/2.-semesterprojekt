#include "pico/stdlib.h"
#ifndef DCMOTOR_H
#define DCMOTOR_H

class DcMotor {
public:
    DcMotor(uint pwmPin, uint input1, uint input2);
    void forwards();
    void backwards();
    void stop();

private:
    uint pwmPin;
    uint input1;
    uint input2;
    uint slice;
    uint duty;
};

#endif