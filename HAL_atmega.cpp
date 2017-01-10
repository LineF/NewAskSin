#include "HAL_atmega.h"

//-- pin functions --------------------------------------------------------------------------------------------------------
void set_pin_output(const s_pin_def *ptr_pin) {
	*ptr_pin->DDREG |= _BV(ptr_pin->PINBIT);
}

void set_pin_input(const s_pin_def *ptr_pin) {
	*ptr_pin->DDREG &= ~_BV(ptr_pin->PINBIT);
}

void set_pin_high(const s_pin_def *ptr_pin) {
	*ptr_pin->PORTREG |= _BV(ptr_pin->PINBIT);
}

void set_pin_low(const s_pin_def *ptr_pin) {
	*ptr_pin->PORTREG &= ~_BV(ptr_pin->PINBIT);
}

void set_pin_toogle(const s_pin_def *ptr_pin) {
	*ptr_pin->PORTREG ^= _BV(ptr_pin->PINBIT);
}

uint8_t get_pin_status(const s_pin_def *ptr_pin) {
	return *ptr_pin->PINREG & _BV(ptr_pin->PINBIT);
}
//- -----------------------------------------------------------------------------------------------------------------------



//-- interrupt functions --------------------------------------------------------------------------------------------------
struct  s_pcint_vector {
	volatile uint8_t *PINREG;
	uint8_t curr;
	uint8_t prev;
	uint8_t mask;
	uint32_t time;
};
volatile s_pcint_vector pcint_vector[PCINT_PCIE_SIZE];

void register_PCINT(const s_pin_def *ptr_pin) {
	set_pin_input(ptr_pin);																	// set the pin as input
	set_pin_high(ptr_pin);																	// key is connected against ground, set it high to detect changes

	pcint_vector[ptr_pin->VEC].PINREG = ptr_pin->PINREG;									// set the vector struct
	pcint_vector[ptr_pin->VEC].curr |= get_pin_status(ptr_pin);
	pcint_vector[ptr_pin->VEC].prev = pcint_vector[ptr_pin->VEC].curr;
	pcint_vector[ptr_pin->VEC].mask |= _BV(ptr_pin->PINBIT);

	*ptr_pin->PCICREG |= _BV(ptr_pin->PCIEREG);												// pci functions
	*ptr_pin->PCIMASK |= _BV(ptr_pin->PCIBYTE);												// make the pci active
}

uint8_t check_PCINT(const s_pin_def *ptr_pin, uint8_t debounce) {
	
	uint8_t status = pcint_vector[ptr_pin->VEC].curr & _BV(ptr_pin->PINBIT) ? 1 : 0;		// evaluate the pin status
	uint8_t prev = pcint_vector[ptr_pin->VEC].prev & _BV(ptr_pin->PINBIT) ? 1 : 0;			// evaluate the previous pin status

	if (status == prev) return status;														// check if something had changed since last time
	if (debounce && ((get_millis() - pcint_vector[ptr_pin->VEC].time) < DEBOUNCE)) return status;	// seems there is a change, check if debounce is necassary and done

	pcint_vector[ptr_pin->VEC].prev ^= _BV(ptr_pin->PINBIT);								// if we are here, there was a change and debounce check was passed, remember for next time

	if (status) return 3;																	// pin is 1, old was 0
	else return 2;																			// pin is 0, old was 1
}

void(*pci_ptr)(uint8_t vec, uint8_t pin, uint8_t flag) = NULL;

void maintain_PCINT(uint8_t vec) {
	pcint_vector[vec].curr = *pcint_vector[vec].PINREG & pcint_vector[vec].mask;			// read the pin port and mask out only pins registered
	pcint_vector[vec].time = get_millis();													// store the time, if debounce is asked for

	if (pci_ptr) {
		uint8_t pin_int = pcint_vector[vec].curr ^ pcint_vector[vec].prev;					// evaluate the pin which raised the interrupt
		pci_ptr(vec, pin_int, pcint_vector[vec].curr & pin_int);							// callback the interrupt function in user sketch
	}
}

#if PCINT_PCIE_SIZE > 0
ISR(PCINT0_vect) {
	maintain_PCINT(0);
}
#endif

#if PCINT_PCIE_SIZE > 1
ISR(PCINT1_vect) {
	maintain_PCINT(1);
}
#endif

#if PCINT_PCIE_SIZE > 2
ISR(PCINT2_vect) {
	maintain_PCINT(2);
}
#endif

#if PCINT_PCIE_SIZE > 3
ISR(PCINT3_vect) {
	maintain_PCINT(3);
}
#endif
//- -----------------------------------------------------------------------------------------------------------------------



//-- spi functions --------------------------------------------------------------------------------------------------------
void enable_spi(void) {
	// SPI enable, master, speed = CLK/4
	SPCR = _BV(SPE) | _BV(MSTR);
}

uint8_t spi_send_byte(uint8_t send_byte) {
	SPDR = send_byte;																		// send byte
	while (!(SPSR & _BV(SPIF))); 															// wait until transfer finished
	return SPDR;																			// return the data register
}
//- -----------------------------------------------------------------------------------------------------------------------


//-- eeprom functions -----------------------------------------------------------------------------------------------------
void init_eeprom(void) {
	// place the code to init a i2c eeprom
}

void get_eeprom(uint16_t addr, uint8_t len, void *ptr) {
	eeprom_read_block((void*)ptr, (const void*)addr, len);									// AVR GCC standard function
}

void set_eeprom(uint16_t addr, uint8_t len, void *ptr) {
	eeprom_write_block((const void*)ptr, (void*)addr, len);									// AVR GCC standard function
}

void clear_eeprom(uint16_t addr, uint16_t len) {
	uint8_t tB = 0;
	if (!len) return;
	for (uint16_t l = 0; l < len; l++) {													// step through the bytes of eeprom
		set_eeprom(addr + l, 1, (void*)&tB);
	}
}
//- -----------------------------------------------------------------------------------------------------------------------


//-- timer functions ------------------------------------------------------------------------------------------------------
static volatile uint32_t milliseconds;
#ifndef F_CPU
#error "F_CPU not defined!"
#endif

#ifdef TIMER0
#define REG_TCRA        ( TCCR0A = _BV(WGM01) )
#define REG_TCRB        ( TCCR0B = (_BV(CS01) | _BV(CS00)) )
#define REG_OCRA        ( OCR0A = ((F_CPU / 64) / 1000) )
#define POW_ENABLE      power_timer0_enable()
#define REG_TMSK        ( TIMSK0 = _BV(OCIE0A) )
#define ISR_VECT		TIMER0_COMPA_vect
#endif

#ifdef TIMER1
#define REG_TCRA        ( TCCR1A = 0 )
#define REG_TCRB        ( TCCR1B = (_BV(WGM12) | _BV(CS10) | _BV(CS11)) )
#define REG_OCRA        ( OCR1A = ((F_CPU / 64) / 1000) )
#define POW_ENABLE      power_timer1_enable()
#define REG_TMSK        ( TIMSK1 = _BV(OCIE1A) )
#define ISR_VECT		TIMER1_COMPA_vect
#endif

#ifdef TIMER2_Pn
#define REG_TCRA        ( TCCR2A = _BV(WGM21) )
#define REG_TCRB        ( TCCR2B = (_BV(CS21) | _BV(CS20)) )
#define REG_OCRA        ( OCR2A = ((F_CPU / 32) / 1000) )
#define POW_ENABLE      power_timer2_enable()
#define REG_TMSK        ( TIMSK2 = _BV(OCIE2A) )
#define ISR_VECT		TIMER2_COMPA_vect
#endif

#ifdef TIMER2_LOW_FREQ_OSC
extern void startTimer1ms();
extern uint32_t ocrCorrCnt;
extern uint16_t ocrSleep_TIME;
#define REG_TCRA        ( TCCR2A = _BV(WGM21) )
#define REG_TCRB        ( TCCR2B = (_BV(CS21) | _BV(CS20)) )
#define REG_OCRA        {}
#define POW_ENABLE      power_timer2_enable()
#define ASSR_ENABLE		(ASSR |= (1<<AS2), _delay_ms(1000))
#define REG_TMSK        ( TIMSK2 = _BV(OCIE2A) )
#define ISR_VECT		TIMER2_COMPA_vect
#define INIT_TIMER		{startTimer1ms(); {while (ASSR & _BV(TCR2AUB)) ;} }
#define PRESC_32		(_BV(CS21)|_BV(CS20))
#define PRESC_1024		(_BV(CS22)|_BV(CS21)|_BV(CS20))
#define FREQ_CORR_FACT	234375L
#define FREQ_MAX_CORR	10000000L
#endif

void init_millis(void) {														
	POW_ENABLE;
	ASSR_ENABLE;
	REG_TCRA;
	REG_TCRB;
	REG_TMSK;
	INIT_TIMER;
	REG_OCRA;
}

uint32_t get_millis(void) {
	uint32_t ms;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		ms = milliseconds;
	}
	return ms;
}

void add_millis(uint32_t ms) {
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		milliseconds += ms;
	}
}

ISR(ISR_VECT) {
#ifdef TIMER2_LOW_FREQ_OSC
	static uint32_t freq_corr;
	freq_corr += ocrCorrCnt;
	if (freq_corr >= FREQ_MAX_CORR)
		freq_corr -= FREQ_MAX_CORR;
	else
		milliseconds += ocrSleep_TIME;
#else
	milliseconds += 1;
#endif
}
//- -----------------------------------------------------------------------------------------------------------------------


//-- battery measurement functions ----------------------------------------------------------------------------------------
uint8_t get_internal_voltage(void) {
	uint16_t result = get_adc_value(admux_internal);										// get the adc value on base of the predefined adc register setup
	result = 11253L / result;																// calculate Vcc (in mV); 11253 = 1.1*1023*10 (*10 while we want to get 10mv)
	return (uint8_t)result;																	// Vcc in millivolts
}

uint8_t get_external_voltage(const s_pin_def *ptr_enable, const s_pin_def *ptr_measure, uint8_t z1, uint8_t z2) {
	/* set the pins to enable measurement */
	set_pin_output(ptr_enable);																// set the enable pin as output
	set_pin_low(ptr_enable);																// and to gnd, while measurement goes from VCC over the resistor network to GND
	set_pin_input(ptr_measure);																// set the ADC pin as input
	//set_pin_high(ptr_measure);

	/* call the adc get function to get the adc value, do some mathematics on the result */
	uint16_t result = get_adc_value(admux_external | ptr_measure->PINBIT);					// get the adc value on base of the predefined adc register setup
	result = ((result * ref_v_external) / 103) / z1;										// calculate vcc between gnd and measurement pin 
	result = result * (z1 + z2) / 100;														// interpolate result to vcc 

	/* finally, we set both pins as input again to waste no energy over the resistor network to VCC */
	set_pin_input(ptr_enable);	
	set_pin_input(ptr_measure);	

	return result;																			// Vcc in millivolts
}

uint16_t get_adc_value(uint8_t reg_admux) {
	// save power reduction register
	
	/* enable and set adc */
	power_adc_enable();																		// start adc 

	ADMUX = reg_admux;																		// set adc
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);										// enable ADC and set ADC pre scaler

	/* measure the adc */
	_delay_ms(2);																			// wait for Vref to settle

	ADCSRA |= _BV(ADSC);																	// start conversion
	while (bit_is_set(ADCSRA, ADSC));														// measuring

	//uint8_t low = ADCL;																	// must read ADCL first - it then locks ADCH  
	//uint8_t high = ADCH;																	// unlocks both
	//uint16_t result = (high << 8) | low;
	uint16_t result = ADCW;																	// read register once, alternative above 

	/* restore power reduction register and stop adc measurement */
	power_adc_disable();																	// stop adc
	return result;
}