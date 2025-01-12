#include "Buttons.h"
#include "Accelerometer.h"
#include "Alarm.h"
#include "Optical_Sensor.h"
#include <avr/interrupt.h>
#include <avr/io.h> // For AVR register definitions

uint8_t flag_accelerated_time = 0; // 0: normal, 1: accelerated1, 2: accelerated2
uint8_t flag_optical_start = 0; // 0: not started, 1: started
uint8_t flag_time_format = 1; // 0: 12-hour, 1: 24-hour
uint8_t flag_alarm_dismissed = 1; // 0: not dismissed, 1: dismissed

// Initialize button inputs
// PC3 ON
void init_buttons(void)
{
    DDRD &= ~((1 << PD6) | (1 << PD7) | (1 << PD4));
    PORTD |= (1 << PD6) | (1 << PD7) | (1 << PD4);

    DDRC &= ~((1 << PC5));
    DDRC |= ((1 << PC3));
    PORTC |= (1 << PC5) | (1 << PC3);

    DDRC |= (1 << PC2); // Set PC2 as output
    PORTC &= ~(1 << PC2); // Initially turn off PC2 (LED or whatever is connected)
    DDRD |= (1 << PD5); // Set PD3 as output for the buzzer
    PORTD &= ~(1 << PD5); // Ensure buzzer is off initially

    // Enable external interrupt on INT1 (PD3)
    EICRA |= (1 << ISC10); // Trigger on any logic change on INT1
    EIMSK |= (1 << INT1); // Enable INT1 interrupt
    sei(); // Enable global interrupts
}

// ISR for INT1 (External Interrupt on PD3)
ISR(INT1_vect)
{
    // Check which button is pressed
    if (!(PIND & (1 << PD6))) {
        if (is_rotated == 1) {
            button_24hr_format();
        } else {
            button_accelerated_time();
        }
    } else if (!(PIND & (1 << PD7))) {
        if (is_rotated == 1) {
            button_dismiss_alarm();
        } else {
            button_optical_start();
        }
    } else if (!(PIND & (1 << PD4))) {
        if (is_rotated == 1) {
            button_optical_start();
        } else {
            button_dismiss_alarm();
        }
    } else if (!(PINC & (1 << PC5))) {
        if (is_rotated == 1) {
            button_accelerated_time();
        } else {
            button_24hr_format();
        }
    }
}

void button_accelerated_time(void)
{
    if (flag_accelerated_time == 0) {
        flag_accelerated_time = 1;
    } else if (flag_accelerated_time == 1) {
        flag_accelerated_time = 2;
    } else {
        flag_accelerated_time = 0;
    }
}

void button_optical_start(void)
{
    flag_optical_start = !flag_optical_start; // Set flag to indicate optical transfer started
    optical_display = 0;
}

void button_24hr_format(void)
{
    flag_time_format = !flag_time_format; // Toggle between 0 and 1
}

void button_dismiss_alarm(void)
{
    flag_alarm_dismissed = 1;
    alarm_number = 0; // Turn off alarm
    led_on = 0; // Turn off LED
    buzzer_on = 0; // Turn off buzzer
    PORTC &= ~(1 << PC2); // Turn off alarm LED
    PORTD &= ~(1 << PD5); // Turn off alarm buzzer
}

void button_check()
{
    if (!(PIND & (1 << PD6))) {
        if (is_rotated == 1) {
            button_24hr_format();
        } else {
            button_accelerated_time();
        }
    } else if (!(PIND & (1 << PD7))) {
        if (is_rotated == 1) {
            button_dismiss_alarm();
        } else {
            button_optical_start();
        }
    } else if (!(PIND & (1 << PD4))) {
        if (is_rotated == 1) {
            button_optical_start();
        } else {
            button_dismiss_alarm();
        }
    } else if (!(PINC & (1 << PC5))) {
        if (is_rotated == 1) {
            button_accelerated_time();
        } else {
            button_24hr_format();
        }
    }
}