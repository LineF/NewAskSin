//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- Hardware abstraction layer --------------------------------------------------------------------------------------------
//- -----------------------------------------------------------------------------------------------------------------------


#include <avr/boot.h>
#include "HAL.h"
#include "00_debug-flag.h"




//- randum number functions -----------------------------------------------------------------------------------------------
void get_random(uint8_t *buf) {
	/* not random, but most likely, as real random takes 200 byte more */

	uint32_t time = get_millis();												// get current time 
	memcpy(buf, (uint8_t*)&time, 4);											// copy the time into the array
	memcpy(buf + 2, (uint8_t*)&time, 4);
	for (uint8_t i = 0; i < 6; i++) {											// do some xors and byte shift
		if (i) buf[i] ^= buf[i - 1];
		else buf[0] ^= 0x35;
		if (buf[i] & 1) buf[i] = (buf[i] >> 1) ^ 0xA0;
		else buf[i] = (buf[i] >> 1);
	}
}
//- -----------------------------------------------------------------------------------------------------------------------



// todo: customize baudrate
// remove mcu dependencies

//- -----------------------------------------------------------------------------------------------------------------------


//- power management functions --------------------------------------------------------------------------------------------
// http://donalmorrissey.blogspot.de/2010/04/sleeping-arduino-part-5-wake-up-via.html
// http://www.mikrocontroller.net/articles/Sleep_Mode#Idle_Mode

#ifdef LOW_FREQ_OSC
//static volatile uint8_t wdt_int;
uint32_t ocrCorrCnt;
uint16_t ocrSleep_TIME;															// uint16 is enough - 32 bit here not needed

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

static uint16_t wdtSleep_TIME;
static volatile uint8_t wdt_int;
uint16_t wdt_cal_ms;															// uint16 is enough - 32 bit here not needed

void    startTimer32ms(void) {
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = (1<<WDIE) | (1<<WDP0);
	wdtSleep_TIME = wdt_cal_ms / 8;
}
void    startTimer64ms(void) {
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = (1<<WDIE) | (1<<WDP1);
	wdtSleep_TIME = wdt_cal_ms / 4;
}
void    startTimer250ms(void) {
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = (1<<WDIE) | (1<<WDP2);
	wdtSleep_TIME = wdt_cal_ms;
}
void    startTimer8000ms(void) {
	WDTCSR |= (1<<WDCE) | (1<<WDE);
	WDTCSR = (1<<WDIE) | (1<<WDP3) | (1<<WDP0);
	wdtSleep_TIME = wdt_cal_ms * 32;
}

void	calibrateWatchdog() {													// initMillis() must have been called yet
	uint8_t sreg = SREG;														// remember interrupt state (sei / cli)
	wdt_cal_ms = 0;
	startTimer250ms();

	uint16_t startMillis = getMillis();
	wdt_int = 0;
	wdt_reset();
	sei();
	
	while(!wdt_int)																// wait for watchdog interrupt
		;
	SREG = sreg;																// restore previous interrupt state
	wdt_cal_ms = getMillis() - startMillis;										// wdt_cal_ms now has "real" length of 250ms wdt_interrupt
	stopWDG();
	DBG(SER, F("wdt_cal: "), wdt_cal_ms, F("\n"));
}
void    startWDG() {
	WDTCSR = (1<<WDIE);
}
void    stopWDG() {
	WDTCSR &= ~(1<<WDIE);
}
void    setSleepMode() {
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
}

ISR(WDT_vect) {
	// nothing to do, only for waking up
	add_millis(wdtSleep_TIME);
	wdt_int = 1;
}
#endif
void    setSleep(void) {
	// some power savings by switching off some CPU functionality
	ADCSRA = 0;																	// disable ADC
	DIDR1 |= _BV(AIN0D) | _BV(AIN1D);											// switch off analog comparator input buffers
	DIDR0 |= _BV(ADC0D) | _BV(ADC1D) | _BV(ADC2D) | _BV(ADC3D)
		   | _BV(ADC4D) | _BV(ADC5D);											// switch off adc input buffers
	backupPwrRegs();															// save content of power reduction register and set it to all off

	sleep_enable();																// enable sleep
	offBrownOut();																// turn off brown out detection

	sleep_cpu();																// goto sleep
	// sleeping now
	// --------------------------------------------------------------------------------------------------------------------
	// wakeup will be here
	sleep_disable();															// first thing after waking from sleep, disable sleep...
	recoverPwrRegs();															// recover the power reduction register settings
}
//- -----------------------------------------------------------------------------------------------------------------------


//- timer functions -------------------------------------------------------------------------------------------------------
//static volatile tMillis milliseconds;
//#ifdef LOW_FREQ_OSC
//static volatile uint32_t freq_corr;
//#endif
//void    initMillis() {
//	SET_TCCRA();
//	SET_TCCRB();
//	REG_TIMSK = _BV(BIT_OCIE);
//#ifdef LOW_FREQ_OSC
//	startTimer1ms();
//	while (ASSR & _BV(TCR2AUB))
//		;
//#else
//	REG_OCR = ((F_CPU / PRESCALER) / 1000) - 1;												// as of atmel docu: ocr should be one less than divider
//#endif
//}
//tMillis getMillis() {
//	tMillis ms;
//	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//		ms = milliseconds;
//	}
//	return ms;
//}
//void    addMillis(tMillis ms) {
//	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
//		milliseconds += ms;
//	}
//}
//ISR(ISR_VECT) {
//#ifdef LOW_FREQ_OSC
//	freq_corr += ocrCorrCnt;
//	if (freq_corr >= FREQ_MAX_CORR)
//		freq_corr -= FREQ_MAX_CORR;
//	else
//		milliseconds += ocrSleep_TIME;
//#else
//	milliseconds += 1;
//	//setPinCng(LED_RED_PORT, LED_RED_PIN);													// for generating a 1 KHz signal on LED pin to calibrate CPU
//#endif
//}
//- -----------------------------------------------------------------------------------------------------------------------




//- battery measurement functions -----------------------------------------------------------------------------------------
uint16_t getAdcValue(uint8_t adcmux) {
	uint16_t adcValue = 0;

	#if defined(__AVR_ATmega32U4__)												// save content of Power Reduction Register
		uint8_t tmpPRR0 = PRR0;
		uint8_t tmpPRR1 = PRR1;
	#else
		uint8_t tmpPRR = PRR;
	#endif
	power_adc_enable();

	ADMUX = adcmux;																// start ADC
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);							// Enable ADC and set ADC pre scaler

	for (uint8_t i = 0; i < BAT_NUM_MESS_ADC + BAT_DUMMY_NUM_MESS_ADC; i++) {	// take samples in a round
		ADCSRA |= (1 << ADSC);													// start conversion
		while (ADCSRA & (1 << ADSC)) {}											// wait for conversion complete

		if (i >= BAT_DUMMY_NUM_MESS_ADC) {										// we discard the first dummy measurements
			adcValue += ADCW;
		}
	}

	ADCSRA &= ~(1 << ADEN);														// ADC disable
	adcValue = adcValue / BAT_NUM_MESS_ADC;										// divide adcValue by amount of measurements

	#if defined(__AVR_ATmega32U4__)												// restore power management
		PRR0 = tmpPRR0;
		PRR1 = tmpPRR1;
	#else
		PRR = tmpPRR;
	#endif

	ADCSRA = 0;																	// ADC off
	power_adc_disable();

	//dbg << "x:" << adcValue << '\n';

	return adcValue;															// return the measured value
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


//- -----------------------------------------------------------------------------------------------------------------------
