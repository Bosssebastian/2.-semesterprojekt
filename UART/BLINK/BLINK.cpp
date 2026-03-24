#include <stdio.h>
#include "pico/stdlib.h"

int main()
{
    stdio_init_all();
    sleep_ms(2000);   // gives USB serial time to appear after reset

    const uint LED_PIN = 25;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        printf("Hello, world!\n");

        gpio_put(LED_PIN, 1);
        sleep_ms(200);

        gpio_put(LED_PIN, 0);
        sleep_ms(200);
    }
}