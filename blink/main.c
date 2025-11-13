#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"

int main() {

    const uint r_pin = 6;
    const uint g_pin = 7;
    const uint b_pin = 8;

    // Initialize LED pin
    gpio_init(r_pin);
    gpio_init(g_pin);
    gpio_init(b_pin);
    gpio_set_dir(r_pin, GPIO_OUT);
    gpio_set_dir(g_pin, GPIO_OUT);
    gpio_set_dir(b_pin, GPIO_OUT);

    // Initialize chosen serial port
    stdio_init_all();

    // Loop forever
    while (true) {

        //Sequence one - Sequential Activation (Red, Green, Blue)
        sleep_ms(5000);
        gpio_put(r_pin, true);
        sleep_ms(1000);
        gpio_put(r_pin, false);
        gpio_put(g_pin, true);
        sleep_ms(1000);
        gpio_put(g_pin, false);
        gpio_put(b_pin, true);
        sleep_ms(1000);
        gpio_put(b_pin, false);

        //Sequence two - Simultaneous Activation (Red and Green and Blue)
        sleep_ms(500);
        gpio_put(r_pin, true);
        gpio_put(g_pin, true);
        gpio_put(b_pin, true);
        sleep_ms(2000);
        gpio_put(r_pin, false);
        gpio_put(g_pin, false);
        gpio_put(b_pin, false);
    }
}