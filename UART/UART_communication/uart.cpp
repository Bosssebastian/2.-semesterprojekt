#include "uart.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"

Uart::Uart(int uart_id, int tx, int rx, int baud)
    : uart_id(uart_id), tx_pin(tx), rx_pin(rx), baudrate(baud) {}

void Uart::init() {

}

void Uart::send(char c) {

}

void Uart::receive() {

}