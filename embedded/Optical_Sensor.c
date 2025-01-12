#include "Optical_Sensor.h"
#include "Buttons.h"
#include "Display.h"
#include "UART.h"
#include "main.h"
#include <stdio.h>
#include <util/delay.h>

#define PHOTO_PIN PC4 // Phototransistor connected to PC4

uint16_t lightValue = 0;
char bufferx[34];
uint8_t optical_count = 0;
uint8_t started = 1;
uint8_t finished = 0;
uint8_t optical_display = 0;

void ADC_init()
{
    // Set up ADC
    ADMUX = (1 << REFS0) | (1 << MUX2);

    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_read()
{
    ADMUX = (ADMUX & 0xF0) | (4 & 0x0F);
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC))
        ; // Wait until conversion is complete
    return ADC;
}

void process_light_sensor()
{
    if (!optical_display) {
        display_optical();
        optical_display = 1;
    }
    lightValue = 0;
    for (uint8_t i = 0; i < 10; i++) {
        lightValue += ADC_read(); // Average light level from phototransistor
    }

    lightValue /= 10;

    if (started) {
        // Check if the optical_count has reached 33 (33 bits received)
        if (optical_count == 31) {
            finished = 1; // Signal that the message is complete
            started = 0;
            optical_count = 0;
        } else if (lightValue <= 500) { // Black (0) condition
            if (optical_count < 33) {
                bufferx[optical_count] = '0';
                optical_count++; // Increment only when a bit is added
            }
        } else if (lightValue > 940 && lightValue < 980) { // White (1) condition
            if (optical_count < 33) {
                bufferx[optical_count] = '1';
                optical_count++; // Increment only when a bit is added
            }
        } else {
            bufferx[0] = '\0';
            started = 0;
            optical_count = 0;
        }
    }

    if (started == 0 && lightValue >= 620 && lightValue < 730 && finished == 0) {
        started = 1; // Start reading if light value is in the start range
        optical_count = 0; // Reset optical_count when starting
    }

    if (finished == 1) {
        flag_optical_start = 0;
        // Null-terminate bufferx and convert the time
        bufferx[33] = '\0';
        convert_and_send_time(bufferx); // Convert binary to time and send via UART
        clear_icon();
        optical_display = 0;
        // Reset for the next reading
        optical_count = 0;
        started = 0;
        finished = 0;
        bufferx[0] = '\0';
    } else {
        // Debugging: print the light value and optical_count to the serial terminal
        // char temp_bufferx[16];
        // snprintf(temp_bufferx, sizeof(temp_bufferx), "Light: %d %d %d\n",
        // lightValue, optical_count, started); USART_send_string(temp_bufferx);
        _delay_ms(750);
    }
}

int binary_to_int(const char* binary_str)
{
    return (int)strtol(binary_str, NULL, 2);
}

void convert_and_send_time(char* binary_data)
{
    cli();
    // Extract different components from binary data
    char hour_str[6] = { 0 }; // 5 bits for hour
    char minute_str[7] = { 0 }; // 6 bits for minute
    char day_str[6] = { 0 }; // 5 bits for day
    char month_str[5] = { 0 }; // 4 bits for month
    char year_str[13] = { 0 }; // 12 bits for year

    strncpy(hour_str, binary_data, 5); // First 5 bits for hour
    strncpy(minute_str, binary_data + 5, 6); // Next 6 bits for minute
    strncpy(day_str, binary_data + 11, 5); // Next 5 bits for day
    strncpy(month_str, binary_data + 16, 4); // Next 4 bits for month
    strncpy(year_str, binary_data + 20, 12); // Last 12 bits for year

    // Convert binary strings to integers
    int hour = binary_to_int(hour_str);
    int minute = binary_to_int(minute_str);
    int day = binary_to_int(day_str);
    int month = binary_to_int(month_str);
    int year = binary_to_int(year_str) * 2;

    // Format the time and date as a string: "HH:MM DD/MM/YYYY"
    snprintf(datetime, sizeof(datetime), "%02d:%02d %02d/%02d/%04d\n", hour,
        minute, day, month, year);

    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_mday = day;
    timeinfo.tm_mon = month - 1; // Months are 0-11 in struct tm
    timeinfo.tm_year = year - 1900; // Years since 1900

    timeKeeping = mktime(&timeinfo);
    sei();

    // Send the formatted time string via UART
    USART_send_string(datetime);
    USART_flush();
}