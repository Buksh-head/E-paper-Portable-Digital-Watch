#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define F_CPU 16000000UL
#define BAUD 9600 // 115200
#define BAUD_PRESCALE (((F_CPU / (BAUD * 16UL))) - 1)
#define BUFFER_SIZE 800
#define MAX_DAYS 7
#define EEPROM_TIME_ADDR 0x00 // Start address for storing time
#define EEPROM_FORMAT_ADDR 0x17 // 8 bytes (time_format[8])
#define EEPROM_ALARM1_ADDR \
    (EEPROM_FORMAT_ADDR + 8) // 0x08, 17 bytes (alarm1[17])
#define EEPROM_ALARM2_ADDR \
    (EEPROM_ALARM1_ADDR + 17) // 0x19, 17 bytes (alarm2[17])
#define EEPROM_ALARM1_LED_ADDR \
    (EEPROM_ALARM2_ADDR + 17) // 0x2A, 2 bytes (alarm1_led[2])
#define EEPROM_ALARM1_BUZZER_ADDR \
    (EEPROM_ALARM1_LED_ADDR + 2) // 0x2C, 2 bytes (alarm1_buzzer[2])
#define EEPROM_ALARM2_LED_ADDR \
    (EEPROM_ALARM1_BUZZER_ADDR + 2) // 0x2E, 2 bytes (alarm2_led[2])
#define EEPROM_ALARM2_BUZZER_ADDR \
    (EEPROM_ALARM2_LED_ADDR + 2) // 0x30, 2 bytes (alarm2_buzzer[2])
#define EEPROM_ALARM1_MSG_ADDR \
    (EEPROM_ALARM2_BUZZER_ADDR + 2) // 0x32, 257 bytes (alarm1_msg[257])
#define EEPROM_ALARM2_MSG_ADDR \
    (EEPROM_ALARM1_MSG_ADDR + 257) // 0x133, 257 bytes (alarm2_msg[257])
#define EEPROM_WEATHER_ADDR \
    (EEPROM_ALARM2_MSG_ADDR + 257) // 0x234, 2 bytes (temp_unit[2]) + weather_data

extern void USART_init(void);
void USART_send_char(char c);
extern void USART_send_string(const char* str);
void parse_and_save_data(void);
extern void send_configuration_data_to_gui(void);
void USART_flush(void);
void store_configuration_in_eeprom();
void load_configuration_from_eeprom();

extern char datetime[17]; // For "08:50 PM 11/09/2024"
extern char time_format[8]; // For "12-hour"
extern char alarm1[17]; // For "A1: No"
extern char alarm2[17]; // For "A2: No"
extern char temp_unit[2]; // For "C"
extern uint8_t data_ready;
extern char alarm1_led[2];
extern char alarm1_buzzer[2];
extern char alarm2_led[2];
extern char alarm2_buzzer[2];
extern char alarm1_msg[257];
extern char alarm2_msg[257];

struct WeatherData {
    char date[11]; // Date in "YYYY-MM-DD" time_format
    char forecast[7]; // Forecast like "Sunny", "Cloudy", etc.
    char temperature[5]; // Temperature as a string (to avoid floating point
    // issues)
    char humidity[4]; // Humidity as a string (e.g., "65.0")
};
extern struct WeatherData weather_data[MAX_DAYS];
