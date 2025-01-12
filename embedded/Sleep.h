#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

extern void configure_interrupt(void);
extern void sleepNow(void);
extern int sleep_main(void);