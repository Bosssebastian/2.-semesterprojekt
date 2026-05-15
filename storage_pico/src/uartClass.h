#ifndef uartClass_h
#define uartClass_h

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <string>
#include <vector>


class UartClass {
public:

    UartClass(int uart_id, int pin_tx, int pin_rx, int baud);
    void setup();

    std::vector<std::string> split(const std::string& line) const;

    void sendLine(std::string line); //writePackage
    std::string getLine(); //readPackage
    bool hasLine(); //hasPackage

    void clearRXQue();
    //&result is reference parameter used as output parameter
    
    //bool requestPackage(uint8_t msgType, uint8_t &result, uint32_t ackTimeout_ms = 500, uint32_t resTimeout_ms = 500);

    //bool tryReadPackage(uint8_t &flag, uint8_t &msgType, uint8_t &data);

private:
    uart_inst_t* uart = nullptr;

    int uart_id;
    int pin_tx;
    int pin_rx;
    int baud;
    
    std::string rxBuffer;
    bool mLineReady = false;
};

#endif
