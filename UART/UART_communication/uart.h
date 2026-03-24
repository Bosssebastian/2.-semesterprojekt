#pragma once 

class uart {
private:
    int uart_id; //uart1 or uart 0
    int tx_pin;
    int rx_pin;
    int baudrate;

public;
    Uart(int uart_id, int tx, int rx, int baud);

    void init();
    void send(char c);
    char receive();
};