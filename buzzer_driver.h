#ifndef BUZZER_DRIVER_H
#define BUZZER_DRIVER_H

#include <stdint.h>
#include "pico/stdlib.h"

void init_buzzer_pin(uint buzzer_pin);

void switch_buzzer_on(uint buzzer_pin);

void switch_buzzer_off(uint buzzer_pin);

#endif