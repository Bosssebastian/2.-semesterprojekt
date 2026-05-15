#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "dcMotor.h"
#include "hardware/pwm.h"

DcMotor::DcMotor(uint pwmPin, uint input1, uint input2) {
    this->pwmPin = pwmPin; // pin 0 form the pico this is 1,2EN form the L293D
    this->input1 = input1; //pin 1 from the pico this is 1Y from the L293D
    this->input2 = input2; //pin 11 from the pico this is 2Y from the L293D
    this->slice = pwm_gpio_to_slice_num(pwmPin);
    this->duty = 650; // divide by 1000 to find duty cycle in %

    //initializing pwm in the following block
    gpio_init(this->pwmPin);
    gpio_set_function(this->pwmPin, GPIO_FUNC_PWM);
    pwm_set_wrap(this->slice, 1000);
    uint channel = pwm_gpio_to_channel(pwmPin);
    pwm_set_chan_level(this->slice, channel, duty); 
    pwm_set_enabled(this->slice, true);

    //initializin input1
    gpio_init(this->input1);
    gpio_set_dir(this->input1, GPIO_OUT);


    //initializing input2
    gpio_init(this->input2);
    gpio_set_dir(this->input2, GPIO_OUT);
}

void DcMotor::forwards() {
    gpio_put(input1, 1);
    gpio_put(input2, 0);
}

void DcMotor::backwards() {
    gpio_put(input1, 0);
    gpio_put(input2, 1);
}

void DcMotor::stop(){
    gpio_put(input1, 0);
    gpio_put(input2, 0);
}



