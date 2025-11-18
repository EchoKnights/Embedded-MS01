#include "ov7670_driver.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "ov7670_driver.h"
#include "hardware/gpio.h"

uint8_t frame_buffer[FRAME_HEIGHT][FRAME_WIDTH];
uint8_t small_frame[SMALL_HEIGHT][SMALL_WIDTH];

void downscale_2x2(void) {
    for (int y = 0; y < SMALL_HEIGHT; y++) {
        int src_y = y * 2;
        for (int x = 0; x < SMALL_WIDTH; x++) {
            int src_x = x * 2;

            uint16_t s =
                frame_buffer[src_y][src_x] +
                frame_buffer[src_y][src_x + 1] +
                frame_buffer[src_y + 1][src_x] +
                frame_buffer[src_y + 1][src_x + 1];

            small_frame[y][x] = (uint8_t)(s / 4);
        }
    }
}

void cam_capture_frame(void) {
    // Wait for: VSYNC high -> low = start of a new frame
    while (!gpio_get(CAM_VSYNC));   // wait until VSYNC = 1
    while ( gpio_get(CAM_VSYNC));   // wait until it goes back to 0

    for (int y = 0; y < FRAME_HEIGHT; y++) {

        // Wait for start of active line (HREF high)
        while (!gpio_get(CAM_HREF)) {
            // if VSYNC goes high, frame restarted unexpectedly
            if (gpio_get(CAM_VSYNC)) return;
        }

        int x = 0;

        while (gpio_get(CAM_HREF) && x < FRAME_WIDTH) {
            // --- Read Y byte (luma) ---
            while (!gpio_get(CAM_PCLK));   // wait rising edge
            uint8_t y_byte = (gpio_get_all() >> CAM_D0) & 0xFF;
            while ( gpio_get(CAM_PCLK));   // wait falling edge

            frame_buffer[y][x++] = y_byte;

            // --- Skip chroma byte (U/V) ---
            while (!gpio_get(CAM_PCLK));
            (void)(gpio_get_all());
            while ( gpio_get(CAM_PCLK));
        }

        // If line ended early, pad the rest with zeros
        while (x < FRAME_WIDTH) {
            frame_buffer[y][x++] = 0;
        }

        // Wait for HREF to go low before next line
        while (gpio_get(CAM_HREF)) {
            if (gpio_get(CAM_VSYNC)) return;
        }
    }
}


const regpair_t qvga_yuv[] = {
    {0x12, 0x80}, // Reset
    {0x12, 0x00}, // YUV mode
    {0x11, 0x01}, // Prescaler for ~24MHz input (adjust if needed)
    {0x0C, 0x00}, // YUV output sequence
    {0x3E, 0x00}, //
    {0x70, 0x3A}, // Scaling
    {0x71, 0x35},
    {0x72, 0x11},
    {0x73, 0xF0},
    {0xA2, 0x02}, // YUV mode
    {0x15, 0x00}, // Full output range
    {0x17, 0x16}, {0x18, 0x04}, // HStart/HStop adjust QVGA
    {0x32, 0xA4}, {0x19, 0x02}, {0x1A, 0x7A},
    {0x03, 0x0A}, // Vref adjust
    {0xFF, 0xFF}  // END
};

void ov7670_apply_config(const regpair_t *cfg) {
    while (cfg->reg != 0xFF) {
        ov7670_write_reg(cfg->reg, cfg->val);

        if (cfg->reg == 0x12 && cfg->val == 0x80) {
            // Special case: reset requires a longer wait
            sleep_ms(10);
        } else {
            sleep_ms(2);
        }

        cfg++;
    }
}

// Initialize GPIO directions & functions (but don’t start anything yet)
void ov7670_init_pins(void) {
    // XCLK (we'll drive this with PWM later)
    gpio_set_function(CAM_XCLK, GPIO_FUNC_PWM);

    // I2C pins
    gpio_set_function(CAM_SDA, GPIO_FUNC_I2C);
    gpio_set_function(CAM_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(CAM_SDA);
    gpio_pull_up(CAM_SCL);

    // Data bus as inputs for later
    const uint cam_data_pins[] = {
        CAM_D0, CAM_D1, CAM_D2, CAM_D3,
        CAM_D4, CAM_D5, CAM_D6, CAM_D7
    };
    for (int i = 0; i < 8; i++) {
        gpio_init(cam_data_pins[i]);
        gpio_set_dir(cam_data_pins[i], GPIO_IN);
    }

    // Sync & pixel clock inputs
    gpio_init(CAM_VSYNC);
    gpio_set_dir(CAM_VSYNC, GPIO_IN);

    gpio_init(CAM_HREF);
    gpio_set_dir(CAM_HREF, GPIO_IN);

    gpio_init(CAM_PCLK);
    gpio_set_dir(CAM_PCLK, GPIO_IN);
}

// Drive XCLK on CAM_XCLK using PWM (~20 MHz)
void ov7670_start_xclk(void) {
    uint slice_num = pwm_gpio_to_slice_num(CAM_XCLK);

    pwm_config cfg = pwm_get_default_config();

    // clk = 125 MHz / (clkdiv * (TOP + 1))
    // clkdiv = 1.25, TOP = 4 -> 125e6 / (1.25 * 5) = 20 MHz
    pwm_config_set_clkdiv(&cfg, 1.25f);
    pwm_config_set_wrap(&cfg, 4);

    pwm_init(slice_num, &cfg, false);

    // ~50% duty cycle: half of (TOP + 1) = 5 -> level = 2 or 3
    pwm_set_gpio_level(CAM_XCLK, 2);

    pwm_set_enabled(slice_num, true);
}


void ov7670_init_i2c(void) {
    i2c_init(CAM_I2C, 100 * 1000);  // 100 kHz
    // pins already set to I2C function in ov7670_init_pins()
}

// Write a single OV7670 register
bool ov7670_write_reg(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = { reg, value };
    int ret = i2c_write_blocking(CAM_I2C, OV7670_ADDR, buf, 2, false);
    return ret >= 0;
}

// Read a single OV7670 register (SCCB-friendly: write with STOP, then read)
bool ov7670_read_reg(uint8_t reg, uint8_t *value) {
    // 1) Send register address (WRITE, with STOP)
    int ret = i2c_write_blocking(CAM_I2C, OV7670_ADDR, &reg, 1, false);
    if (ret < 0) {
        printf("[OV7670] i2c_write_blocking for reg 0x%02X failed (ret=%d)\n", reg, ret);
        return false;
    }

    // 2) Read back 1 byte (READ)
    ret = i2c_read_blocking(CAM_I2C, OV7670_ADDR, value, 1, false);
    if (ret < 0) {
        printf("[OV7670] i2c_read_blocking for reg 0x%02X failed (ret=%d)\n", reg, ret);
        return false;
    }

    return true;

}
