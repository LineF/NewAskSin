#include "HAL_atmega.h"
#include "as_power.h"
#include "as_status_led.h"

extern POM *pom;
extern LED *led;
extern uint8_t ledFreqTest;

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

void set_pin_toggle(const s_pin_def *ptr_pin) {
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

void(*pci_ptr)(uint8_t vec, uint8_t pin, uint8_t flag) = NULL;								// call back function pointer

void maintain_PCINT(uint8_t vec) {
	pcint_vector[vec].curr = *pcint_vector[vec].PINREG & pcint_vector[vec].mask;			// read the pin port and mask out only pins registered
	pcint_vector[vec].time = get_millis();													// store the time, if debounce is asked for
	pom->stayAwake(DEBOUNCE+1);

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
	power_spi_enable();																		// enable only needed functions
	SPCR = _BV(SPE) | _BV(MSTR);															// SPI enable, master, speed = CLK/4
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
	/* update is much faster, while writes only when needed; needs some byte more space
	* but otherwise we run in timing issues */
	eeprom_update_block((const void*)ptr, (void*)addr, len);								// AVR GCC standard function
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
static volatile uint8_t timer = 255;
#ifdef TIMER2_LOW_FREQ_OSC
static volatile uint32_t freq_corr;
uint32_t ocrCorrCnt;
uint16_t ocrSleep_TIME;															// uint16 is enough - 32 bit here not needed
#endif

#ifdef hasTimer0
void init_millis_timer0(int16_t correct_ms) {
	timer = 0;
	power_timer0_enable();

	TCCR0A = _BV(WGM01);
	TCCR0B = (_BV(CS01) | _BV(CS00));
	TIMSK0 = _BV(OCIE0A);
	OCR0A = ((F_CPU / 64) / 1000) - 1 + correct_ms;
}
#endif

#ifdef hasTimer1
void init_millis_timer1(int16_t correct_ms) {
	timer = 1;
	power_timer1_enable();

	TCCR1A = 0;
	TCCR1B = (_BV(WGM12) | _BV(CS10) | _BV(CS11));
	TIMSK1 = _BV(OCIE1A);
	OCR1A = ((F_CPU / 64) / 1000) - 1 + correct_ms;
}
#endif

#ifdef hasTimer2
void init_millis_timer2(int16_t correct_ms) {
	timer = 2;
	power_timer2_enable();
	#ifdef TIMER2_LOW_FREQ_OSC
		ASSR |= (1<<AS2);																	// enable async Timer/Counter 2
		_delay_ms(1000);																	// wait for clean osc startup
	#endif

	TCCR2A = _BV(WGM21);
	TCCR2B = (_BV(CS21) | _BV(CS20));
	TIMSK2 = _BV(OCIE2A);
	#ifdef TIMER2_LOW_FREQ_OSC
		startTimer1ms();
		while (ASSR & _BV(TCR2AUB))
			;
	#else
		OCR2A = ((F_CPU / 32) / 1000) - 1 + correct_ms;
	#endif
}
#endif

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

ISR(TIMER0_COMPA_vect) {
	if (timer == 0) ++milliseconds;
}
ISR(TIMER1_COMPA_vect) {
	if (timer == 1) ++milliseconds;
}
ISR(TIMER2_COMPA_vect) {
	#ifdef TIMER2_LOW_FREQ_OSC
		freq_corr += ocrCorrCnt;
		if (freq_corr >= FREQ_MAX_CORR)
			freq_corr -= FREQ_MAX_CORR;
		else
			milliseconds += ocrSleep_TIME;
	#else
		if (timer == 2) ++milliseconds;
		//setPinCng(LED_RED_PORT, LED_RED_PIN);													// for generating a 1 KHz signal on LED pin to calibrate CPU
	#endif
	if (ledFreqTest)
		set_pin_toggle(led->pin_red);
}
//- -----------------------------------------------------------------------------------------------------------------------


//-- battery measurement functions ----------------------------------------------------------------------------------------
uint16_t get_internal_voltage(void) {
	uint16_t result = get_adc_value(admux_internal);										// get the adc value on base of the predefined adc register setup
	result = 11253L / result;																// calculate Vcc (in mV); 11253 = 1.1*1023*10 (*10 while we want to get 10mv)
	return (uint8_t)result;																	// Vcc in millivolts
}

uint16_t get_external_voltage(const s_pin_def *ptr_enable, const s_pin_def *ptr_measure, uint8_t z1, uint8_t z2) {
	#define BAT_OVERSAMPLING 12
	static uint16_t values[BAT_OVERSAMPLING];
	static uint8_t v_idx;
	uint8_t cnt = 0;
	uint32_t result = 0;

	/* set the pins to enable measurement */
	set_pin_output(ptr_enable);																// set the enable pin as output
	set_pin_low(ptr_enable);																// and to gnd, while measurement goes from VCC over the resistor network to GND
	set_pin_input(ptr_measure);																// set the ADC pin as input
	set_pin_low(ptr_measure);																// switch off pull-up resistor to get correct measurement

	/* call the adc get function to get the adc value, do some mathematics on the result */
	values[v_idx++] = get_adc_value(admux_external | ptr_measure->PINBIT);					// get the adc value on base of the predefined adc register setup
	if (v_idx >= BAT_OVERSAMPLING) v_idx = 0;

	for (uint8_t i = 0; i < BAT_OVERSAMPLING; i++)
		if (values[i] > 0)
			result += values[i], cnt++;
	result = (cnt > 0) ? result / cnt : 0;
	
	result = ((result * ref_v_external) / 102) / z1;										// calculate vcc between gnd and measurement pin 
	result = result * (z1 + z2) / 100;														// interpolate result to vcc 

	/* finally, we set both pins as input again to waste no energy over the resistor network to VCC */
	set_pin_input(ptr_enable);	
	set_pin_input(ptr_measure);	

	return result;																			// Vcc in millivolts
}

uint16_t get_adc_value(uint8_t reg_admux) {
	uint16_t adcValue = 0;
	/* enable and set adc */
	/* enable and set adc */
	power_adc_enable();																		// start adc 

	ADMUX = reg_admux;																		// set adc
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);										// enable ADC and set ADC pre scaler
	_delay_ms(2);

	/* measure the adc */
	for (uint8_t i = 0; i < BAT_NUM_MESS_ADC; i++) {										// take samples in a round
		ADCSRA |= (1 << ADSC);																// start conversion
		while (ADCSRA & (1 << ADSC))														// wait for conversion complete
			;
		adcValue += ADCW;
	}

	ADCSRA &= ~(1 << ADEN);																	// ADC disable
	adcValue = adcValue / BAT_NUM_MESS_ADC;													// divide adcValue by amount of measurements

	power_adc_disable();																	// stop adc
	return adcValue;
}
//- -----------------------------------------------------------------------------------------------------------------------


//-- power management functions --------------------------------------------------------------------------------------------
// http://donalmorrissey.blogspot.de/2010/04/sleeping-arduino-part-5-wake-up-via.html
// http://www.mikrocontroller.net/articles/Sleep_Mode#Idle_Mode
#ifdef TIMER2_LOW_FREQ_OSC
#define PRESC_32		(_BV(CS21)|_BV(CS20))
#define PRESC_1024		(_BV(CS22)|_BV(CS21)|_BV(CS20))

void writeOCR2A(uint8_t val) {
	OCR2A = val;
	while (ASSR & _BV(OCR2AUB))
		;
}
void writePRESC(uint8_t val) {
	TCCR2B = TCCR2B & ~(_BV(CS22)|_BV(CS21)|_BV(CS20)) | val;
	while (ASSR & _BV(TCR2BUB))
		;
}
void startTimer1ms(void) {
	writePRESC(PRESC_32);
	writeOCR2A(0);
	ocrSleep_TIME = 1;
	ocrCorrCnt = FREQ_CORR_FACT;
}
void startTimer32ms(void) {
	writePRESC(PRESC_32);
	writeOCR2A(31);
	ocrSleep_TIME = 32;
	ocrCorrCnt = 32 * FREQ_CORR_FACT;
}
void startTimer64ms(void) {
	writePRESC(PRESC_32);
	writeOCR2A(63);
	ocrSleep_TIME = 64;
	ocrCorrCnt = 64 * FREQ_CORR_FACT;
}
void startTimer250ms(void) {
	writePRESC(PRESC_32);
	writeOCR2A(255);
	ocrSleep_TIME = 250;
	ocrCorrCnt = 0;
}
void startTimer8000ms(void) {
	writePRESC(PRESC_1024);
	writeOCR2A(255);
	ocrSleep_TIME = 8000;
	ocrCorrCnt = 0;
}
void setSleepMode() {
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
}

#else

static volatile uint8_t wdt_int;
uint16_t wdt_cal_ms;															// uint16 is enough - 32 bit here not needed

void startTimer32ms(void) {
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDIE) | (1 << WDP0);
	wdtSleep_TIME = wdt_cal_ms / 8;
}
void startTimer64ms(void) {
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDIE) | (1 << WDP1);
	wdtSleep_TIME = wdt_cal_ms / 4;
}
void startTimer250ms(void) {
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDIE) | (1 << WDP2);
	wdtSleep_TIME = wdt_cal_ms;
}
void startTimer8000ms(void) {
	WDTCSR |= (1 << WDCE) | (1 << WDE);
	WDTCSR = (1 << WDIE) | (1 << WDP3) | (1 << WDP0);
	wdtSleep_TIME = wdt_cal_ms * 32;
}

void calibrateWatchdog() {														// initMillis() must have been called yet
	uint8_t sreg = SREG;														// remember interrupt state (sei / cli)
	wdt_cal_ms = 0;
	startTimer250ms();

	uint16_t startMillis = get_millis();
	wdt_int = 0;
	wdt_reset();
	sei();
	
	while(!wdt_int)																// wait for watchdog interrupt
		;
	SREG = sreg;																// restore previous interrupt state
	wdt_cal_ms = get_millis() - startMillis;										// wdt_cal_ms now has "real" length of 250ms wdt_interrupt
	stopWDG();
	//DBG_SER(F("wdt_cal: "), wdt_cal_ms, F("\n"));
}

void    startWDG() {
	WDTCSR = (1 << WDIE);
}
void    stopWDG() {
	WDTCSR &= ~(1 << WDIE);
}
void    setSleepMode() {
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

ISR(WDT_vect) {
	add_millis(wdtSleep_TIME);																// nothing to do, only for waking up
	wdt_int = 1;
}
#endif


void setSleep(void) {
	// some power savings by switching off some CPU functionality
	ADCSRA = 0;																				// disable ADC
	DIDR1 |= _BV(AIN0D) | _BV(AIN1D);														// switch off analog comparator input buffers
	DIDR0 |= _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D) | _BV(ADC3D)
			| _BV(ADC4D) | _BV(ADC5D);														// switch off adc input buffers
	backupPwrRegs();																		// save content of power reduction register and set it to all off

	sleep_enable();																			// enable sleep
	offBrownOut();																			// turn off brown out detection

	sleep_cpu();																			// goto sleep
	// sleeping now
	// --------------------------------------------------------------------------------------------------------------------
	// wakeup will be here
	sleep_disable();																		// first thing after waking from sleep, disable sleep...
	recoverPwrRegs();																		// recover the power reduction register settings
	//dbg << '.';																			// some debug
}

//- -----------------------------------------------------------------------------------------------------------------------

// read factory defined OSCCAL value from signature row (address 0x0001)
uint8_t getDefaultOSCCAL(void)
{
	uint8_t oscCal;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		oscCal = boot_signature_byte_get(0x0001);
	}
	return oscCal;
}

//- -----------------------------------------------------------------------------------------------------------------------
