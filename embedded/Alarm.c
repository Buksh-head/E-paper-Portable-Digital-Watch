#include "Alarm.h"
#include "Buttons.h"
#include "Display.h"
#include "UART.h"
#include "main.h"
#include <avr/eeprom.h>
#include <avr/io.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <util/delay.h>

struct tm alarm1_time = { 0 };
struct tm alarm2_time = { 0 };

uint8_t led_on = 0;
uint8_t buzzer_on = 0;
uint8_t alarm_number = 0;

int interval = 15;
time_t alarm_time1 = 0;
time_t alarm_time2 = 0;

uint8_t a1set = 0;
uint8_t a2set = 0;
// Buzzer PD5
// LED PC2 for alarms yellow
// When the notification LED is active, it must blink at a rate of approximately
// 200ms on, 200ms off (+/- 100ms). Similarly, when the audible indicator is
// active, it must generate a tone at a rate of 500ms on, 500ms off (+/- 100ms).

void load_alarms()
{
    if (strcmp(alarm1, "No") != 0) {

        int hour1, minute1, day1, month1, year1;

        sscanf(alarm1, "%d:%d %d/%d/%d", &hour1, &minute1, &day1, &month1, &year1);

        // Set the struct tm values
        alarm1_time.tm_hour = hour1;
        alarm1_time.tm_min = minute1;
        alarm1_time.tm_mday = day1;
        alarm1_time.tm_mon = month1;
        alarm1_time.tm_year = year1 - 1900; // Year since 1900

        // Convert to time_t and check if valid
        alarm_time1 = mktime(&alarm1_time);

        a1set = 1;
    }

    if (strcmp(alarm2, "No") != 0) {
        int hour2, minute2, day2, month2, year2;

        sscanf(alarm2, "%d:%d %d/%d/%d", &hour2, &minute2, &day2, &month2, &year2);

        // Set the struct tm values
        alarm2_time.tm_hour = hour2;
        alarm2_time.tm_min = minute2;
        alarm2_time.tm_mday = day2;
        alarm2_time.tm_mon = month2;
        alarm2_time.tm_year = year2 - 1900;

        alarm_time2 = mktime(&alarm2_time);
        a2set = 1;
    }
}

void check_alarms()
{
    time_t current_time = mktime(&timeinfo);

    int time_diff_1 = difftime(current_time, alarm_time1);
    int time_diff_2 = difftime(current_time, alarm_time2);

    if (flag_accelerated_time == 2) {
        interval = 3600;
    } else {
        interval = 30;
    }

    if (strcmp(alarm1, "No") != 0 && time_diff_1 >= 0 && time_diff_1 <= interval) {
        flag_alarm_dismissed = 0;
        alarm_number = 1;

        if (strcmp(alarm1_led, "1") == 0) {
            led_on = 1;
        }
        if (strcmp(alarm1_buzzer, "1") == 0) {
            buzzer_on = 1;
        }

        if (strcmp(alarm1_msg, "0") != 0) {
            // Handle alarm message display
        }

        // Clear the alarm1 since it has triggered
        memset(&alarm1_time, 0, sizeof(alarm1_time)); // Clear alarm1_time
        strcpy(alarm1, "No");
        eeprom_update_block(alarm1, (void*)0x08, 17);
    }

    else if (strcmp(alarm2, "No") != 0 && time_diff_2 >= 0 && time_diff_2 <= interval) {
        flag_alarm_dismissed = 0;
        alarm_number = 2;

        if (strcmp(alarm2_led, "1") == 0) {
            led_on = 1;
        }
        if (strcmp(alarm2_buzzer, "1") == 0) {
            buzzer_on = 1;
        }

        if (strcmp(alarm2_msg, "0") != 0) {
            // Handle alarm message display
        }

        // Clear the alarm2 since it has triggered
        memset(&alarm2_time, 0, sizeof(alarm2_time)); // Clear alarm2_time
        strcpy(alarm2, "No");
        eeprom_update_block(alarm2, (void*)0x19, 17);
    }
}

void play_alarm()
{
    if (led_on) {
        PORTC ^= (1 << PC2); // Toggle LED on PC2
    }
    if (buzzer_on) {
        PORTD ^= (1 << PD5);
    }
    _delay_ms(220);
    if (led_on) {
        PORTC ^= (1 << PC2); // Toggle LED off (if it was on)
    }
    _delay_ms(220);
}