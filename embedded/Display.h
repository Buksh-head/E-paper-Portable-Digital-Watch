
#include <avr/io.h>
#include <avr/pgmspace.h>

// Pin definitions
#define BUSY (PIND & (1 << PD2)) // INT0 (PD2 for busy)
#define RST PB0 // Reset pin
#define DC PB1 // Data/Command pin
#define CS1 PB2 // Chip Select pin
#define SDA PB3 // MOSI
#define SCK PB5 // SCK
#define F_CPU 16000000UL

#define uchar unsigned char
#define uint unsigned int

// xx Private macro
// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

// Set pin high or low macros

#define SDA_H (PORTB |= (1 << SDA)) // Set SDA pin high (MOSI - PB3)
#define SDA_L (PORTB &= ~(1 << SDA)) // Set SDA pin low
#define SCLK_H (PORTB |= (1 << SCK)) // Set SCK pin high (SCK - PB5)
#define SCLK_L (PORTB &= ~(1 << SCK)) // Set SCK pin low
#define nCS_H (PORTB |= (1 << CS1)) // Set CS1 pin high (PB2)
#define nCS_L (PORTB &= ~(1 << CS1)) // Set CS1 pin low
#define nDC_H (PORTB |= (1 << DC)) // Set DC pin high (PD6)
#define nDC_L (PORTB &= ~(1 << DC)) // Set DC pin low
#define nRST_H (PORTB |= (1 << RST)) // Set RST pin high (PD7)
#define nRST_L (PORTB &= ~(1 << RST)) // Set RST pin low

#define DELAY_TIME 2
#define PIC_BLACK 252
#define PIC_WHITE 255
#define PIC_A 1
#define PIC_B 2
#define PIC_C 3
#define PIC_HLINE 4
#define PIC_VLINE 5
#define PIC_GRAY 6
#define PIC_D 7
#define PIC_E 8
#define PIC_F 9

#define Source_Pixel 128
#define Gate_Pixel 250

extern void EPD_Reset(void); // Hardware Reset
extern void EPD_W21_WriteCMD(unsigned char command); // Write command
extern void EPD_W21_WriteDATA(unsigned char data); // Write data
extern void SPI_Transmit(unsigned char data); // SPI send data
extern void EPD_init(void); // Write Initialization
extern void EPD_refresh(void); // Start refreshing the screen
extern void EPD_sleep(void); // sleep
extern void lut_GC(void); // Write full brush waveform
extern void lut_DU(void); // Write partial brush waveform
extern void PIC_display_GC(const unsigned char* picData); // full brush
extern void PIC_display_DU(const unsigned char* picData); // partial brush
extern void Clean_display_GC(const unsigned char Dat);
extern void lcd_chkstatus(void); // Determine whether the screen is idle

extern void SPI_Init(void); // SPI initialization
extern void main_display(void);

extern void display_time(void);
extern void display_optical(void);
extern void display_alarm1();
extern void display_alarm2();
extern void display_weather();
extern void rotate_display();
