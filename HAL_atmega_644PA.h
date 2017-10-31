#ifndef _HAL_ATMEGA_644PA_H
#define _HAL_ATMEGA_644PA_H

#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>


//- debug specific --------------------------------------------------------------------------------------------------------
#define USART_1						// USART0 or USART1

#if defined(USART_0)
	#define power_serial_enable()   power_usart0_enable();
	#define power_serial_disable()  power_usart0_disable();
	#define SerialX					Serial
	#define dbg						SerialX
	#define UCSR                    UCSR0B
	#define RXEN                    RXEN0
#elif defined(USART_1)
	#define power_serial_enable()   power_usart1_enable();
	#define power_serial_disable()  power_usart1_disable();
	#define SerialX					Serial1
	#define dbg						SerialX
	#define UCSR                    UCSR1B
	#define RXEN                    RXEN1
#else
	#error "please use USART_0 or USART_1"
#endif
//- -------------------------------------------------------------------------------------------------------------------------


//- timer definitions -----------------------------------------------------------------------------------------------------
#define hasTimer0																			// timer0 should be enough, while timer1 and 3 are for PWM mode
#define hasTimer1
#define hasTimer2
extern volatile uint8_t timer;																// here we store the active timer

static void init_millis_timer0(int16_t correct_ms) {
	timer = 0;
	power_timer0_enable();

	TCCR0A = _BV(WGM01);																	// CTC mode
	TCCR0B = (_BV(CS01) | _BV(CS00));														// prescaler 64; 8.000.000 / 64 = 125.000 / 1000 = 125 
	TIMSK0 = _BV(OCIE0A);
	OCR0A = ((F_CPU / 64) / 1000) + correct_ms;
}
static void init_millis_timer1(int16_t correct_ms) {
	timer = 1;
	power_timer1_enable();

	TCCR1A = 0;
	TCCR1B = (_BV(WGM12) | _BV(CS10) | _BV(CS11));
	TIMSK1 = _BV(OCIE1A);
	OCR1A = ((F_CPU / 64) / 1000) + correct_ms;
}
static void init_millis_timer2(int16_t correct_ms) {
	timer = 2;
	power_timer2_enable();

	TCCR2A = _BV(WGM21);
	TCCR2B = (_BV(CS21) | _BV(CS20));
	TIMSK2 = _BV(OCIE2A);
	OCR2A = ((F_CPU / 32) / 1000) + correct_ms;
}
//- -------------------------------------------------------------------------------------------------------------------------


//- power management definitions --------------------------------------------------------------------------------------------
#define backupPwrRegs()         uint8_t xPrr = PRR0; PRR0 = 0xFF;
#define recoverPwrRegs()        PRR0 = xPrr;
#define offBrownOut()           MCUCR = (1<<BODS)|(1<<BODSE); MCUCR = (1<<BODS);
//- -------------------------------------------------------------------------------------------------------------------------


//- adc definitions ---------------------------------------------------------------------------------------------------------
const uint8_t admux_internal = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);					// internal ADMUX register setup
const uint8_t admux_external = _BV(REFS1) | _BV(REFS0);											// | measurement pin
const uint16_t ref_v_external = 1100;															// internal reference voltage in 10mv
//- -------------------------------------------------------------------------------------------------------------------------


//- pin definition ----------------------------------------------------------------------------------------------------------
#define pc_interrupt_vectors 3																// amount of pin change interrupt vectors

#define pinB0 (0)		// &DDRB, &PORTB, &PINB,  0,  PCINT0, &PCICR, &PCMSK0, PCIE0, 0
#define pinB1 (1)
#define pinB2 (2)
#define pinB3 (3)
#define pinB4 (4)
#define pinB5 (5)
#define pinB6 (6)
#define pinB7 (7)

#define pinD0 (8)		// &DDRD, &PORTD, &PIND, 16, PCINT16, &PCICR, &PCMSK2, PCIE2, 2
#define pinD1 (9)
#define pinD2 (10)
#define pinD3 (11)
#define pinD4 (12)
#define pinD5 (13)
#define pinD6 (14)
#define pinD7 (15)

#define pinC0 (16)
#define pinC1 (17)
#define pinC2 (18)
#define pinC3 (19)
#define pinC4 (20) 
#define pinC5 (21)
#define pinC6 (22)
#define pinC7 (23)

#define pinA0 (24)
#define pinA1 (25)
#define pinA2 (26)
#define pinA3 (27)
#define pinA4 (28)
#define pinA5 (29)
#define pinA6 (30)
#define pinA7 (31)
//- -------------------------------------------------------------------------------------------------------------------------


#endif