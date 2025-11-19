#include "keypad_driver.h"
#include <stdio.h>
#include "pico/stdlib.h"

void init_keypad_pins(uint row1_pin, uint row2_pin, uint row3_pin, uint row4_pin, uint col1_pin, uint col2_pin, uint col3_pin){
    gpio_init(row1_pin);
    gpio_init(row2_pin);
    gpio_init(row3_pin);
    gpio_init(row4_pin);
    gpio_set_dir(row1_pin, GPIO_OUT);
    gpio_set_dir(row2_pin, GPIO_OUT);
    gpio_set_dir(row3_pin, GPIO_OUT);
    gpio_set_dir(row4_pin, GPIO_OUT);
    gpio_init(col1_pin);
    gpio_init(col2_pin);
    gpio_init(col3_pin);
    gpio_set_dir(col1_pin, GPIO_IN);
    gpio_set_dir(col2_pin, GPIO_IN);
    gpio_set_dir(col3_pin, GPIO_IN);
}

char determine_key_pressed(uint row1_pin, uint row2_pin, uint row3_pin, uint row4_pin, uint col1_pin, uint col2_pin, uint col3_pin){
    bool c1;
    bool c2;
    bool c3;
    gpio_put(row1_pin, true);
    sleep_ms(5);
    c1 = gpio_get(col1_pin);
    c2 = gpio_get(col2_pin);
    c3 = gpio_get(col3_pin);
    if(c1) { gpio_put(row1_pin, false); return '1'; }
    if(c2) { gpio_put(row1_pin, false); return '2'; }
    if(c3) { gpio_put(row1_pin, false); return '3'; }
    gpio_put(row1_pin, false);
    
    gpio_put(row2_pin, true);
    sleep_ms(5);
    c1 = gpio_get(col1_pin);
    c2 = gpio_get(col2_pin);
    c3 = gpio_get(col3_pin);
    if(c1) { gpio_put(row2_pin, false); return '4'; }
    if(c2) { gpio_put(row2_pin, false); return '5'; }
    if(c3) { gpio_put(row2_pin, false); return '6'; }
    gpio_put(row2_pin, false);
    
    gpio_put(row3_pin, true);
    sleep_ms(5);
    c1 = gpio_get(col1_pin);
    c2 = gpio_get(col2_pin);
    c3 = gpio_get(col3_pin);
    if(c1) { gpio_put(row3_pin, false); return '7'; }
    if(c2) { gpio_put(row3_pin, false); return '8'; }
    if(c3) { gpio_put(row3_pin, false); return '9'; }
    gpio_put(row3_pin, false);
    
    gpio_put(row4_pin, true);
    sleep_ms(5);
    c1 = gpio_get(col1_pin);
    c2 = gpio_get(col2_pin);
    c3 = gpio_get(col3_pin);
    if(c1) { gpio_put(row4_pin, false); return '*'; }
    if(c2) { gpio_put(row4_pin, false); return '0'; }
    if(c3) { gpio_put(row4_pin, false); return '#'; }
    gpio_put(row4_pin, false);
    
    return 'N';
}