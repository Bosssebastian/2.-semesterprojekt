#include "Uart.h"
#include "pico/stdlib.h"
#include "hardware/uart.h"

Uart::Uart(int uart_id, int tx, int rx, int baud)
    : uart_id(uart_id), tx_pin(tx), rx_pin(rx), baudrate(baud) {}

void Uart::init() {
    uart_inst_t* uart = (uart_id == 0) ? uart0 : uart1;

    uart_init(uart, baudrate);

    gpio_set_function(tx_pin, GPIO_FUNC_UART);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
}

void Uart::send(char c) {
    uart_inst_t* uart = (uart_id == 0) ? uart0 : uart1;

    uart_putc(uart, c);
}

char Uart::receive() {
    uart_inst_t* uart = (uart_id == 0) ? uart0 : uart1;

    if (uart_is_readable(uart)) {
        return uart_getc(uart);
    }
    return '\0';
}