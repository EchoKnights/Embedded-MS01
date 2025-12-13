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
#include "webcam_driver.h"

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

void controller(){

    sleep_ms(1000);

    bool button_1 = false;
    bool tens_entered = false;
    bool units_entered = false;
    bool wrong = false;
    int tens;
    int units;
    char key;
    bool threshold = false;
    float acceptable_drop = 500;

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
        }else if(key != 'N'){
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
        if(!(key == 'N') && !(key == '#')){
            tens = ((int) key) - 48;
            tens_entered = true;
            printf("Tens: %d\n", tens);
        }
    }

    sleep_ms(1000);

    while(!units_entered){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "ENTER 2-DIGIT");
        lcd_move_cursor_next_line(I2C_PORT);
        lcd_print(I2C_PORT, "DISTANCE CLUE CM");
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            return;
        }
        if(!(key == 'N') && !(key == '#')){
            units = ((int) key) - 48;
            units_entered = true;
            printf("Units: %d\n", units);
        }
    }

    int value = (float) tens * 10 + units;
    printf("Value: %d\n", value);

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "TARGET: %dCM", value);

    const char* target_confirm = buffer;

    lcd_clear(I2C_PORT);
    lcd_print(I2C_PORT, target_confirm);

    sleep_ms(4000);
    
    lcd_clear(I2C_PORT);
    lcd_print(I2C_PORT, "PLACE SHAPE ON");
    lcd_move_cursor_next_line(I2C_PORT);
    lcd_print(I2C_PORT, "ROVER PLATFORM");

    char* distance_reading;
	float reading;
	float last_reading = 0.0f;

	uint64_t now;
	uint64_t last_seen = 0;
	uint64_t in_range_since = 0;

	while (!threshold) {

		now = to_ms_since_boot(get_absolute_time());

		control_motors_with_potentiometer(POT_ADC_PIN, MOT_FWD, MOT_REV);
		key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            switch_buzzer_off(buzzer_pin);
            return;
        }

		reading = poll_usb_distance();

		if (reading >= 0.0f) {  // ✅ valid distance
			last_seen = now;
			last_reading = reading;

			printf("DISTANCE: %.1f cm\n", reading);
		}

		// Heartbeat timeout (USB silence)
		if (now - last_seen > acceptable_drop) {
			in_range_since = 0;
			continue;
		}

		// Range check
		if (last_reading < value - 2.0f || last_reading > value + 2.0f) {
			in_range_since = 0;
			continue;
		}

		// Start timing if newly in range
		if (in_range_since == 0) {
			in_range_since = now;
		}

		char buffer[50];
		snprintf(buffer, sizeof(buffer), "DISTANCE: %.1fCM", last_reading);
		const char* distance_reading = buffer;
		lcd_clear(I2C_PORT);
		lcd_print(I2C_PORT, distance_reading);
		lcd_move_cursor_next_line(I2C_PORT);
		lcd_print(I2C_PORT, target_confirm);

		// Hold condition
		if (now - in_range_since >= 5000) {
			motors_stop(MOT_FWD, MOT_REV);
			threshold = true;
			printf("THRESHOLD REACHED\n");
			break;
		}

		sleep_ms(1);
	}

    lcd_clear(I2C_PORT);
    lcd_print(I2C_PORT, "CALIBRATION LOCK");
    lcd_move_cursor_next_line(I2C_PORT);
    lcd_print(I2C_PORT, "ACHIEVED");

    sleep_ms(4000);
    
    int twice = 0;
    while(twice < 2){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "ACCESS GRANTED");

        switch_buzzer_on(buzzer_pin);
        sleep_ms(1000);
        switch_buzzer_off(buzzer_pin);
        sleep_ms(1000);
        switch_buzzer_on(buzzer_pin);
        sleep_ms(1000);
        switch_buzzer_off(buzzer_pin);

        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            switch_buzzer_off(buzzer_pin);
            return;
        }
        twice++;
    }

    while(true){
        lcd_clear(I2C_PORT);
        lcd_print(I2C_PORT, "CLUE 7541");
        key = determine_key_pressed(row1, row2, row3, row4, col1, col2, col3);
        if(key == '*'){
            switch_buzzer_off(buzzer_pin);
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

// pico_diag_echo.c
#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"

int test(void) {
	stdio_init_all();
#if defined(PICO_DEFAULT_LED_PIN)
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif

	printf("DIAG_ECHO: ready\r\n");

	uint32_t loops = 0;
	uint32_t time_ms = to_ms_since_boot(get_absolute_time());
	uint32_t last_report = time_ms;

	while (true) {
		bool got_any = false;
		int c;
		// drain all available bytes this tick
		while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT) {
			got_any = true;
			// echo back and also print to USB stdio as visible text
			putchar(c);
		}

		if (got_any) {
#if defined(PICO_DEFAULT_LED_PIN)
			gpio_put(PICO_DEFAULT_LED_PIN, 1);
			sleep_ms(60);
			gpio_put(PICO_DEFAULT_LED_PIN, 0);
#endif
			fflush(stdout);
		}

		// report status once per second
		loops++;
		time_ms = to_ms_since_boot(get_absolute_time());
		if (time_ms - last_report >= 1000) {
			printf("DIAG: loops=%u time=%u\n", loops, time_ms);
			fflush(stdout);
			last_report = time_ms;
			loops = 0;
		}

		sleep_ms(2);
	}
	return 0;
}