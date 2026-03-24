#include <stdio.h>
#include "pico/stdlib.h"


int main()
{
    stdio_init_all();

    const uint LED_PIN = 25;

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);

        gpio_put(LED_PIN, 1);  // LED ON
        sleep_ms(500);

        gpio_put(LED_PIN, 0);  // LED OFF
        sleep_ms(500);

    }
}

