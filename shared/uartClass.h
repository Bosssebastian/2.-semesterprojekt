#ifndef uartClass_h
#define uartClass_h

#include "pico/stdlib.h"
#include "hardware/uart.h"



class UartClass {
public:

    //Constants for the UART-Class (uint8_t is an unsigned 8-bit integer)
    static const uint8_t REQ = 1;
    static const uint8_t ACK = 2;
    static const uint8_t RES = 3;
    static const uint8_t ERR = 4;

    static const uint8_t GET_CURRENT = 1;
    static const uint8_t GET_POSITION = 2;
    static const uint8_t CHANGE_POSITION = 3;

    static const uint8_t packageSize = 4;

    static const uint8_t START = 0xAA;

    // Constructor for the UartClass
    UartClass(int uart_id, int pin_tx, int pin_rx, int baud);

    // Methods for Uart-Class
    void writePackage(uint8_t flag, uint8_t msgType, uint8_t data);
    //readPackage uses reference parameters to output the read values
    void readPackage(uint8_t &flag, uint8_t &msgType, uint8_t &data);
    void clearRXQue();
    //&result is reference parameter used as output parameter
    bool requestPackage(uint8_t msgType, uint8_t &result, uint32_t ackTimeout_ms = 500, uint32_t resTimeout_ms = 500);

    bool tryReadPackage(uint8_t &flag, uint8_t &msgType, uint8_t &data);

private:
    uart_inst_t* uart;
    int pin_tx;
    int pin_rx;
    int baud;
};

#endif
