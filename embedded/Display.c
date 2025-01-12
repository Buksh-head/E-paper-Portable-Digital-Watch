#include "Display.h"
#include "Accelerometer.h"
#include "Alarm.h"
#include "Bitmap.h"
#include "Buttons.h"
#include "UART.h"
#include "main.h"
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <util/delay.h>

//***************************************************************//
// LUT_Flag Calling Waveform Markers
// Border_Flag  Border Markers,Only the first screen after initialization will
// refresh the border£¬Do not brush at other times
//***************************************************************//
unsigned long LUT_Flag = 0, Border_Flag = 0;

char* numbers[] = { zero, one, two, three, four, five, six, seven, eight, nine };
char* days[] = { sun, mon, tue, wed, thu, fri, sat };
char* small_numbers[] = { smallzero, smallone, smalltwo, smallthree,
    smallfour, smallfive, smallsix, smallseven,
    smalleight, smallnine };
char* super_small_numbers[] = {
    superSmallZero, superSmallOne, superSmallTwo, superSmallThree,
    superSmallFour, superSmallFive, superSmallSix, superSmallSeven,
    superSmallEight, superSmallNine
};

uint8_t local_hour = 99;
uint8_t local_minute = 61;
uint8_t local_day = 32;
uint8_t local_date = 13;
uint8_t local_month = 121;
uint8_t hour = 25;
uint8_t minute = 61;
uint8_t day = 8;
uint8_t date = 32;
uint8_t month = 13;
uint8_t year = 121;
int local_year = 0;
uint8_t am_pm = 0;

// SPI Control functions
void SPI_Init()
{
    // Configure pins as outputs
    DDRB |= (1 << SDA) | (1 << SCK) | (1 << CS1); // Set SDA, SCK, CS1 as output on PORTB
    DDRB |= (1 << DC) | (1 << RST); // Set DC, RST as output on PORTD
    PORTB |= (1 << CS1);
    PORTB |= (1 << RST) | (1 << DC);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0) | (1 << SPR1);
}

// Control functions
void EPD_Reset(void)
{
    DDRB |= (1 << RST); // Set RST pin as output
    PORTB &= ~(1 << RST); // Reset low
    _delay_ms(200);
    PORTB |= (1 << RST); // Reset high
    _delay_ms(200);
}

void EPD_W21_WriteCMD(uint8_t command)
{
    PORTB &= ~(1 << CS1); // CS low to select the device
    PORTB &= ~(1 << DC); // DC low for command
    SPI_Transmit(command);
    PORTB |= (1 << CS1); // CS high to deselect the device
}

void EPD_W21_WriteDATA(uint8_t data)
{
    PORTB &= ~(1 << CS1); // CS low to select the device
    PORTB |= (1 << DC); // DC high for data
    SPI_Transmit(data);
    PORTB |= (1 << CS1); // CS high to deselect the device
}

void SPI_Transmit(uint8_t data)
{
    SPDR = data; // Start transmission
    while (!(SPSR & (1 << SPIF)))
        ; // Wait for transmission complete
}

//*******************EPD_init(void)***************************//
// Initialize settings. You must re-initialize settings after resetting.
//******************************************************//
void EPD_init(void)
{

    LUT_Flag = 0, Border_Flag = 0;

    EPD_W21_WriteCMD(0x00);
    EPD_W21_WriteDATA(0xFF); // You can change the scanning direction, FF -> F3

    EPD_W21_WriteCMD(0x01);
    EPD_W21_WriteDATA(0x03);
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(0x3F);
    EPD_W21_WriteDATA(0x3F);
    EPD_W21_WriteDATA(0x03);

    EPD_W21_WriteCMD(0x03);
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0x06);
    EPD_W21_WriteDATA(0x27);
    EPD_W21_WriteDATA(0x27);
    EPD_W21_WriteDATA(0x2F);

    EPD_W21_WriteCMD(0x30);
    EPD_W21_WriteDATA(0x09);

    EPD_W21_WriteCMD(0x60);
    EPD_W21_WriteDATA(0x22);

    EPD_W21_WriteCMD(0x82); // vcom
    EPD_W21_WriteDATA(0x00); //

    EPD_W21_WriteCMD(0xE3);
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0x41);
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0x61);
    EPD_W21_WriteDATA(0x80);
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(0xFA);

    EPD_W21_WriteCMD(0x65);
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0X50);
    EPD_W21_WriteDATA(0xB7);
}

//****************EPD_refresh(void)************************//
// Screen refresh command
//******************************************************//
void EPD_refresh(void)
{
    Border_Flag = 1;
    EPD_W21_WriteCMD(0x17);
    EPD_W21_WriteDATA(0xA5);
    lcd_chkstatus();
}

//***************EPD_sleep(void)************************//
// Enter sleep
// After exiting sleep, you need to re-initialize the settings. Use GC to
// refresh the first screen, and then use DU to refresh the screen.
//******************************************************//
void EPD_sleep(void)
{
    EPD_W21_WriteCMD(0X07); // deep sleep
    EPD_W21_WriteDATA(0xA5);
}

//***************lut_GC(void)************************//
// Call GC waveform
//***************************************************//
void lut_GC(void)
{
    unsigned int count;

    EPD_W21_WriteCMD(0x20); // vcom
    for (count = 0; count < 56; count++) {
        EPD_W21_WriteDATA(pgm_read_byte(&lut_R20_GC[count]));
    }

    EPD_W21_WriteCMD(0x21); // red not use
    for (count = 0; count < 56; count++) {
        EPD_W21_WriteDATA(pgm_read_byte(&lut_R21_GC[count]));
    }

    EPD_W21_WriteCMD(0x24); // bb b
    for (count = 0; count < 56; count++) {
        EPD_W21_WriteDATA(pgm_read_byte(&lut_R24_GC[count]));
    }

    if (LUT_Flag == 0) {

        EPD_W21_WriteCMD(0x22); // bw r
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R22_GC[count]));
        }

        EPD_W21_WriteCMD(0x23); // wb w
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R23_GC[count]));
        }
        LUT_Flag = 1;
    } else {

        EPD_W21_WriteCMD(0x22); // bw r
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R23_GC[count]));
        }

        EPD_W21_WriteCMD(0x23); // wb w
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R22_GC[count]));
        }
        LUT_Flag = 0;
    }
}

//***************lut_DU(void)************************//
// Call DU waveform
//***************************************************//
void lut_DU(void)
{
    unsigned int count;

    EPD_W21_WriteCMD(0x20); // vcom
    for (count = 0; count < 56; count++) {
        EPD_W21_WriteDATA(pgm_read_byte(&lut_R20_DU[count]));
    }

    EPD_W21_WriteCMD(0x21); // red not use
    for (count = 0; count < 56; count++) {
        EPD_W21_WriteDATA(pgm_read_byte(&lut_R21_DU[count]));
    }

    EPD_W21_WriteCMD(0x24); // bb b
    for (count = 0; count < 56; count++) {
        EPD_W21_WriteDATA(pgm_read_byte(&lut_R24_DU[count]));
    }

    if (LUT_Flag == 0) {

        EPD_W21_WriteCMD(0x22); // bw r
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R22_DU[count]));
        }

        EPD_W21_WriteCMD(0x23); // wb w
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R23_DU[count]));
        }
        LUT_Flag = 1;
    } else {

        EPD_W21_WriteCMD(0x22); // bw r
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R23_DU[count]));
        }

        EPD_W21_WriteCMD(0x23); // wb w
        for (count = 0; count < 56; count++) {
            EPD_W21_WriteDATA(pgm_read_byte(&lut_R22_DU[count]));
        }
        LUT_Flag = 0;
    }
}

//***************PIC_display_DU**********************//
// Call DU waveform to refresh the screen
//***************************************************//
void PIC_display_DU(const unsigned char* picData)
{
    unsigned int i;

    EPD_W21_WriteCMD(0X50); //
    EPD_W21_WriteDATA(0xD7); // Border

    EPD_W21_WriteCMD(0x13); // Transfer new data
    for (i = 0; i < (Gate_Pixel * Source_Pixel / 8); i++) {
        EPD_W21_WriteDATA(pgm_read_byte(&picData[i]));
    }

    lut_DU();
    EPD_refresh();
}

//***************Clean display_GU**********************//
// Call GC waveform to refresh the screen
//***************************************************//
void Clean_display_GC(const unsigned char Dat)
{
    unsigned int i;

    //***********************************************************************//
    // Border_Flag   Only the first screen after initialization refreshes the
    // border, and no refreshes at other times

    if (Border_Flag == 1) {
        EPD_W21_WriteCMD(0X50); //
        EPD_W21_WriteDATA(0xD7); // no refreshes Border
    }
    //***********************************************************************//

    EPD_W21_WriteCMD(0x13);
    for (i = 0; i < (Gate_Pixel * Source_Pixel / 8); i++) {
        EPD_W21_WriteDATA(Dat);
    }

    lut_GC();
    EPD_refresh();
}

//***************PIC_display_GU**********************//
// Call GC waveform to refresh the screen
//***************************************************//
void PIC_display_GC(const unsigned char* picData)
{
    unsigned int i;

    //***********************************************************************//
    ////Border_Flag   Only the first screen after initialization refreshes the
    /// border, and no refreshes at other times

    if (Border_Flag == 1) {
        EPD_W21_WriteCMD(0X50); //
        EPD_W21_WriteDATA(0xD7); // no refreshes Border
    }
    //***********************************************************************//

    EPD_W21_WriteCMD(0x13);
    for (i = 0; i < (Gate_Pixel * Source_Pixel / 8); i++) {
        // EPD_W21_WriteDATA(pgm_read_byte(&picData[i]));
        EPD_W21_WriteDATA(0xFF);
    }

    lut_GC();
    EPD_refresh();
}

//***************lcd_chkstatus***********************//
// READ BUSY Pin level = L means the screen driver chip is busy and cannot send
// data or instructions. Pin level = H means the screen driver chip is in idle
// state and can send data and instructions
//***************************************************//
void lcd_chkstatus(void)
{
    // while (PIND & (1 << PD2)) {
    while (!(PIND & (1 << PD2))) {
        // Wait until BUSY pin goes low, indicating the display is ready
    }
}

void EPD_Dis_Part(unsigned int x_start, unsigned int y_start,
    const unsigned char* datas, unsigned int width,
    unsigned int height)
{

    x_start &= 0xF8;
    uint32_t x_end = (x_start + width - 1) | 0x07;

    uint32_t y_end = y_start + height - 1;

    EPD_W21_WriteCMD(0x91); // Make display enter partial mode
    EPD_W21_WriteCMD(0x90); // Resolution setting

    EPD_W21_WriteDATA(x_start); // Initial start x position
    EPD_W21_WriteDATA(x_end); // End x position
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(y_start); // Initial y position
    EPD_W21_WriteDATA(0x00);
    EPD_W21_WriteDATA(y_end); // End y position
    EPD_W21_WriteDATA(0x00);

    EPD_W21_WriteCMD(0x13); // Transfer new data

    for (int i = 0; i < (width * height) / 8; i++) {
        EPD_W21_WriteDATA(pgm_read_byte(&datas[i]));
        // EPD_W21_WriteDATA(0xFF);
    }

    EPD_W21_WriteCMD(0x92);
    lut_GC();
    // EPD_refresh();
}

void display_optical()
{
    EPD_Dis_Part(0, 230, lightbulb, 36, 20);
    EPD_refresh();
}

void clear_icon()
{
    EPD_Dis_Part(0, 230, clear_icon, 36, 20);
    EPD_refresh();
}

void display_time(void)
{
    hour = timeinfo.tm_hour;
    minute = timeinfo.tm_min;
    day = timeinfo.tm_wday;
    date = timeinfo.tm_mday;
    month = timeinfo.tm_mon + 1;
    year = timeinfo.tm_year;

    if (flag_time_format) {
        if (am_pm) {
            EPD_Dis_Part(0, 190, superSmallClear, 8, 6);
            EPD_Dis_Part(0, 196, superSmallClear, 8, 6);
            am_pm = 0;
        }
    } else {
        am_pm = 1;
        if (timeinfo.tm_hour > 12) {
            hour -= 12;
            EPD_Dis_Part(0, 190, superSmallP, 8, 6);
            EPD_Dis_Part(0, 196, superSmallM, 8, 6);
        } else {
            EPD_Dis_Part(0, 190, superSmallA, 8, 6);
            EPD_Dis_Part(0, 196, superSmallM, 8, 6);
        }
    }

    if (local_hour != hour) {
        local_hour = hour;
        EPD_Dis_Part(0, 0, numbers[hour / 10], 73, 38);
        EPD_Dis_Part(0, 38, numbers[hour % 10], 73, 38);
    }

    if (local_minute != minute) {
        local_minute = minute;
        EPD_Dis_Part(0, 114, numbers[minute / 10], 73, 38);
        EPD_Dis_Part(0, 152, numbers[minute % 10], 73, 38);
    }

    if (local_day != day) {
        local_day = day;
        EPD_Dis_Part(100, 100, days[day], 28, 90);
    }

    if (local_date != date) {
        local_date = date;
        EPD_Dis_Part(73, 90, small_numbers[date / 10], 25, 13);
        EPD_Dis_Part(73, 100, small_numbers[date % 10], 25, 13);
        // display_weather();
    }

    if (local_month != month) {
        local_month = month;
        EPD_Dis_Part(73, 120, small_numbers[month / 10], 25, 13);
        EPD_Dis_Part(73, 130, small_numbers[month % 10], 25, 13);
    }

    if (local_year != year) {
        local_year = year;
        EPD_Dis_Part(73, 150, small_numbers[(year / 10) % 10], 25, 13);
        EPD_Dis_Part(73, 160, small_numbers[year % 10], 25, 13);
    }
    EPD_refresh();
}

void display_alarm1()
{
    //// Alarm 1
    EPD_Dis_Part(65, 190, superSmallA, 8, 6);
    EPD_Dis_Part(65, 196, superSmallOne, 8, 6);
    EPD_Dis_Part(65, 202, superSmallColon, 8, 6);
    EPD_Dis_Part(65, 208, super_small_numbers[alarm1_time.tm_hour / 10], 8, 6);
    EPD_Dis_Part(65, 214, super_small_numbers[alarm1_time.tm_hour % 10], 8, 6);
    EPD_Dis_Part(65, 220, superSmallColon, 8, 6);
    EPD_Dis_Part(65, 226, super_small_numbers[alarm1_time.tm_min / 10], 8, 6);
    EPD_Dis_Part(65, 232, super_small_numbers[alarm1_time.tm_min % 10], 8, 6);

    EPD_Dis_Part(57, 190, super_small_numbers[alarm1_time.tm_mday / 10], 8, 6);
    EPD_Dis_Part(57, 196, super_small_numbers[alarm1_time.tm_mday % 10], 8, 6);
    EPD_Dis_Part(57, 202, superSmallDash, 8, 6);
    EPD_Dis_Part(57, 208, super_small_numbers[(alarm1_time.tm_mon + 1) / 10], 8,
        6);
    EPD_Dis_Part(57, 214, super_small_numbers[(alarm1_time.tm_mon + 1) % 10], 8,
        6);
    EPD_Dis_Part(57, 220, superSmallDash, 8, 6);
    EPD_Dis_Part(57, 226, super_small_numbers[(alarm1_time.tm_year / 10) % 10], 8,
        6);
    EPD_Dis_Part(57, 232, super_small_numbers[alarm1_time.tm_year % 10], 8, 6);
    EPD_refresh();
}
void display_alarm2()
{
    EPD_Dis_Part(49, 190, superSmallA, 8, 6);
    EPD_Dis_Part(49, 196, superSmallTwo, 8, 6);
    EPD_Dis_Part(49, 202, superSmallColon, 8, 6);
    EPD_Dis_Part(49, 208, super_small_numbers[alarm2_time.tm_hour / 10], 8, 6);
    EPD_Dis_Part(49, 214, super_small_numbers[alarm2_time.tm_hour % 10], 8, 6);
    EPD_Dis_Part(49, 220, superSmallColon, 8, 6);
    EPD_Dis_Part(49, 226, super_small_numbers[alarm2_time.tm_min / 10], 8, 6);
    EPD_Dis_Part(49, 232, super_small_numbers[alarm2_time.tm_min % 10], 8, 6);

    EPD_Dis_Part(43, 190, super_small_numbers[alarm2_time.tm_mday / 10], 8, 6);
    EPD_Dis_Part(43, 196, super_small_numbers[alarm2_time.tm_mday % 10], 8, 6);
    EPD_Dis_Part(43, 202, superSmallDash, 8, 6);
    EPD_Dis_Part(43, 208, super_small_numbers[(alarm2_time.tm_mon + 1) / 10], 8,
        6);
    EPD_Dis_Part(43, 214, super_small_numbers[(alarm2_time.tm_mon + 1) % 10], 8,
        6);
    EPD_Dis_Part(43, 220, superSmallDash, 8, 6);
    EPD_Dis_Part(43, 226, super_small_numbers[(alarm2_time.tm_year / 10) % 10], 8,
        6);
    EPD_Dis_Part(43, 232, super_small_numbers[alarm2_time.tm_year % 10], 8, 6);
    EPD_refresh();
}

void display_weather()
{
    char extracted_date[11]; // To hold the YYYY-MM-DD format
    strcpy(extracted_date, "2024-10-22"); // Temporary test date

    for (int i = 0; i < MAX_DAYS; i++) {

        if (strcmp(weather_data[i].date, extracted_date) == 0) {

            // Display the weather icon
            if (strcmp(weather_data[i].forecast, "Sunny") == 0) {
                EPD_Dis_Part(82, 0, sunny, 36, 100);
            } else if (strcmp(weather_data[i].forecast, "Cloudy") == 0) {
                EPD_Dis_Part(82, 0, cloudy, 36, 100);
            } else if (strcmp(weather_data[i].forecast, "Rain") == 0) {
                EPD_Dis_Part(82, 0, rain, 36, 100);
            } else if (strcmp(weather_data[i].forecast, "Snow") == 0) {
                EPD_Dis_Part(82, 0, snow, 36, 100);
            } else if (strcmp(weather_data[i].forecast, "Storm") == 0) {
                EPD_Dis_Part(82, 0, storm, 36, 100);
            }

            // Display the temperature
            int temp_value = atoi(
                weather_data[i].temperature); // Convert temperature string to int
            if (temp_value >= 0 && temp_value <= 99) {
                EPD_Dis_Part(73, 206, small_numbers[temp_value / 10], 25, 13);
                EPD_Dis_Part(73, 219, small_numbers[temp_value % 10], 25, 13);
            }

            // Display temperature unit
            if (strcmp(temp_unit, "F") == 0) {
                EPD_Dis_Part(73, 232, fahrenheight, 25, 13); // Fahrenheit icon
            } else {
                EPD_Dis_Part(73, 232, celcius, 25, 13); // Celsius icon
            }

            // Display the humidity
            int humidity_value = atoi(weather_data[i].humidity); // Convert humidity string to int
            if (humidity_value >= 0 && humidity_value <= 99) {
                EPD_Dis_Part(100, 196, small_numbers[humidity_value / 10], 25, 13);
                EPD_Dis_Part(100, 209, small_numbers[humidity_value % 10], 25, 13);
            }

            // Refresh the display once
            EPD_refresh();
        }
    }
}

void rotate_display()
{
    if (is_rotated) {
        EPD_W21_WriteCMD(0x00);
        EPD_W21_WriteDATA(0xF3); // You can change the scanning direction, FF -> F3
    } else {
        EPD_W21_WriteCMD(0x00);
        EPD_W21_WriteDATA(0xFF);
    }
    EPD_Dis_Part(0, 76, colon, 73, 38);
    EPD_Dis_Part(73, 110, dash, 25, 13);
    EPD_Dis_Part(73, 140, dash, 25, 13);
    EPD_Dis_Part(73, 110, dash, 25, 13);
    EPD_Dis_Part(73, 140, dash, 25, 13);
    EPD_Dis_Part(100, 170, h, 25, 13);
    EPD_Dis_Part(100, 183, smallcolon, 25, 13);
    EPD_Dis_Part(100, 222, percentage, 25, 13);
    EPD_Dis_Part(73, 180, t, 25, 13);
    EPD_Dis_Part(73, 193, smallcolon, 25, 13);
    EPD_Dis_Part(0, 210, usb, 36, 20);
    local_hour = 99;
    local_minute = 61;
    local_day = 32;
    local_date = 13;
    local_month = 121;
    local_year = 0;
    display_time();

    EPD_refresh();
}

void main_display(void)
{
    //***************************************************************//
    // Power on: Reset ---> Initialize settings ---> Use full refresh waveform to
    // refresh the screen. The first screen after initialization must be refreshed
    // with full refresh waveform
    //***************************************************************/
    EPD_Reset(); // Electronic paper IC reset
    EPD_init(); // EPD init
    _delay_ms(200);
    PIC_display_GC(colon);
    // display_time();
    EPD_Dis_Part(0, 76, colon, 73, 38);
    EPD_Dis_Part(73, 110, dash, 25, 13);
    EPD_Dis_Part(73, 140, dash, 25, 13);
    EPD_Dis_Part(73, 110, dash, 25, 13);
    EPD_Dis_Part(73, 140, dash, 25, 13);
    EPD_Dis_Part(100, 170, h, 25, 13);
    EPD_Dis_Part(100, 183, smallcolon, 25, 13);
    EPD_Dis_Part(100, 196, smallzero, 25, 13);
    EPD_Dis_Part(100, 209, smallfive, 25, 13);
    EPD_Dis_Part(100, 222, percentage, 25, 13);
    EPD_Dis_Part(73, 180, t, 25, 13);
    EPD_Dis_Part(73, 193, smallcolon, 25, 13);
    EPD_Dis_Part(73, 206, smalltwo, 25, 13);
    EPD_Dis_Part(73, 219, smallone, 25, 13);
    EPD_Dis_Part(73, 232, celcius, 25, 13);
    EPD_Dis_Part(0, 210, usb, 36, 20);
    EPD_Dis_Part(82, 0, cloudy, 36, 100);
    //

    //
    EPD_refresh();
}