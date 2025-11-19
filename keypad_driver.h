#ifndef KEYPAD_DRIVER_H
#define KEYPAD_DRIVER_H

#include <stdint.h>
#include "pico/stdlib.h"

void init_keypad_pins(uint row1_pin, uint row2_pin, uint row3_pin, uint row4_pin, uint col1_pin, uint col2_pin, uint col3_pin);

char determine_key_pressed(uint row1_pin, uint row2_pin, uint row3_pin, uint row4_pin, uint col1_pin, uint col2_pin, uint col3_pin);

#endif