#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define POT_ADC_PIN 28

#define MOT_FWD 14
#define MOT_REV 15


void motors_stop(void) {
    gpio_put(MOT_FWD, 0);
    gpio_put(MOT_REV, 0);
}

void motors_forward(void) {
    gpio_put(MOT_FWD, 1);
    gpio_put(MOT_REV, 0);
}

void motors_backward(void) {
    gpio_put(MOT_FWD, 0);
    gpio_put(MOT_REV, 1);
}


int main()
{
    stdio_init_all();

    adc_init();
    adc_gpio_init(POT_ADC_PIN);
    adc_select_input(2);

    gpio_init(MOT_FWD);
    gpio_init(MOT_REV);

    gpio_set_dir(MOT_FWD, GPIO_OUT);
    gpio_set_dir(MOT_REV, GPIO_OUT);

    motors_stop();

    while (true) {
        uint16_t raw = adc_read();
        float norm = raw / 4095.0f;

        if (norm < 0.45f) {
            motors_backward();
        } else if (norm > 0.65f) {
            motors_forward();
        } else {
            motors_stop();
        }

        printf("norm=%.2f\n", norm);
        sleep_ms(50);
    }
}
