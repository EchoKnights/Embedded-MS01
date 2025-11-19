#ifndef I2C_LCD_DRIVER_H
#define I2C_LCD_DRIVER_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// PCF8574 pin bits
#define LCD_BACKLIGHT   0x08     // backlight enable bit
#define LCD_ENABLE      0x04     // EN bit
#define LCD_RS          0x01     // Register select (0=command, 1=data)

// Command flags
#define LCD_4BITMODE        0x28
#define LCD_DISPLAYON       0x0C
#define LCD_CLEARDISPLAY    0x01
#define LCD_ENTRYMODE       0x06
#define LCD_NEXT_LINE       0xC0

#define LCD_ADDRESS         0x27

void init_i2c_pins(i2c_inst_t *i2c_port, uint sda_pin, uint scl_pin);

// --- Low level write to PCF8574 ---
void pcf8574_write(i2c_inst_t *i2c_port, uint8_t data);

// --- Pulse Enable Line ---
void lcd_pulse(i2c_inst_t *i2c_port, uint8_t data);

// --- Send 4-bit nibble ---
void lcd_write4(i2c_inst_t *i2c_port, uint8_t nibble, uint8_t mode);

// --- Send full byte: high nibble, then low nibble ---
void lcd_send(i2c_inst_t *i2c_port, uint8_t value, uint8_t mode);

// --- Public wrapper helpers ---
void lcd_cmd(i2c_inst_t *i2c_port, uint8_t cmd);

void lcd_write_char(i2c_inst_t *i2c_port, char c);

// --- Clear display ---
void lcd_clear(i2c_inst_t *i2c_port);

// --- Initialization sequence ---
void lcd_init(i2c_inst_t *i2c_port);

void lcd_print(i2c_inst_t *i2c_port, const char *text);

void lcd_move_cursor_next_line(i2c_inst_t *i2c_port);

#endif