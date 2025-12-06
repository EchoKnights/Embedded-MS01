#include "motors_driver.h"
#include <stdint.h>
#include "pico/stdlib.h"
#include <stdio.h>

void init_motor_pins(uint potentiometer_pin, uint pin_adc_channel, uint forward_pin, uint reverse_pin){
    adc_init();
    adc_gpio_init(potentiometer_pin);
    adc_select_input(pin_adc_channel);
    gpio_init(forward_pin);
    gpio_init(reverse_pin);
    gpio_set_dir(forward_pin, GPIO_OUT);
    gpio_set_dir(reverse_pin, GPIO_OUT);
}

void motors_stop(uint forward_pin, uint reverse_pin) {
    gpio_put(forward_pin, 0);
    gpio_put(reverse_pin, 0);
}

void motors_forward(uint forward_pin, uint reverse_pin) {
    gpio_put(forward_pin, 1);
    gpio_put(reverse_pin, 0);
}

void motors_backward(uint forward_pin, uint reverse_pin) {
    gpio_put(forward_pin, 0);
    gpio_put(reverse_pin, 1);
}

void control_motors_with_potentiometer(uint potentiometer_pin, uint forward_pin, uint reverse_pin){
        uint16_t raw = adc_read();
        float norm = raw / 4095.0f;
        printf("Normalized value: %f\n", norm);
        if (norm < 0.30f) {
            motors_backward(forward_pin, reverse_pin);
            printf("Moving backward\n");
        } else if (norm > 0.80f) {
            motors_forward(forward_pin, reverse_pin);
            printf("Moving forward\n");
        } else {
            motors_stop(forward_pin, reverse_pin);
        }
}