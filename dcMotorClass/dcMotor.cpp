#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"

DcMotor::DcMotor(uint pwmPin, uint input1, uint input2) {
    this->pwmPin; // pin 0 form the pico this is 1,2EN form the L293D
    this->input1; //pin 1 from the pico this is 1Y from the L293D
    this->input2; //pin 11 from the pico this is 2Y from the L293D
    gpio_init(pwmPin);
    gpio_set_dir(pwmPin, GPIO_OUT);

    gpio_init(input1);
    gpio_set_dir(input1, GPIO_OUT);

    gpio_init(input2);
    gpio_set_dir(input2, GPIO_OUT);
}

void DcMotor::forwards() {
 
}

void DcMotor::backwards() {

}

void DcMotor::stop(){

}



