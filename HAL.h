//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- Hardware abstraction layer --------------------------------------------------------------------------------------------
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _HAL_H
#define _HAL_H

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <stdint.h>
//#include <avr/common.h>

#include "hardware.h"

#include "macros.h"


//- MCU dependent HAL definitions -----------------------------------------------------------------------------------------
#if defined(__AVR__)
	#include "HAL_atmega.h"
#else
	#error "No HAL definition for current MCU available!"
#endif




/*-- pin functions --------------------------------------------------------------------------------------------------------
* all pins defined as a struct, holding all information regarding port, pin, ddr, etc.
* as we support different arduino hw i tried to make it as flexible as possible. everything is defined in seperate 
* hw specific files. the struct and pin manipulation function is defined in HAL_atmega.h because it is similar for all
* ATMEL hardware, the pin structs are defined in HAL_atmega_<model> while different for each cpu type. here we reference
* only on the functions defined in HAL_<type>_<model>.
*/
extern void set_pin_output(const s_pin_def *ptr_pin);
extern void set_pin_input(const s_pin_def *ptr_pin);
extern void set_pin_high(const s_pin_def *ptr_pin);
extern void set_pin_low(const s_pin_def *ptr_pin);
extern void set_pin_toogle(const s_pin_def *ptr_pin);
extern uint8_t get_pin_status(const s_pin_def *ptr_pin);
//- -----------------------------------------------------------------------------------------------------------------------


/*-- interrupt functions --------------------------------------------------------------------------------------------------
* interrupts again are very hardware supplier related, therefor we define her some external functions which needs to be 
* defined in the hardware specific HAL file. for ATMEL it is defined in HAL_atmega.h.
* you can also use the arduino standard timer for a specific hardware by interlinking the function call to getmillis()
*/
extern void register_PCINT(const s_pin_def *ptr_pin);
extern uint8_t check_PCINT(const s_pin_def *ptr_pin);
extern uint8_t check_PCINT(uint8_t port, uint8_t pin, uint8_t debounce);
extern void(*pci_ptr)(uint8_t vec, uint8_t pin, uint8_t flag);
extern void maintain_PCINT(uint8_t vec);
//- -----------------------------------------------------------------------------------------------------------------------


/*-- spi functions --------------------------------------------------------------------------------------------------------
* spi is very hardware supplier related, therefor we define her some external functions which needs to be defined
* in the hardware specific HAL file. for ATMEL it is defined in HAL_atmega.h.
*/
extern void enable_spi(void);
extern uint8_t spi_send_byte(uint8_t send_byte);


/*-- eeprom functions -----------------------------------------------------------------------------------------------------
* eeprom is very hardware supplier related, therefor we define her some external functions which needs to be defined
* in the hardware specific HAL file. for ATMEL it is defined in HAL_atmega.h.
*/
extern void init_eeprom(void);
extern void get_eeprom(uint16_t addr, uint8_t len, void *ptr);
extern void set_eeprom(uint16_t addr, uint8_t len, void *ptr);
extern void clear_eeprom(uint16_t addr, uint16_t len);
//- -----------------------------------------------------------------------------------------------------------------------


/*-- timer functions ------------------------------------------------------------------------------------------------------
* timer is very hardware supplier related, therefor we define her some external functions which needs to be defined
* in the hardware specific HAL file. for ATMEL it is defined in HAL_atmega.h.
* you can also use the arduino standard timer for a specific hardware by interlinking the function call to getmillis()
*/
extern void init_millis(void);
extern uint32_t get_millis(void);
extern void add_millis(uint32_t ms);
//- -----------------------------------------------------------------------------------------------------------------------





/*-- serial print functions -----------------------------------------------------------------------------------------------
* template and some functions for debugging over serial interface
* based on arduino serial class, so should work with all hardware served in arduino
* http://aeroquad.googlecode.com/svn/branches/pyjamasam/WIFIReceiver/Streaming.h
*/
#define dbg Serial
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }

#define _HI_HEX_BITS(x)  char((x>>4)>9?(x>>4)+55:(x>>4)+48)
#define _LO_HEX_BITS(x)  char((x&0xF)>9?(x&0xF)+55:(x&0xF)+48)

struct _HEXB {
	uint8_t val;
	_HEXB(uint8_t v) : val(v) {}
};
inline Print &operator <<(Print &obj, const _HEXB &arg) { obj.print(_HI_HEX_BITS(arg.val)); obj.print(_LO_HEX_BITS(arg.val)); return obj; }

struct _HEX {
	uint8_t *val;
	uint8_t len;
	_HEX(uint8_t *v, uint8_t l) : val(v), len(l) {}
};
inline Print &operator <<(Print &obj, const _HEX &arg) { for (uint8_t i = 0; i<arg.len; i++) { obj.print(_HI_HEX_BITS(arg.val[i])); obj.print(_LO_HEX_BITS(arg.val[i])); if (i <= arg.len) obj.print(' '); }; return obj; }

enum _eTIME { _TIME };
inline Print &operator <<(Print &obj, _eTIME arg) { obj.print('('); obj.print(get_millis()); obj.print(')'); return obj; }
//- -----------------------------------------------------------------------------------------------------------------------



//- randum number functions -----------------------------------------------------------------------------------------------
void get_random(uint8_t *buf);
//- -----------------------------------------------------------------------------------------------------------------------



	//static uint16_t wdtSleep_TIME;

	//- timer functions -------------------------------------------------------------------------------------------------------
	// https://github.com/zkemble/millis/blob/master/millis/
	#ifdef LOW_FREQ_OSC
		#define REG_TCCRA		TCCR2A
		#define REG_TCCRB		TCCR2B
		#define REG_TIMSK		TIMSK2
		#define REG_OCR			OCR2A
		#define BIT_OCIE		OCIE2A
		#define BIT_WGM			WGM21
		#define PRESC_32		(_BV(CS21)|_BV(CS20))
		#define PRESC_1024		(_BV(CS22)|_BV(CS21)|_BV(CS20))
		#define CLOCKSEL        PRESC_32
		#define ISR_VECT		TIMER2_COMPA_vect
		#define FREQ_CORR_FACT	234375L
		#define FREQ_MAX_CORR	10000000L
	#else
		#define REG_TCCRA		TCCR0A
		#define REG_TCCRB		TCCR0B
		#define REG_TIMSK		TIMSK0
		#define REG_OCR			OCR0A
		#define BIT_OCIE		OCIE0A
		#define BIT_WGM			WGM01
		#define CLOCKSEL        (_BV(CS01)|_BV(CS00))
		#define PRESCALER       64
		#define ISR_VECT		TIMER0_COMPA_vect
	#endif




	//- needed for 32u4 to prevent sleep, while USB didn't work in sleep ------------------------------------------------------
	extern void    initWakeupPin(void);															// init the wakeup pin
	extern uint8_t checkWakeupPin(void);														// we can setup a pin which avoid sleep mode
	//- -----------------------------------------------------------------------------------------------------------------------








	//- power management functions --------------------------------------------------------------------------------------------
	extern void    startTimer1ms(void);
	extern void    startTimer32ms(void);
	extern void    startTimer64ms(void);
	extern void    startTimer250ms(void);
	extern void    startTimer8000ms(void);
	extern void    setSleep(void);
	extern void    setSleepMode();

	#ifdef LOW_FREQ_OSC
	#else
	extern uint16_t wdt_cal_ms;

	extern void    calibrateWatchdog();
	extern void    startWDG();
	extern void    stopWDG();
	#endif
	//- -----------------------------------------------------------------------------------------------------------------------


	//- battery measurement functions -----------------------------------------------------------------------------------------
	// http://jeelabs.org/2013/05/17/zero-powe-battery-measurement/
	#define BAT_NUM_MESS_ADC                  20								// real measures to get the best average measure
	#define BAT_DUMMY_NUM_MESS_ADC            40								// dummy measures to get the ADC working

	extern uint16_t getAdcValue(uint8_t adcmux);
	uint16_t  getBatteryVoltage(void);
	//- -----------------------------------------------------------------------------------------------------------------------


	//- system functions -------------------------------------------------------------------------------------------------
	extern uint8_t getDefaultOSCCAL(void);
	//- -----------------------------------------------------------------------------------------------------------------------


	//- main module functions -------------------------------------------------------------------------------------------------
	extern void cnl0Change(void);
	//- -----------------------------------------------------------------------------------------------------------------------

	//- randum number functions -----------------------------------------------------------------------------------------------
	//static uint16_t random_seed;
	//void init_random(void);
	//void get_random(uint8_t *buf);
	//inline void seed_random(void);
	//- -----------------------------------------------------------------------------------------------------------------------


#endif 
