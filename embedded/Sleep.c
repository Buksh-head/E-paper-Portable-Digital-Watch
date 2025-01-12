#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#define F_CPU 16000000UL

// green led on PC3

// Interrupt service routine for INT1 (connected to PD3)
// ISR(INT1_vect) {
// 	// This ISR is automatically triggered when the button on PD3 (pin 5) is
// pressed 	sleep_disable();  // Disable sleep after waking up

// 	// Re-enable UART after waking up
// 	UCSR0B = (1 << TXEN0) | (1 << RXEN0);  // Re-enable UART transmit and
// receive 	PRR &= ~(1 << PRUSART0); // Re-enable the USART clock in the
// power reduction register
// }

// Function to configure external interrupt on PD3 (INT1)
void configure_interrupt(void)
{
    // Configure PD3 (pin 5) as input with a pull-up resistor
    DDRD &= ~(1 << PD3); // Set PD3 as input
    PORTD |= (1 << PD3); // Enable internal pull-up resistor on PD3

    // Configure external interrupt on INT1 (PD3)
    EICRA |= (1 << ISC11); // Set INT1 to trigger on falling edge (button press)
    EIMSK |= (1 << INT1); // Enable external interrupt INT1 (on PD3)z
}

// Function to disable UART and set TX/RX pins to high-impedance before sleeping
void disable_uart(void)
{
    // Disable UART (USART0)
    UCSR0B = 0; // Disable the UART receiver and transmitter
    UCSR0C = 0; // Clear the control registers

    // Disable USART clock by setting PRUSART0 bit in the PRR register
    PRR |= (1 << PRUSART0); // Disable USART in the power reduction register

    // Set TX (PD1) and RX (PD0) pins to high-impedance (input mode with no
    // pull-up)
    DDRD &= ~(1 << PD1); // Set PD1 (TX) as input
    PORTD &= ~(1 << PD1); // Disable pull-up on PD1 (TX)
    DDRD &= ~(1 << PD0); // Set PD0 (RX) as input
    PORTD &= ~(1 << PD0); // Disable pull-up on PD0 (RX)
}

// Function to put the microcontroller into sleep mode
void sleepNow(void)
{
    // Disable UART before sleep
    disable_uart();

    // Set sleep mode to Power Down (lowest power mode)
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);

    // Disable interrupts before entering sleep to prevent accidental wake-ups
    cli();
    sleep_enable(); // Enable sleep mode
    sei(); // Re-enable interrupts (allowing wake-up from sleep)

    // Put the microcontroller to sleep (execution halts here)
    sleep_cpu();

    // After waking up, disable sleep mode to avoid re-entering it
    sleep_disable();
}

int sleep_main(void)
{
    // Initialize external interrupt on PD3 (INT1)
    configure_interrupt();

    // Ensure that nothing else is consuming extra current
    PRR = (1 << PRADC) | (1 << PRSPI) | (1 << PRTIM1) | (1 << PRTWI); // Disable other peripherals to save power

    // Put the microcontroller into sleep mode immediately
    sleepNow();
}
