#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include <string.h>

#define I2C_PORT        i2c1
#define I2C_SDA         26
#define I2C_SCL         27

#define LCD_ADDR        0x27     // change to 0x3F if needed

// PCF8574 pin bits
#define LCD_BACKLIGHT   0x08     // backlight enable bit
#define LCD_ENABLE      0x04     // EN bit
#define LCD_RS          0x01     // Register select (0=command, 1=data)

// Command flags
#define LCD_4BITMODE        0x28
#define LCD_DISPLAYON       0x0C
#define LCD_CLEARDISPLAY    0x01
#define LCD_ENTRYMODE       0x06

#define POT_ADC_PIN 28

#define MOT_FWD 14
#define MOT_REV 15
#define row1 17
#define row2 18
#define row3 19
#define row4 20
#define col1 21
#define col2 22
#define col3 16

bool c1;
bool c2;
bool c3;

char determineKey(bool c1, bool c2, bool c3){
    
    gpio_put(row1, true);
    sleep_ms(5);
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    if(c1) { gpio_put(row1, false); return '1'; }
    if(c2) { gpio_put(row1, false); return '2'; }
    if(c3) { gpio_put(row1, false); return '3'; }
    gpio_put(row1, false);
    
    gpio_put(row2, true);
    sleep_ms(5);
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    if(c1) { gpio_put(row2, false); return '4'; }
    if(c2) { gpio_put(row2, false); return '5'; }
    if(c3) { gpio_put(row2, false); return '6'; }
    gpio_put(row2, false);
    
    gpio_put(row3, true);
    sleep_ms(5);
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    if(c1) { gpio_put(row3, false); return '7'; }
    if(c2) { gpio_put(row3, false); return '8'; }
    if(c3) { gpio_put(row3, false); return '9'; }
    gpio_put(row3, false);
    
    gpio_put(row4, true);
    sleep_ms(5);
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    if(c1) { gpio_put(row4, false); return '*'; }
    if(c2) { gpio_put(row4, false); return '0'; }
    if(c3) { gpio_put(row4, false); return '#'; }
    gpio_put(row4, false);
    
    return 'N';
}

void motors_stop(void) {
    gpio_put(MOT_FWD, 0);
    gpio_put(MOT_REV, 0);
}

void motors_forward(void) {
    gpio_put(MOT_FWD, 1);
    gpio_put(MOT_REV, 0);
}

void motors_backward(void) {
    gpio_put(MOT_FWD, 0);
    gpio_put(MOT_REV, 1);
}

// --- Low level write to PCF8574 ---
static void pcf8574_write(uint8_t data) {
    i2c_write_blocking(I2C_PORT, LCD_ADDR, &data, 1, false);
}

// --- Pulse Enable Line ---
static void lcd_pulse(uint8_t data) {
    pcf8574_write(data | LCD_ENABLE);
    sleep_us(500);
    pcf8574_write(data & ~LCD_ENABLE);
    sleep_us(500);
}

// --- Send 4-bit nibble ---
static void lcd_write4(uint8_t nibble, uint8_t mode) {
    uint8_t data = (nibble & 0xF0) | LCD_BACKLIGHT | mode;
    pcf8574_write(data);
    lcd_pulse(data);
}

// --- Send full byte: high nibble, then low nibble ---
static void lcd_send(uint8_t value, uint8_t mode) {
    lcd_write4(value & 0xF0, mode);
    lcd_write4((value << 4) & 0xF0, mode);
}

// --- Public wrapper helpers ---
static void lcd_cmd(uint8_t cmd) {
    lcd_send(cmd, 0x00);
    sleep_ms(2);
}

static void lcd_write_char(char c) {
    lcd_send(c, LCD_RS);
}

// --- Clear display ---
static void lcd_clear() {
    lcd_cmd(LCD_CLEARDISPLAY);
    sleep_ms(2);
}

// --- Initialization sequence ---
static void lcd_init() {
    sleep_ms(50);

    lcd_write4(0x30, 0); sleep_ms(5);
    lcd_write4(0x30, 0); sleep_us(200);
    lcd_write4(0x30, 0);
    lcd_write4(0x20, 0);   // switch to 4-bit mode

    lcd_cmd(LCD_4BITMODE);
    lcd_cmd(LCD_DISPLAYON);
    lcd_cmd(LCD_ENTRYMODE);
    lcd_clear();
}

void lcd_print(const char *text) {
    while (*text) {
        lcd_write_char(*text++);
    }
}

int main() {
    stdio_init_all();

    // Init I2C
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // LCD
    lcd_init();

    // Print Hello World
    lcd_print("Hello World!");
    lcd_cmd(0xC0);
    lcd_print("Line 2");

    adc_init();
    adc_gpio_init(POT_ADC_PIN);
    adc_select_input(2);

    gpio_init(row4);
    gpio_init(row3);
    gpio_init(row2);
    gpio_init(row1);
    gpio_init(col3);
    gpio_init(col2);
    gpio_init(col1);
    gpio_set_dir(row1, GPIO_OUT);
    gpio_set_dir(row2, GPIO_OUT);
    gpio_set_dir(row3, GPIO_OUT);
    gpio_set_dir(row4, GPIO_OUT);
    gpio_set_dir(col1, GPIO_IN);
    gpio_set_dir(col2, GPIO_IN);
    gpio_set_dir(col3, GPIO_IN);

    gpio_init(MOT_FWD);
    gpio_init(MOT_REV);

    gpio_set_dir(MOT_FWD, GPIO_OUT);
    gpio_set_dir(MOT_REV, GPIO_OUT);

    motors_stop();

    while (true) {

        char pressed_key = determineKey(c1, c2, c3);
        if(pressed_key != 'N'){
            printf("you pressed: %c\n", pressed_key);
        }

        uint16_t raw = adc_read();
        float norm = raw / 4095.0f;

        if (norm < 0.45f) {
            motors_backward();
        } else if (norm > 0.65f) {
            motors_forward();
        } else {
            motors_stop();
        }

        sleep_ms(50);
    }

    return 0;
}