#ifndef MOTORS_DRIVER_H
#define MOTORS_DRIVER_H

#include "hardware/adc.h"
#include <stdint.h>
#include "pico/stdlib.h"

void init_motor_pins(uint potentiometer_pin, uint pin_adc_channel, uint forward_pin, uint reverse_pin);

void motors_stop(uint forward_pin, uint reverse_pin);

void motors_forward(uint forward_pin, uint reverse_pin);

void motors_backward(uint forward_pin, uint reverse_pin);

void control_motors_with_potentiometer(uint potentiometer_pin, uint forward_pin, uint reverse_pin);

#endif