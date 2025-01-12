#include <avr/eeprom.h>
#include <avr/io.h> // For PORTB and bit manipulation
#include <stdio.h>
#include <string.h>
#include <time.h>

// Global variables declared elsewhere (as externs)
extern char alarm1_msg[257]; // 256 chars + '\0'
extern char alarm2_msg[257]; // 256 chars + '\0'
extern char alarm1_led[2];
extern char alarm1_buzzer[2];
extern char alarm2_led[2];
extern char alarm2_buzzer[2];
extern char alarm1[17]; // 16 chars + '\0'
extern char alarm2[17]; // 16 chars + '\0'

extern uint8_t flag_accelerated_time;

// Function prototypes
extern void load_alarms(void); // Load alarm times into struct tm
extern void check_alarms(void); // Check if alarms should be triggered
extern void play_alarm(void); // Play alarm sound and light LED
extern uint8_t alarm_number; // Flag to indicate if alarm is on
extern uint8_t led_on; // Flag to indicate if LED should be on
extern uint8_t buzzer_on; // Flag to indicate if buzzer should be on
extern uint8_t
    flag_alarm_dismissed; // Flag to indicate if alarm has been dismissed
extern struct tm alarm1_time; // Alarm 1 time
extern struct tm alarm2_time; // Alarm 2 time

extern uint8_t a1set;
extern uint8_t a2set;
