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

UartClass::UartClass(int uart_id, int pin_tx, int pin_rx, int baud)
    
{

    this->pin_tx = pin_tx;
    this->pin_rx = pin_rx;
    this->baud = baud;
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


void UartClass::writePackage(uint8_t flag, uint8_t msgType, uint8_t data) {
    //printf("Writing to UART\n");
    uint8_t package[packageSize] = {START, flag, msgType, data};
    uart_write_blocking(uart, package, packageSize);
}

void UartClass::readPackage(uint8_t &flag, uint8_t &msgType, uint8_t &data) {
    //printf("Reading package\n");
    
    uint8_t byteRecieved = 0;

    do {
        byteRecieved = uart_getc(uart);

    } while (byteRecieved !=START);

    flag = uart_getc(uart);
    msgType = uart_getc(uart);
    data = uart_getc(uart);

    
}

void UartClass::clearRXQue() {
    printf("Clearing RX Que\n");
    while (uart_is_readable(uart)) {
        uart_getc(uart);
    }

}

bool UartClass::tryReadPackage(uint8_t &flag, uint8_t &msgType, uint8_t &data) {
    static bool synced = false;
    static uint8_t buffer[3];
    static int index = 0;

    while (uart_is_readable(uart)) {
        uint8_t b = uart_getc(uart);

        if (!synced) {
            if (b == START) {
                synced = true;
                index = 0;
            }
            continue;
        }

        buffer[index++] = b;

        if (index == 3) {
            flag = buffer[0];
            msgType = buffer[1];
            data = buffer[2];

            synced = false;
            index = 0;
            return true;
        }

    }
    
    return false;
}
/*
void awaitPackage(uint8_t flag, uint8_t msgType, uint8_t data) {
    
}
*/
bool UartClass::requestPackage(uint8_t msgType, uint8_t &result, uint32_t ackTimeout_ms, uint32_t resTimeout_ms) {
    printf("Sending Request via UART\n");

    clearRXQue();
    writePackage(REQ, msgType, 0);

    bool gotAck = false;
    uint8_t flag = 0;
    uint8_t recievedType = 0;
    uint8_t data = 0;

    uint32_t start = to_ms_since_boot(get_absolute_time());

    while ((to_ms_since_boot(get_absolute_time()) - start) < ackTimeout_ms) {
        if (tryReadPackage(flag, recievedType, data)) {
            printf("During ACK wait got: %d %d %d\n", flag, recievedType, data);
            if (flag == ACK && recievedType == msgType) {
                gotAck = true;
                printf("ACK matched\n");
                break;
            }
        }
        sleep_ms(5);
    }
    
    if (!gotAck) {
        printf("NO ACK RECIEVED, TIMED OUT!\n");
        return false;
    }

    start = to_ms_since_boot(get_absolute_time());

    while ((to_ms_since_boot(get_absolute_time()) - start) < resTimeout_ms) {
        if (tryReadPackage(flag, recievedType, data)) {
            printf("During RES wait got: %d %d %d\n", flag, recievedType, data);
            if (flag == RES && recievedType == msgType) {
                result = data;
                printf("RES matched, result = %D\n", result);
                return true;
            }
        }
        sleep_ms(5);
    }

    printf("NO RES RECIEVED, TIMED OUT!\n");
    return false;
    
}

