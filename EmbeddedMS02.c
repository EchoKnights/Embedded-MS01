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

#include <math.h>

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

float YOLO_Read(target){
    return rand();
}

void controller(){

    bool button_1 = false;
    bool tens_entered = false;
    bool units_entered = false;
    bool wrong = false;
    int tens;
    int units;
    char key;
    bool threshold = false;

    while(!button_1){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "ENTER 1-DIGIT");
        lcd_move_cursor_next_line(I2C_PORT);
        lcd_print(I2C_PORT, "SHAPE INDEX");
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            return;
        }
        if(key == '3'){
            button_1 = true;
        }else{
            button_1 = true;
            wrong = true;
        }
    }

    while(wrong){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "ERROR: RETRY");
        lcd_move_cursor_next_line(I2C_PORT);
        lcd_print(I2C_PORT, "SHAPE INDEX");
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            return;
        }
        if(key == '3'){
           wrong = false;
        }
    }

    lcd_clear(I2C_PORT);
    lcd_print(I2C_PORT, "SHAPE INDEX 3");
    lcd_move_cursor_next_line(I2C_PORT);
    lcd_print(I2C_PORT, "LOCKED");

    sleep_ms(4000);

    while(!tens_entered){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "ENTER 2-DIGIT");
        lcd_move_cursor_next_line(I2C_PORT);
        lcd_print(I2C_PORT, "DISTANCE CLUE CM");
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            return;
        }
        if(!key == 'N' && !key == '#'){
            tens = ((int) key) - 49;
            tens_entered = true;
        }
    }

    while(!units_entered){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "ENTER 2-DIGIT");
        lcd_move_cursor_next_line(I2C_PORT);
        lcd_print(I2C_PORT, "DISTANCE CLUE CM");
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            return;
        }
        if(!key == 'N' && !key == '#'){
            units = ((int) key) - 49;
            units_entered = true;
        }
    }

    int value = tens * 10 + units;

    char* target_confirm;
    //concatenate
    //target_confirm = TARGET: {value}CM
    
    lcd_clear(I2C_PORT);
    lcd_print(I2C_PORT, target_confirm);

    sleep_ms(4000);
    
    lcd_clear(I2C_PORT);
    lcd_print(I2C_PORT, "PLACE SHAPE ON");
    lcd_move_cursor_next_line(I2C_PORT);
    lcd_print(I2C_PORT, "ROVER PLATFORM");

    char* distance_reading;

    float result;

    while(!threshold){

        control_motors_with_potentiometer(POT_ADC_PIN, MOT_FWD, MOT_REV);

        result = YOLO_Read();

        //concatenate
        //distance_reading = DISTANCE {YOLO_reading} CM
        
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, distance_reading);
        lcd_move_cursor_next_line(I2C_PORT);
        lcd_print(I2C_PORT, target_confirm);
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            return;
        }
        if(result > 100){
            threshold = true;
        }
    }

    lcd_clear(I2C_PORT);
    lcd_print(I2C_PORT, "CALIBRATION LOCK");
    lcd_move_cursor_next_line(I2C_PORT);
    lcd_print(I2C_PORT, "ACHIEVED");

    sleep_ms(4000);
    
    while(true){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "ACCESS GRANTED");
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            return;
        }
    }

}

int main() {
    stdio_init_all();

    init_motor_pins(POT_ADC_PIN, 2, MOT_FWD, MOT_REV);

    init_i2c_pins(I2C_PORT, I2C_SDA, I2C_SCL);

    lcd_init(I2C_PORT);

    init_buzzer_pin(buzzer_pin);

    init_keypad_pins(row1, row2, row3, row4, col1, col2, col3);

    switch_buzzer_on(buzzer_pin);

    switch_buzzer_off(buzzer_pin);

    while(true){
        controller();
    }

    return 0;

}
