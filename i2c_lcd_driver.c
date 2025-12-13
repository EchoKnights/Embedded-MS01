#include "i2c_lcd_driver.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

i2c_inst_t port; uint sda; uint scl;

void init_i2c_pins(i2c_inst_t *i2c_port, uint sda_pin, uint scl_pin){    // Init I2C
    i2c_init(i2c_port, 50 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
    port = *i2c_port;
    sda = sda_pin;
    scl = scl_pin;
}

// --- Low level write to PCF8574 ---
void pcf8574_write(i2c_inst_t *i2c_port, uint8_t data){
    int rc = i2c_write_blocking(i2c_port, LCD_ADDRESS, &data, 1, false);
    if (rc < 0) {
        init_i2c_pins(&port, sda, scl);
    }
}

// --- Pulse Enable Line ---
void lcd_pulse(i2c_inst_t *i2c_port, uint8_t data) {
    pcf8574_write(i2c_port, data | LCD_ENABLE);
    sleep_us(500);
    pcf8574_write(i2c_port, data & ~LCD_ENABLE);
    sleep_us(500);
}

// --- Send 4-bit nibble ---
void lcd_write4(i2c_inst_t *i2c_port, uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | LCD_BACKLIGHT | mode;
    pcf8574_write(i2c_port, data);
    lcd_pulse(i2c_port, data);
}

// --- Send full byte: high nibble, then low nibble ---
void lcd_send(i2c_inst_t *i2c_port, uint8_t value, uint8_t mode) {
    lcd_write4(i2c_port, value & 0xF0, mode);
    lcd_write4(i2c_port, (value << 4) & 0xF0, mode);
}

// --- Wrapper helpers ---
void lcd_cmd(i2c_inst_t *i2c_port, uint8_t cmd) {
    lcd_send(i2c_port, cmd, 0x00);
    sleep_ms(2);
}

// --- Initialization sequence ---
void lcd_init(i2c_inst_t *i2c_port) {

    lcd_write4(i2c_port, 0x30, 0); 
    sleep_ms(5);

    lcd_write4(i2c_port, 0x30, 0); 
    sleep_us(200);

    lcd_write4(i2c_port, 0x30, 0);
    lcd_write4(i2c_port, 0x20, 0);   // switch to 4-bit mode

    lcd_cmd(i2c_port, LCD_4BITMODE);
    lcd_cmd(i2c_port, LCD_DISPLAYON);
    lcd_cmd(i2c_port, LCD_ENTRYMODE);
    lcd_clear(i2c_port);
}

void lcd_write_char(i2c_inst_t *i2c_port, char c) {
    lcd_send(i2c_port, c, LCD_RS);
}

// --- Clear display ---
void lcd_clear(i2c_inst_t *i2c_port) {
    lcd_cmd(i2c_port, LCD_CLEARDISPLAY);
    sleep_ms(2);
}

void lcd_print(i2c_inst_t *i2c_port, const char *text) {
    while (*text) {
        lcd_write_char(i2c_port, *text++);
    }
}

void lcd_move_cursor_next_line(i2c_inst_t *i2c_port){
    lcd_cmd(i2c_port, LCD_NEXT_LINE);
}