#ifndef WEBCAM_DRIVER_H
#define WEBCAM_DRIVER_H

#include <stdint.h>
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

#define MAX_LINE 48
#define HOLD_MS 5000           // time input must be continuously asserted to trigger next step
#define HEARTBEAT_TIMEOUT_MS 1200  // if no messages seen for this long, consider input released
#define USB_NO_DATA     (-1.0f)
#define USB_PARSE_ERROR (-2.0f)

float poll_usb_distance(void);

// bool control_while_maintaining_assert_for(void (*function_ptr)(uint, uint, uint), float time_seconds);

#endif