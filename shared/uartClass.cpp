#include "uartClass.h"
#include <iostream> 

static uart_inst_t* getUartFromId(int uart_id) {
    if (uart_id == 0) {
        return uart0;
    }
    if (uart_id == 1) {
        return uart1;
    }
    return nullptr;
}

UartClass::UartClass(int uart_id, int pin_tx, int pin_rx, int baud){
    this->uart_id = uart_id;
    this->pin_tx = pin_tx;
    this->pin_rx = pin_rx;
    this->baud = baud;
}

void UartClass::setup() {
    uart = getUartFromId(uart_id);

    if (uart == nullptr) {
        printf("INVALID UART ID\n");
        return;
    }

    uart_init(uart, baud);
    gpio_set_function(pin_tx, GPIO_FUNC_UART);
    gpio_set_function(pin_rx, GPIO_FUNC_UART);

    uart_set_format(uart, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(uart, true);
}


void UartClass::writePackage(std::string line) {
    uart_write_blocking(uart,
                        reinterpret_cast<const uint8_t*>(line.c_str()),
                        line.length());
}

std::string UartClass::readPackage() {
    while (!hasPackage()) {
        sleep_ms(1);
    }

    std::string line = rxBuffer;
    rxBuffer.clear();
    packageReady = false;
    return line;
}

bool UartClass::hasPackage() {
    if (packageReady) {
        return true;
    }

    while (uart_is_readable(uart)) {
        char c = static_cast<char>(uart_getc(uart));

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            packageReady = true;
            return true;
        }

        rxBuffer += c;
    }

    return false;
}
