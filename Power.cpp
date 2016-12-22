//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin power management function --------------------------------------------------------------------------------------
//- with a lot of support from martin876 at FHEM forum
//- -----------------------------------------------------------------------------------------------------------------------

//#define PW_DBG
#include "Power.h"
#include "AS.h"

PW pom;																						// declare power management, defined in Power.h
waitTimer pwrTmr;																			// power timer functionality

// private:		//---------------------------------------------------------------------------------------------------------

/**
* @brief Initialize the power module
*/
PW::PW() {
	pwrMode = POWER_MODE_NO_SLEEP;															// set default

//	stayAwake(5000);																		// startup means stay awake for next 5 seconds
}

/**
 * @brief Set power mode
 */
void PW::setMode(uint8_t mode) {
	pwrMode = mode;

	#ifdef PW_DBG																			// only if pw debug is set
	dbg << F("PowerMode: ") << pwrMode << '\n';											// ...and some information
	#endif

	initWakeupPin();
	setSleepMode();
}

/**
 * @brief Stay awake for specific time
 *
 * @param time in milliseconds for stay awake
 */
void PW::stayAwake(uint16_t time) {
	if (time < pwrTmr.remain()) return;														// set new timeout only if we have to add something
	pwrTmr.set(time);
}

/**
 * @brief Check against active flag of various modules
 */
void PW::poll(void) {
	
	if (pwrMode == POWER_MODE_NO_SLEEP) return;												// no power savings, there for we can exit
	if (!pwrTmr.done()) return;																// timer active, jump out
	if (checkWakeupPin()) return;															// wakeup pin active
	
	// some communication still active, jump out
	if ((snd_msg.active) || (list_msg.active) || (config_mode.active) || (pair_mode.active) || (cbn->button_check.armed) || (led.active)) return;
	
	#ifdef PW_DBG																			// only if pw debug is set
	dbg << '.';																				// ...and some information
	_delay_ms(1);
	//uint32_t fTme = getMillis();
	#endif


	if (pwrMode == POWER_MODE_WAKEUP_ONRADIO) {												// check communication on power mode 1

		tmpCCBurst = com->detect_burst();
		if ((tmpCCBurst) && (!chkCCBurst)) {												// burst detected for the first time
			chkCCBurst = 1;																	// set the flag
			
			#ifdef PW_DBG																	// only if pw debug is set
			dbg << '1';																		// ...and some information
			#endif

		} else if ((tmpCCBurst) && (chkCCBurst)) {											// burst detected for the second time
			chkCCBurst = 0;																	// reset the flag
			stayAwake(500);																	// stay awake for some time to check if we receive a valid message

			#ifdef PW_DBG																	// only if pw debug is set
			dbg << '2';																		// ...and some information
			#endif

			return;																			// we don't want proceed further, jump out
			
		} else if ((!tmpCCBurst) && (chkCCBurst)) {											// secondary test was negative, reset the flag
			chkCCBurst = 0;																	// reset the flag

			#ifdef PW_DBG																	// only if pw debug is set
			dbg << '-';																		// ...and some information
			#endif			
		}
	}

	// if we are here, we could go sleep. set cc module idle, switch off led's and sleep
	com->set_idle();																		// set communication module to idle
	led->set(LED_STAT::NONE);																// switch off all led's

	// start the respective watchdog timers
	cli();
	if      ((pwrMode == POWER_MODE_WAKEUP_ONRADIO) && (!chkCCBurst)) startTimer250ms();
	else if ((pwrMode == POWER_MODE_WAKEUP_ONRADIO) && (chkCCBurst))  startTimer32ms();
	else if  (pwrMode == POWER_MODE_WAKEUP_32MS)                      startTimer32ms();
//	else if  (pwrMode == POWER_MODE_WAKEUP_64MS)                      startTimer64ms();
	else if  (pwrMode == POWER_MODE_WAKEUP_250MS)                     startTimer250ms();
	else if  (pwrMode == POWER_MODE_WAKEUP_8000MS)                    startTimer8000ms();

	// todo: move sei() to setSleep() before sleep_cpu();
	sei();

	#if defined(PW_DBG)||defined(SER_DBG)||defined(SN_DBG)||defined(RV_DBG)||defined(EE_DBG)||defined(LD_DBG)||defined(CC_DBG)||defined(AS_DBG)||defined(AES_DBG)||defined(BT_DBG)||defined(TH_DBG)
	Serial.flush();																			// give UART some time to send last chars
	#endif

	setSleep();																				// call sleep function in HAL

	/*************************
	 * Wake up at this point *
	 *************************/
	#ifdef LOW_FREQ_OSC
		startTimer1ms();
	#else
	if (pwrMode != POWER_MODE_WAKEUP_EXT_INT) {
		stopWDG();																			// stop the watchdog
	}
	#endif

	#ifdef PW_DBG																			// only if pw debug is set
	dbg << ':';// << (getMillis() -fTme) << '\n';												// ...and some information
	#endif
		
}
