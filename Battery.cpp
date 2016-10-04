//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin battery status functions ---------------------------------------------------------------------------------------
//- with a lot of support from dirk at FHEM forum
//- -----------------------------------------------------------------------------------------------------------------------

//#define BT_DBG
#include "Battery.h"
#include "AS.h"

BT bat;																						// declare battery status, defined in Battery.h
waitTimer battTmr;																			// battery timer for duration check

// public:		//---------------------------------------------------------------------------------------------------------
void    BT::set(uint16_t centiVolt, uint32_t duration) {
	bDuration = duration;
	checkCentiVolt = centiVolt;
	poll();
}
uint16_t BT::getVolts(void) {
	return measureCentiVolt;
}
uint8_t BT::getStatus(void) {
	return bState;
}

// private:		//---------------------------------------------------------------------------------------------------------
BT::BT() {
	#ifdef BT_DBG																			// only if ee debug is set
	dbgStart();																				// serial setup
	dbg << F("BT.\n");																		// ...and some information
	#endif

	bMode = 0;
	bDuration = 0;
}

void    BT::poll(void) {
	if (!battTmr.done() ) return;															// timer still running

	measureCentiVolt = getBatteryVoltage();
	bState = (measureCentiVolt < checkCentiVolt) ? 1 : 0;									// set the battery status

	#ifdef BT_DBG																			// only if ee debug is set
		dbg << "cTV:" << checkCentiVolt << ", mTV:" << measureCentiVolt << " , s:" << bState << '\n';
	#endif

	battTmr.set(bDuration);																	// set next check time
}
