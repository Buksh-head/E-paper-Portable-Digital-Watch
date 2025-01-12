#include <avr/io.h>
#include <stdint.h>

extern void ADC_init(void);
uint16_t ADC_read(void);
extern void process_light_sensor(void);
void convert_and_send_time(char* binary_data);
int binary_to_int(const char* binary_str);

extern uint8_t finished;
extern uint8_t started;
extern uint8_t optical_count;
extern char datetime[17];
extern uint8_t optical_display;
