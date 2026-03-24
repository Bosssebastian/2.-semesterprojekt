#include <cstdio>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);

    while (true) {
        printf("A sent\n");
        sleep_ms(1000);
    }

    return 0;
}