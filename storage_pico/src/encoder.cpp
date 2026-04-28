#include "encoder.h"

ADS7830::ADS7830(i2c_inst_t* i2cPort, uint sdaPin, uint sclPin, uint32_t baurate, uint8_t i2cAddress)
    : _i2cPort(i2cPort), _sdaPin(sdaPin), _sclPin(sclPin), _baurate(baurate), _i2cAddress(i2cAddress)
{
    
}

void ADS7830::init() {
    i2c_init(_i2cPort, _baurate);
    gpio_set_function(_sdaPin, GPIO_FUNC_I2C);
    gpio_set_function(_sclPin, GPIO_FUNC_I2C);
    gpio_pull_up(_sdaPin);
    gpio_pull_up(_sclPin);

}

uint8_t ADS7830::channelCode(uint8_t ch) {
    if (ch > 7)
    {
        return 0; // Invalid channel, default to 0
    }
    return ((ch >> 1) | ((ch & 1) << 2)) & 0x07; // Convert channel number to control byte
}

uint8_t ADS7830::readChannel(uint8_t channel) {
    uint8_t code = channelCode(channel);
    uint8_t controlByte = 0x80 | (code << 4) |0x04; // Start bit + channel code + single-ended mode
    uint8_t result  = 0;

    i2c_write_blocking(_i2cPort, _i2cAddress, &controlByte, 1, true); // Send control byte with repeated start
    i2c_read_blocking(_i2cPort, _i2cAddress, &result, 1, false); // Read data byte

    return result;
}
/*
int ADS7830::getPosition() {
    uint8_t ch[8];
    
    for( int i = 0; i < 8; ++i) {
        ch[i] = readChannel(i);
    }

    // convert analog to binary (threshold)
    int bits[4];
    bits[0] = ch[0] > 128 ? 1 : 0;
    bits[1] = ch[1] > 128 ? 1 : 0;
    bits[2] = ch[2] > 128 ? 1 : 0;
    bits[3] = ch[3] > 128 ? 1 : 0;

    // match against ciffertabel
    for(int i = 0; i < 0; ++i) {
        bool match = true;
        for(int j = 0; j < 4; ++j) {
            if(bits[j] !=  CIFFERTABEL[i][j]) {
                match = false;
                break;
            }
        }
        if(match) {
            return i + 1; 
        }
    }

    return 0; // if no valid position is found
}
    */