#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"

#include <string.h>
#include "motors_driver.h"
#include "buzzer_driver.h"
#include "i2c_lcd_driver.h"
#include "keypad_driver.h"

#define I2C_PORT        i2c1
#define I2C_SDA         26
#define I2C_SCL         27

#define LCD_ADDR        0x27

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

#define buzzer_pin 0

int main() {
    stdio_init_all();

    init_motor_pins(POT_ADC_PIN, 2, MOT_FWD, MOT_REV);

    init_i2c_pins(I2C_PORT, I2C_SDA, I2C_SCL);
    lcd_init(I2C_PORT);

    init_buzzer_pin(buzzer_pin);

    init_keypad_pins(row1, row2, row3, row4, col1, col2, col3);

    init_motor_pins(POT_ADC_PIN, 2, MOT_FWD, MOT_REV);
    
        sleep_ms(10000);

        printf("Buzzer ON\n");
        switch_buzzer_on(buzzer_pin);
        sleep_ms(5000);

        printf("Buzzer OFF\n");
        switch_buzzer_off(buzzer_pin);

        for(int i = 0; i < 2; i++){
            lcd_clear(I2C_PORT);
            lcd_print(I2C_PORT, "Drivers");
            lcd_move_cursor_next_line(I2C_PORT);
            lcd_print(I2C_PORT, "Go BRRR");

            sleep_ms(5000);

            lcd_clear(I2C_PORT);
            lcd_print(I2C_PORT, "Yipeeee");
            lcd_move_cursor_next_line(I2C_PORT);
            lcd_print(I2C_PORT, "Successs");

            sleep_ms(5000);
        }

    while (true) {

            char key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
            if(key != 'N') printf("You pressed: %c \n", key);

            control_motors_with_potentiometer(POT_ADC_PIN, MOT_FWD, MOT_REV);
        
        }
    return 0;
}
