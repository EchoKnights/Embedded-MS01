#include "buzzer_driver.h"
#include <stdint.h>
#include "pico/stdlib.h"

void init_buzzer_pin(uint buzzer_pin){
    gpio_init(buzzer_pin);
    gpio_set_dir(buzzer_pin, GPIO_OUT);
}

void switch_buzzer_on(uint buzzer_pin){
    gpio_put(buzzer_pin, 1);
}

void switch_buzzer_off(uint buzzer_pin){
    gpio_put(buzzer_pin, 0);
}