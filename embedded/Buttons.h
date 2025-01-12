#include <stdint.h> // For uint8_t

// Global flags and variables to store the button states
extern uint8_t
    flag_accelerated_time; // 0: normal, 1: accelerated1, 2: accelerated2
extern uint8_t flag_optical_start; // 0: not started, 1: started
extern uint8_t flag_time_format; // 0: 12-hour, 1: 24-hour
extern uint8_t flag_alarm_dismissed; // 0: not dismissed, 1: dismissed

// Function prototypes
extern void init_buttons(void); // Initialize all buttons
void button_accelerated_time(void); // Handle button press for accelerated time
void button_optical_start(void); // Handle button press for optical start
void button_24hr_format(
    void); // Handle button press for toggling 12/24-hour format
void button_dismiss_alarm(void); // Handle button press for dismissing alarm
extern void button_check(void); // Check the state of all buttons