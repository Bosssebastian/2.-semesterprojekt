#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all(); // required for USB/UART stdio

    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

    
    while (true) {
        gpio_put(25, 1);
        printf("t=%d, led=%d\n", to_ms_since_boot(get_absolute_time()), 1);
        sleep_ms(1000);
        gpio_put(25, 0);
        printf("t=%d, led=%d\n", to_ms_since_boot(get_absolute_time()), 0);
        sleep_ms(1000);
    }
}