#include "pico/stdlib.h"
#include "Uart.h"

int main() {
    stdio_init_all();

    const uint LED_PIN = 25;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    Uart uart1(1, 4, 5, 115200);
    uart1.init();

    while (true) {
        printf("Sending A\n");

        uart1.send('A');

        gpio_put(LED_PIN, 1);
        sleep_ms(500);
        gpio_put(LED_PIN, 0);
        sleep_ms(500);
    }
}