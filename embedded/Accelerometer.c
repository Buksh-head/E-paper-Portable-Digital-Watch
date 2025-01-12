#include "Accelerometer.h"
#include "Display.h"
#include <avr/io.h>
#include <math.h> // For atan2 and fabs
#include <stdio.h>
#include <util/delay.h>

// Assuming ADC pins for X and Y axes of the ADXL335
#define ADXL335_Y_PIN 0

uint8_t is_rotated = 0;

void init_accelerometer(void)
{
    // Set the reference voltage to AVcc (5V)
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t read_adc(uint8_t channel)
{
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // Select the ADC channel
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC))
        ; // Wait for conversion to complete

    // Return the ADC result
    return ADC;
}

void read_accelerometer(int* y)
{
    *y = read_adc(ADXL335_Y_PIN); // Read Y-axis (PC0)
}
float calculate_angle(int axis_value, int reference_value)
{
    return atan2(axis_value, reference_value) * 180.0 / M_PI;
}

void check_orientation()
{
    int y;
    int z = 1023;

    read_accelerometer(&y);

    // Calculate tilt angles
    float angleY = calculate_angle(y, z);

    // Check if the tilt exceeds the threshold
    if (fabs(angleY) < THRESHOLD_ANGLE) {
        is_rotated = 1; // display is rotated
        _delay_ms(100);

        // USART_send_string("Display is rotated\n");
    } else {
        is_rotated = 0; // normal orientation

        _delay_ms(100);
        // USART_send_string("Display is not rotated\n");
    }
    // To display angles as integers if floating-point is unsupported in snprintf:
    // int intAngleX = (int)(angleX * 100);  // Multiply for two decimal precision
    // int intAngleY = (int)(angleY * 100);
    //
    // char buffer[32];
    // snprintf(buffer, sizeof(buffer), " Y: %d.%02d",
    // intAngleY / 100, abs(intAngleY % 100));
    // USART_send_string(buffer);
}

// 26.5 x
//  26.2 y 24,2
