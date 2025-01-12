#include "UART.h"
#include "Alarm.h"
#include "Buttons.h"
#include "main.h"

char uart_buffer[800];
uint16_t buffer_index = 0;
uint8_t data_ready = 0;
uint8_t flag_coms_connected = 0;

char datetime[17] = "00:00 21/10/2024";
char time_format[8] = "12-hour";
char alarm1[17] = "No";
char alarm1_msg[257] = "";
char alarm2[17] = "No";
char alarm2_msg[257] = "";
char temp_unit[2] = "C";
char alarm1_led[2] = "0";
char alarm1_buzzer[2] = "0";
char alarm2_led[2] = "0";
char alarm2_buzzer[2] = "0";

struct WeatherData weather_data[MAX_DAYS];
// Initialize USART for serial communication
void USART_init()
{
    // Set baud rate
    UBRR0H = (BAUD_PRESCALE >> 8);
    UBRR0L = BAUD_PRESCALE;
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); // Enable RX, TX, and RX Complete Interrupt
    // Set frame time_format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
    sei(); // Enable global interrupts
}

// Flush the UART receive buffer
void USART_flush(void)
{
    unsigned char dummy;
    while (UCSR0A & (1 << RXC0)) {
        dummy = UDR0; // Read and discard incoming data
    }
}

// ISR for UART Receive Complete
ISR(USART_RX_vect)
{
    char input = UDR0;
    if (buffer_index < BUFFER_SIZE - 1) {
        uart_buffer[buffer_index++] = input;
        uart_buffer[buffer_index] = '\0'; // Null-terminate the string
    }

    // Check for newline or delimiter
    if (input == '\n') {
        if (strcmp(uart_buffer, "GETDATA\n") == 0) {
            send_configuration_data_to_gui();

        } else if (strcmp(uart_buffer, "Connection Status: Connected\n") == 0) {
            flag_coms_connected = 1;
        } else {
            char temp[6];
            strncpy(temp, uart_buffer, 6);
            temp[5] = '\0';

            if (strcmp(temp, "SEND:") == 0) {
                parse_and_save_data();
            }
        }
        buffer_index = 0;
        memset(uart_buffer, 0, sizeof(uart_buffer));
        buffer_index = 0;
    }
    USART_flush();
}

// Send a single character over USART
void USART_send_char(char c)
{
    // Wait for the transmit buffer to be ready
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = c; // Load the data into the buffer for transmission
}

// Send a string over USART
void USART_send_string(const char* str)
{
    while (*str) {
        USART_send_char(*str++);
    }
}

void parse_and_save_data(void)
{
    char* rest = uart_buffer + 5; // Skip "SEND:"
    char* token;
    char *saveptr1, *saveptr2, *saveptr3;

    // Parse datetime
    token = strtok_r(rest, ",", &saveptr1);
    if (token != NULL) {
        strncpy(datetime, token, sizeof(datetime));
    }

    // Parse time_format
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(time_format, token, sizeof(time_format));
    }

    // Parse alarm1
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm1, token, sizeof(alarm1));
    }

    // Parse alarm1_msg
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm1_msg, token, sizeof(alarm1_msg));
    }

    // Parse alarm1_led
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm1_led, token, sizeof(alarm1_led));
    }

    // Parse alarm1_buzzer
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm1_buzzer, token, sizeof(alarm1_buzzer));
    }

    // Parse alarm2
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm2, token, sizeof(alarm2));
    }

    // Parse alarm2_msg
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm2_msg, token, sizeof(alarm2_msg));
    }

    // Parse alarm2_led
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm2_led, token, sizeof(alarm2_led));
    }

    // Parse alarm2_buzzer
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(alarm2_buzzer, token, sizeof(alarm2_buzzer));
    }

    // Parse temp_unit
    token = strtok_r(NULL, ",", &saveptr1);
    if (token != NULL) {
        strncpy(temp_unit, token, sizeof(temp_unit));
    }

    // Now parse weather data
    char* weather_info = saveptr1; // Remaining string after the last comma
    if (weather_info != NULL) {
        int day_index = 0;
        char* day_data = strtok_r(weather_info, ";", &saveptr2);
        while (day_data != NULL && day_index < MAX_DAYS) {
            int field_index = 0;
            char* field = strtok_r(day_data, ",", &saveptr3);
            while (field != NULL) {
                switch (field_index) {
                case 0:
                    strncpy(weather_data[day_index].date, field,
                        sizeof(weather_data[day_index].date) - 1);
                    weather_data[day_index].date[10] = '\0'; // Ensure null-termination
                    break;
                case 1:
                    strncpy(weather_data[day_index].forecast, field,
                        sizeof(weather_data[day_index].forecast) - 1);
                    weather_data[day_index].forecast[6] = '\0'; // Ensure null-termination
                    break;
                case 2:
                    strncpy(weather_data[day_index].temperature, field,
                        sizeof(weather_data[day_index].temperature) - 1);
                    weather_data[day_index].temperature[4] = '\0'; // Ensure null-termination
                    break;
                case 3:
                    strncpy(weather_data[day_index].humidity, field,
                        sizeof(weather_data[day_index].humidity) - 1);
                    weather_data[day_index].humidity[3] = '\0'; // Ensure null-termination
                    break;
                }
                field = strtok_r(NULL, ",", &saveptr3);
                field_index++;
            }
            day_data = strtok_r(NULL, ";", &saveptr2);
            day_index++;
        }
    }

    store_configuration_in_eeprom();

    if (strcmp(time_format, "24-hour") == 0) {
        flag_time_format = 1;
    } else {
        flag_time_format = 0;
    }
    // Update timekeeping variables
    int hours, minutes, day, month, year;
    sscanf(datetime, "%d:%d %d/%d/%d", &hours, &minutes, &day, &month, &year);
    timeinfo.tm_hour = hours;
    timeinfo.tm_min = minutes;
    timeinfo.tm_mday = day;
    timeinfo.tm_mon = month - 1; // Months are 0-11 in struct tm
    timeinfo.tm_year = year - 1900; // Years since 1900

    timeKeeping = mktime(&timeinfo);
    load_alarms();
}

// Send configuration data back to Python
void send_configuration_data_to_gui(void)
{
    // Format the data string to send to Python
    load_configuration_from_eeprom();
    USART_send_string(datetime);
    USART_send_char(',');
    USART_send_string(time_format);
    USART_send_char(',');
    USART_send_string(alarm1);
    USART_send_char(',');
    USART_send_string(alarm2);
    USART_send_char(',');
    USART_send_string(alarm1_led);
    USART_send_char(',');
    USART_send_string(alarm1_buzzer);
    USART_send_char(',');
    USART_send_string(alarm2_buzzer);
    USART_send_char(',');
    USART_send_string(alarm2_led);
    USART_send_char(',');
    USART_send_string(temp_unit);
    USART_send_char(',');
    USART_send_string(alarm1_msg);
    USART_send_char(',');
    USART_send_string(alarm2_msg);

    // Send weather data
    for (int i = 0; i < MAX_DAYS; i++) {
        USART_send_char(',');
        USART_send_string(weather_data[i].date);
        USART_send_char(',');
        USART_send_string(weather_data[i].forecast);
        USART_send_char(',');
        USART_send_string(weather_data[i].temperature);
        USART_send_char(',');
        USART_send_string(weather_data[i].humidity);
    }

    data_ready = 0;
}

void store_configuration_in_eeprom()
{
    cli();
    eeprom_update_block((const void*)datetime, (void*)EEPROM_TIME_ADDR,
        17); // 16 + '\0'
    eeprom_update_block((const void*)alarm1, (void*)EEPROM_ALARM1_ADDR,
        17); // 16 + '\0'
    eeprom_update_block((const void*)alarm2, (void*)EEPROM_ALARM2_ADDR,
        17); // 16 + '\0'

    eeprom_update_block((const void*)alarm1_led, (void*)EEPROM_ALARM1_LED_ADDR,
        2); // 1 + '\0'
    eeprom_update_block((const void*)alarm1_buzzer,
        (void*)EEPROM_ALARM1_BUZZER_ADDR, 2); // 1 + '\0'
    eeprom_update_block((const void*)alarm2_led, (void*)EEPROM_ALARM2_LED_ADDR,
        2); // 1 + '\0'
    eeprom_update_block((const void*)alarm2_buzzer,
        (void*)EEPROM_ALARM2_BUZZER_ADDR, 2); // 1 + '\0'

    eeprom_update_block((const void*)alarm1_msg, (void*)EEPROM_ALARM1_MSG_ADDR,
        257); // 256 + '\0'
    eeprom_update_block((const void*)alarm2_msg, (void*)EEPROM_ALARM2_MSG_ADDR,
        257); // 256 + '\0'
    eeprom_update_block((const void*)temp_unit, (void*)EEPROM_WEATHER_ADDR,
        2); // 1 + '\0'

    for (int i = 0; i < MAX_DAYS; i++) {
        uint16_t base_addr = EEPROM_WEATHER_ADDR + 2 + i * 28; // 2 bytes for temp_unit
        eeprom_update_block((const void*)weather_data[i].date, (void*)base_addr,
            11);
        eeprom_update_block((const void*)weather_data[i].forecast,
            (void*)(base_addr + 11), 7);
        eeprom_update_block((const void*)weather_data[i].temperature,
            (void*)(base_addr + 18), 5);
        eeprom_update_block((const void*)weather_data[i].humidity,
            (void*)(base_addr + 23), 4);
    }
    sei();
}

void load_configuration_from_eeprom()
{
    cli();
    eeprom_read_block((void*)datetime, (const void*)EEPROM_TIME_ADDR,
        17); // 16 + '\0'
    eeprom_read_block((void*)alarm1, (const void*)EEPROM_ALARM1_ADDR,
        17); // 16 + '\0'
    eeprom_read_block((void*)alarm2, (const void*)EEPROM_ALARM2_ADDR,
        17); // 16 + '\0'

    eeprom_read_block((void*)alarm1_led, (const void*)EEPROM_ALARM1_LED_ADDR,
        2); // 1 + '\0'
    eeprom_read_block((void*)alarm1_buzzer,
        (const void*)EEPROM_ALARM1_BUZZER_ADDR, 2); // 1 + '\0'
    eeprom_read_block((void*)alarm2_led, (const void*)EEPROM_ALARM2_LED_ADDR,
        2); // 1 + '\0'
    eeprom_read_block((void*)alarm2_buzzer,
        (const void*)EEPROM_ALARM2_BUZZER_ADDR, 2); // 1 + '\0'

    eeprom_read_block((void*)alarm1_msg, (const void*)EEPROM_ALARM1_MSG_ADDR,
        257); // 256 + '\0'
    eeprom_read_block((void*)alarm2_msg, (const void*)EEPROM_ALARM2_MSG_ADDR,
        257); // 256 + '\0'

    eeprom_read_block((void*)temp_unit, (const void*)EEPROM_WEATHER_ADDR,
        2); // 1 + '\0'

    for (int i = 0; i < MAX_DAYS; i++) {
        uint16_t base_addr = EEPROM_WEATHER_ADDR + 2 + i * 28; // 2 bytes for temp_unit
        eeprom_read_block((void*)weather_data[i].date, (const void*)base_addr,
            11);
        eeprom_read_block((void*)weather_data[i].forecast,
            (const void*)(base_addr + 11), 7);
        eeprom_read_block((void*)weather_data[i].temperature,
            (const void*)(base_addr + 18), 5);
        eeprom_read_block((void*)weather_data[i].humidity,
            (const void*)(base_addr + 23), 4);
    }
    sei();
}
