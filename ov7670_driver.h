#ifndef OV7670_DRIVER_H
#define OV7670_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#define FRAME_WIDTH   320
#define FRAME_HEIGHT  240

#define SMALL_WIDTH   160
#define SMALL_HEIGHT  120

extern uint8_t frame_buffer[FRAME_HEIGHT][FRAME_WIDTH];
extern uint8_t small_frame[SMALL_HEIGHT][SMALL_WIDTH];

void cam_capture_frame(void);
void downscale_2x2(void);



// --- Pin definitions (match your wiring) ---
#define CAM_D0   0
#define CAM_D1   1
#define CAM_D2   2
#define CAM_D3   3
#define CAM_D4   4
#define CAM_D5   5
#define CAM_D6   6
#define CAM_D7   7
#define CAM_SDA  8
#define CAM_SCL  9
#define CAM_VSYNC 10
#define CAM_HREF  11
#define CAM_PCLK  12
#define CAM_XCLK  13

#define CAM_I2C   i2c0
#define OV7670_ADDR 0x21    // 7-bit SCCB address (0x42/0x43 >> 1)

// init functions
void ov7670_init_pins(void);
void ov7670_start_xclk(void);
void ov7670_init_i2c(void);

// simple register R/W
bool ov7670_read_reg(uint8_t reg, uint8_t *value);
bool ov7670_write_reg(uint8_t reg, uint8_t value);

// config structs
typedef struct {
    uint8_t reg;
    uint8_t val;
} regpair_t;

extern const regpair_t qvga_yuv[];
void ov7670_apply_config(const regpair_t *cfg);

#endif
