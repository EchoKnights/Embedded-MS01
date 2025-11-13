#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <string.h>

#define POT_ADC_PIN 28

#define MOT_FWD 14
#define MOT_REV 15
#define row1 17
#define row2 18
#define row3 19
#define row4 20
#define col1 21
#define col2 22
#define col3 26
// #define col4 27
// Pin mapping (change if you use different GPIOs)
#define LCD_RS 0
#define LCD_E  1
#define LCD_D4 2
#define LCD_D5 3
#define LCD_D6 4
#define LCD_D7 5

bool c1;
bool c2;
bool c3;
// bool c4;


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

char determineKey(bool c1, bool c2, bool c3){
    
    gpio_put(row1, true);
    sleep_ms(5);  // Debounce delay BEFORE reading
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    // c4 = gpio_get(col4);  // FIXED: was col3
    if(c1) { gpio_put(row1, false); return '1'; }
    if(c2) { gpio_put(row1, false); return '2'; }
    if(c3) { gpio_put(row1, false); return '3'; }
    // if(c4) { gpio_put(row1, false); return 'A'; }
    gpio_put(row1, false);
    
    gpio_put(row2, true);
    sleep_ms(5);
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    // c4 = gpio_get(col4);  // FIXED: was col3
    if(c1) { gpio_put(row2, false); return '4'; }
    if(c2) { gpio_put(row2, false); return '5'; }
    if(c3) { gpio_put(row2, false); return '6'; }
    // if(c4) { gpio_put(row2, false); return 'B'; }
    gpio_put(row2, false);
    
    gpio_put(row3, true);
    sleep_ms(5);
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    // c4 = gpio_get(col4);  // FIXED: was col3
    if(c1) { gpio_put(row3, false); return '7'; }
    if(c2) { gpio_put(row3, false); return '8'; }
    if(c3) { gpio_put(row3, false); return '9'; }
    // if(c4) { gpio_put(row3, false); return 'C'; }
    gpio_put(row3, false);
    
    gpio_put(row4, true);
    sleep_ms(5);
    c1 = gpio_get(col1);
    c2 = gpio_get(col2);
    c3 = gpio_get(col3);
    // c4 = gpio_get(col4);  // FIXED: was col3
    if(c1) { gpio_put(row4, false); return '*'; }
    if(c2) { gpio_put(row4, false); return '0'; }
    if(c3) { gpio_put(row4, false); return '#'; }
    // if(c4) { gpio_put(row4, false); return 'D'; }
    gpio_put(row4, false);
    
    return 'N';
}

int main()
{
    stdio_init_all();

    adc_init();
    adc_gpio_init(POT_ADC_PIN);
    adc_select_input(2);

    gpio_init(row4);
    gpio_init(row3);
    gpio_init(row2);
    gpio_init(row1);
    // gpio_init(col4);
    gpio_init(col3);
    gpio_init(col2);
    gpio_init(col2);
    gpio_set_dir(row1, GPIO_OUT);
    gpio_set_dir(row2, GPIO_OUT);
    gpio_set_dir(row3, GPIO_OUT);
    gpio_set_dir(row4, GPIO_OUT);
    gpio_set_dir(col1, GPIO_IN);
    gpio_set_dir(col2, GPIO_IN);
    gpio_set_dir(col3, GPIO_IN);
    // gpio_set_dir(col4, GPIO_IN);

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

        // Move cursor to home (optional)
        // lcd_command(0x02); // return home
        // sleep_ms(2);

        // // Print message
        // lcd_print("Hello world");

        // printf("norm=%.2f\n", norm);
        sleep_ms(50);
    }
}

// Timing helpers
// static inline void en_pulse(void) {
//     gpio_put(LCD_E, 1);
//     sleep_us(1);      // E high time (short)
//     gpio_put(LCD_E, 0); // falling edge latches data
//     sleep_us(40);     // wait for the command to be accepted (>= 37 us typical)
// }

// void lcd_write_nibble(uint8_t nibble) {
//     // nibble: lower 4 bits used (b3 b2 b1 b0)
//     gpio_put(LCD_D4, (nibble >> 0) & 1);
//     gpio_put(LCD_D5, (nibble >> 1) & 1);
//     gpio_put(LCD_D6, (nibble >> 2) & 1);
//     gpio_put(LCD_D7, (nibble >> 3) & 1);
//     en_pulse();
// }

// void lcd_send_byte(uint8_t byte, bool is_data) {
//     gpio_put(LCD_RS, is_data ? 1 : 0); // RS=1 -> data, RS=0 -> command
//     // RW tied to GND (write only)
//     // Send high nibble first
//     lcd_write_nibble(byte >> 4);
//     // Then low nibble
//     lcd_write_nibble(byte & 0x0F);
//     // short settle
//     sleep_us(40);
// }

// void lcd_command(uint8_t cmd) { lcd_send_byte(cmd, false); }
// void lcd_write_char(char c) { lcd_send_byte((uint8_t)c, true); }

// void lcd_init(void) {
//     // init GPIOs
//     gpio_init(LCD_RS); gpio_set_dir(LCD_RS, GPIO_OUT);
//     gpio_init(LCD_E);  gpio_set_dir(LCD_E,  GPIO_OUT);
//     gpio_init(LCD_D4); gpio_set_dir(LCD_D4, GPIO_OUT);
//     gpio_init(LCD_D5); gpio_set_dir(LCD_D5, GPIO_OUT);
//     gpio_init(LCD_D6); gpio_set_dir(LCD_D6, GPIO_OUT);
//     gpio_init(LCD_D7); gpio_set_dir(LCD_D7, GPIO_OUT);

//     // ensure pins low
//     gpio_put(LCD_RS, 0);
//     gpio_put(LCD_E, 0);
//     gpio_put(LCD_D4, 0);
//     gpio_put(LCD_D5, 0);
//     gpio_put(LCD_D6, 0);
//     gpio_put(LCD_D7, 0);

//     sleep_ms(50); // wait >40ms after Vcc rises

//     // Initialization sequence for 4-bit mode (as per HD44780 datasheet)
//     // We send the special nibbles to put the LCD into 4-bit mode:
//     gpio_put(LCD_RS, 0);
//     // send 0x3 three times (only high nibble) with delays:
//     lcd_write_nibble(0x03);
//     sleep_ms(5);     // wait >4.1 ms
//     lcd_write_nibble(0x03);
//     sleep_us(150);   // wait >100 us
//     lcd_write_nibble(0x03);
//     // now send 0x2 to switch to 4-bit mode
//     lcd_write_nibble(0x02);

//     // Now the LCD is in 4-bit mode. Send full commands (as bytes):
//     lcd_command(0x28); // Function set: 4-bit, 2 lines (or 4-lines addressed as 2-line mode), 5x8 dots
//     lcd_command(0x08); // Display OFF (clear before setup)
//     lcd_command(0x01); // Clear display
//     sleep_ms(2);       // clear needs >1.53 ms
//     lcd_command(0x06); // Entry mode set: increment, no shift
//     lcd_command(0x0C); // Display ON, cursor OFF, blink OFF
// }

// void lcd_print(const char *s) {
//     while (*s) {
//         lcd_write_char(*s++);
//     }
// }

// int main() {
//     stdio_init_all();
//     while (1) tight_loop_contents();
// }