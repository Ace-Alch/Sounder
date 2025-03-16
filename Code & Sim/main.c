#include <avr/io.h>          // Include AVR input/output definitions
#include <avr/interrupt.h>   // Include AVR interrupt handling definitions
#include <avr/sleep.h>       // Include AVR sleep mode definitions
#include <util/delay.h>      // Include AVR delay functions
#include <stdio.h>           // Include standard input/output library

#define F_CPU 16000000       // Define CPU frequency as 16 MHz
#define BAUD 9600            // Define UART baud rate as 9600
#define UBRR_VALUE ((F_CPU/16/BAUD) - 1)  // Calculate UBRR value for baud rate
#define MIN_FREQ 50          // Define the minimum frequency as 50 Hz
#define MAX_FREQ 1000        // Define the maximum frequency as 1000 Hz

// Function prototypes
void uart_init(void);                // Function to initialize UART
void uart_send_string(const char* str);  // Function to send a string via UART
void adc_init(void);                 // Function to initialize ADC
uint16_t adc_read(void);             // Function to read a value from ADC
void uart_send_char(char c);         // Function to send a character via UART
void timer1_init(void);              // Function to initialize Timer1
void pwm_init(void);                 // Function to initialize PWM
void set_frequency(uint16_t frequency);  // Function to set sound frequency
void set_pwm_duty_cycle(uint8_t duty_cycle);  // Function to set PWM duty cycle
void check_uart_input(void);         // Function to check UART input (prototype)

// Global variables
volatile uint16_t frequency = 440;   // Default frequency set to 440 Hz (A4)
volatile uint8_t system_active = 0;  // Flag indicating if the system is active
volatile uint8_t paused = 0;         // Flag indicating if the system is paused

// Initialize UART
void uart_init(void) {
    // Set baud rate using the calculated UBRR value
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);  // High byte of UBRR
    UBRR0L = (uint8_t)UBRR_VALUE;        // Low byte of UBRR

    // Enable transmitter (TX), receiver (RX), and RX complete interrupt
    UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0);

    // Configure frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (0 << USBS0) | (3 << UCSZ00);
}

// Send a character via UART
void uart_send_char(char c) {
    // Wait until the transmit buffer is empty
    while (!(UCSR0A & (1 << UDRE0)));
    // Send the character
    UDR0 = c;
}

// Send a string via UART
void uart_send_string(const char* str) {
    // Iterate through each character in the string
    while (*str) {
        uart_send_char(*str++);  // Send each character
    }
}

// Initialize ADC for potentiometer input
void adc_init(void) {
    ADMUX = (1 << REFS0);  // Set reference voltage to AVCC
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);  // Enable ADC with prescaler 64
}

// Read a value from the ADC
uint16_t adc_read(void) {
    ADMUX = (ADMUX & 0xF0) | 0x01;  // Select ADC1 channel
    ADCSRA |= (1 << ADSC);          // Start ADC conversion
    while (ADCSRA & (1 << ADSC));   // Wait for conversion to complete
    return ADC;                     // Return the ADC value
}

// Initialize Timer1 for sound generation
void timer1_init(void) {
    TCCR1A = 0;                     // Set Timer1 to normal mode
    TCCR1B = (1 << WGM12) | (1 << CS10);  // Configure Timer1 for CTC mode with no prescaling
    TCNT1 = 0;                      // Initialize counter value
    OCR1A = F_CPU / (2 * 440) - 1;  // Set output compare value for default frequency 440 Hz
    TIMSK1 = (1 << OCIE1A);         // Enable compare match interrupt
    DDRB |= (1 << PB2);             // Set PB2 as output for sound
}

// Initialize PWM for LED brightness control
void pwm_init(void) {
    DDRD |= (1 << PD6);             // Set PD6 as output (PWM output pin)
    TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);  // Fast PWM mode, non-inverting
    TCCR0B = (1 << CS01);           // Set prescaler to 8
}

// Set the sound frequency
void set_frequency(uint16_t freq) {
    if (freq < MIN_FREQ) freq = MIN_FREQ;  // Clamp frequency to minimum limit
    if (freq > MAX_FREQ) freq = MAX_FREQ;  // Clamp frequency to maximum limit
    OCR1A = F_CPU / (2 * freq) - 1;        // Update compare value for the new frequency
}

// Set the PWM duty cycle for LED brightness
void set_pwm_duty_cycle(uint8_t duty_cycle) {
    OCR0A = duty_cycle;             // Set duty cycle (0-255 range)
}

// Timer1 Compare Match A Interrupt Service Routine
ISR(TIMER1_COMPA_vect) {
    if (system_active && !paused) {
        PORTB ^= (1 << PB2);        // Toggle PB2 to generate sound
    }
}

// UART Receive Complete Interrupt Service Routine
ISR(USART_RX_vect) {
    char received_char = UDR0;      // Read the received character
    if (received_char == '+') {
        frequency += 10;            // Increase frequency by 10 Hz
        if (frequency > MAX_FREQ) frequency = MAX_FREQ;  // Clamp to max frequency
    } else if (received_char == '-') {
        frequency -= 10;            // Decrease frequency by 10 Hz
        if (frequency < MIN_FREQ) frequency = MIN_FREQ;  // Clamp to min frequency
    }
    set_frequency(frequency);       // Update frequency
}

// Main program
int main(void) {
       char buffer[50];                // Buffer for UART output
       uint16_t adc_value;             // ADC value for potentiometer input
       uint8_t prev_switch_state;      // Previous state of the switch
       uint8_t print_counter = 0;      // Counter for periodic UART updates
       uint8_t current_switch_state;   // Current state of the switch
       uint8_t brightness;             // LED brightness value
       uint8_t reset_button_state;     // State of the reset button
       uint8_t prev_reset_button_state = 1;  // Previous state of the reset button

    // Configure I/O ports
    DDRB = 0;                       // Set all PORTB pins as input
    DDRC = 0;                       // Set all PORTC pins as input
    DDRD = 0;                       // Set all PORTD pins as input
    PORTB = 0;                      // Disable pull-up resistors on PORTB
    PORTC = 0;                      // Disable pull-up resistors on PORTC
    PORTD = 0;                      // Disable pull-up resistors on PORTD
    // Configure specific pins
    DDRC |= (1 << PC3);      // Set PC3 as output for LED
    DDRB |= (1 << PB2);      // Set PB2 as output for sounder
    DDRD |= (1 << PD1);      // Set PD1 as output for UART TXD
    DDRD &= ~(1 << PD0);     // Set PD0 as input for UART RXD
    DDRC &= ~(1 << PC1);     // Set PC1 as input for potentiometer (ADC1)
    DDRC &= ~(1 << PC4);     // Set PC4 as input for the switch
    DDRC &= ~(1 << PC5);     // Set PC5 as input for the reset button

    PORTC |= (1 << PC4);     // Enable pull-up resistor on switch (PC4)
    PORTC |= (1 << PC5);     // Enable pull-up resistor on reset button (PC5)

    prev_switch_state = (PINC & (1 << PC4)) >> PC4;  // Read the initial state of the switch

    // Initialize peripherals
    uart_init();             // Initialize UART for communication
    adc_init();              // Initialize ADC for potentiometer input
    pwm_init();              // Initialize PWM for LED brightness control
    timer1_init();           // Initialize Timer1 for sound generation

    sei();                   // Enable global interrupts

    _delay_ms(100);          // Short delay for system initialization

    uart_send_string("System Started\r\n");  // Send system start message via UART

    while (1) {              // Main loop
        current_switch_state = (PINC & (1 << PC4)) >> PC4;  // Read current switch state
        reset_button_state = (PINC & (1 << PC5)) >> PC5;    // Read reset button state

        // Handle switch state changes
        if (current_switch_state != prev_switch_state) {
            _delay_ms(20);   // Debounce delay
            current_switch_state = (PINC & (1 << PC4)) >> PC4;  // Recheck switch state

            if (current_switch_state != prev_switch_state) {
                if (current_switch_state == 0) {  // Switch pressed
                    system_active = !system_active;  // Toggle system activity state
                    if (system_active) {
                        uart_send_string("System ON\r\n");  // Indicate system ON via UART
                        PORTC |= (1 << PC3);               // Turn on LED
                        TIMSK1 |= (1 << OCIE1A);           // Enable Timer1 interrupt
                    } else {
                        uart_send_string("System OFF\r\n");  // Indicate system OFF via UART
                        PORTC &= ~(1 << PC3);               // Turn off LED
                        TIMSK1 &= ~(1 << OCIE1A);           // Disable Timer1 interrupt
                        set_pwm_duty_cycle(0);              // Turn off PWM (LED)
                        PORTB &= ~(1 << PB2);               // Ensure sound is off
                    }
                }
                prev_switch_state = current_switch_state;  // Update previous switch state
            }
        }

        // Handle reset button state changes
        if (reset_button_state != prev_reset_button_state) {
            _delay_ms(20);   // Debounce delay
            reset_button_state = (PINC & (1 << PC5)) >> PC5;  // Recheck reset button state

            if (reset_button_state != prev_reset_button_state) {
                if (reset_button_state == 0) {  // Reset button pressed
                    paused = !paused;           // Toggle paused state
                    if (paused) {
                        uart_send_string("System Paused\r\n");  // Indicate pause via UART
                    } else {
                        uart_send_string("System Resumed\r\n");  // Indicate resume via UART
                    }
                }
                prev_reset_button_state = reset_button_state;  // Update previous reset button state
            }
        }

        // If the system is active and not paused, update frequency and brightness
        if (system_active && !paused) {
            adc_value = adc_read();  // Read ADC value from potentiometer
            frequency = MIN_FREQ + ((uint32_t)adc_value * (MAX_FREQ - MIN_FREQ)) / 1023;  // Map ADC value to frequency

            set_frequency(frequency);  // Update the sound frequency

            brightness = 255 - ((uint32_t)(frequency - MIN_FREQ) * 255) / (MAX_FREQ - MIN_FREQ);  // Map frequency to brightness
            set_pwm_duty_cycle(brightness);  // Update LED brightness

            // Print frequency periodically (~2 seconds)
            if (print_counter++ >= 40) {
                sprintf(buffer, "Current frequency: %d Hz\r\n", frequency);  // Format frequency message
                uart_send_string(buffer);  // Send message via UART
                print_counter = 0;        // Reset counter
            }
        }

        _delay_ms(50);  // Delay for stability
    }

    return 0;  // Return 0 (not expected to reach here in an embedded program)
}
