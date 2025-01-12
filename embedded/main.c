#include <avr/io.h>
#include <stdio.h>
#include <time.h>
#include <util/delay.h>

#include "Accelerometer.h"
#include "Alarm.h"
#include "Buttons.h"
#include "Display.h"
#include "Optical_Sensor.h"
#include "Sleep.h"
#include "UART.h"
#include "main.h"

#define F_CPU 16000000UL
#define BAUD 9600 // 115200
#define BAUD_PRESCALE (((F_CPU / (BAUD * 16UL))) - 1)

void init_buttons(void);
void init_accelerometer(void);
void ADC_init(void);
void USART_init(void);
void process_light_sensor(void);
void USART_send_string(const char* str);
void SPI_Init(void);
void init_display(void);
void start_display(void);
void sleep_init(void);
void send_configuration_data_to_gui();
void main_display(void);
void play_alarm(void);
void check_alarms(void);

struct tm timeinfo;
time_t timeKeeping = 0;
uint8_t display_alarm = 0;
uint8_t local_rotate = 0;

// char buffer[64];

const char day_week[7][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

void init_time(void)
{
    // 25th October 2024, 00:00 (and a Friday)
    timeinfo.tm_sec = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_hour = 0;
    timeinfo.tm_mday = 25;
    timeinfo.tm_mon = 9; // October (0-based)
    timeinfo.tm_year = 124; // 2024 (years since 1900)
    timeinfo.tm_wday = 5; // Friday (5 = Friday)
    timeinfo.tm_isdst = -1; // Daylight saving time flag (-1 means "let mktime handle it")

    timeKeeping = mktime(&timeinfo);
}

int setup_timer(void)
{
    // Set up timer
    TCCR1B |= (1 << WGM12); // Configure timer 1 for CTC mode
    TIMSK1 |= (1 << OCIE1A); // Enable CTC interrupt
    OCR1A = 15624; // Set CTC compare value to 31250 7812
    TCCR1B |= (1 << CS12) | (1 << CS10); // Start timer at Fcpu/1024
    sei(); // Enable global interrupts
    return 0;
}

ISR(TIMER1_COMPA_vect)
{
    if (flag_accelerated_time == 1) {
        timeinfo.tm_sec += 15;
    } else if (flag_accelerated_time == 2) {
        timeinfo.tm_hour += 1;
    } else if (flag_accelerated_time == 0) {
        timeinfo.tm_sec += 1;
    }

    timeKeeping = mktime(&timeinfo);

    // Format the time into a buffer (HH:MM DD/MM/YYYY Day)
    // char buffer[32];
    // snprintf(buffer, sizeof(buffer), "%02d:%02d %02d/%02d/%04d %s\n",
    // timeinfo.tm_hour, timeinfo.tm_min,
    // timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
    // day_week[timeinfo.tm_wday]);
    //
    // USART_send_string(buffer);
}

int main(void)
{
    USART_init(); // Initialize USART
    SPI_Init(); // Initialize SPI
    setup_timer(); // Initialize timer
    init_time(); // Initialize time
    ADC_init(); // Initialize ADC for optical
    EPD_Reset(); // Reset IC
    EPD_init(); // Initialize display
    configure_interrupt(); // Initialize interrupt for sleep
    init_accelerometer(); // Initialize accelerometer
    init_buttons(); // Initialize buttons
    main_display();
    while (1) {

        if (flag_optical_start) {
            process_light_sensor();
            if (!(PIND & (1 << PD7))) {
                if (is_rotated == 0) {
                    button_optical_start();
                }
            } else if (!(PIND & (1 << PD4))) {
                if (is_rotated == 1) {
                    button_optical_start();
                }
            }
        } else if (!flag_alarm_dismissed) {
            play_alarm();
            if (!(PIND & (1 << PD7))) {
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
            }

        } else {
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
            check_alarms();
            display_time();
            if (a1set) {
                display_alarm1();
                a1set = 0;
            }
            if (a2set) {
                display_alarm2();
                a2set = 0;
            }
            check_orientation();
            if (local_rotate != is_rotated) {
                rotate_display();
                local_rotate = is_rotated;
            }

            _delay_ms(500);
        }
    }
}

// crystal oscillator PB6 and PB7