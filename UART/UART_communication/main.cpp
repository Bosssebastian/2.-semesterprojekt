#include <cstdio>
#include "pico/stdlib.h"

int main() {
    Uart uart0(0, 0, 1, 115200);  // UART0, pins GP0/GP1
    uart0.init();

    uart0.send('A');
}