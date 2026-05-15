#ifndef uartClass_h
#define uartClass_h

#include <string>
#include <vector>


class UartClass {
public:

    UartClass(int pin_tx, int pin_rx, int baud);
    void setup();

    std::vector<std::string> split(const std::string& line) const;

    void sendLine(std::string line); //writePackage
    std::string getLine(); //readPackage
    bool hasLine(); //hasPackage

private:
    int serial_fd = -1;

    int uart_id;
    int pin_tx;
    int pin_rx;
    int baud;
    
    std::string rxBuffer;
    bool mLineReady = false;
};

#endif
