/**
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin channel module THSensWeather -----------------------------------------------------------------------------------
* - with a lot of support from martin876 at FHEM forum
* - -----------------------------------------------------------------------------------------------------------------------
*/

#include "00_debug-flag.h"


/**------------------------------------------------------------------------------------------------------------------------
*- mandatory functions for every new module to communicate within HM protocol stack -
* -------------------------------------------------------------------------------------------------------------------------
*
* @brief Constructor for channel module THSensWeather
*        pointer to channel table are forwarded to the master class. 
*        Constructor of master class is processed first.
*        Setup of class specific things is done here
*/
#include "newasksin.h"
#include "cm_thsensor.h"
#include <as_main.h>

cm_thsensor::cm_thsensor(const uint8_t peer_max) : CM_MASTER(peer_max) {
	DBG(TH, F("TH.\n"));

	lstC.lst = 1;																			// setup the channel list with all dependencies
	lstC.reg = (uint8_t*)cm_thsensor_ChnlReg;
	lstC.def = (uint8_t*)cm_thsensor_ChnlDef;
	lstC.len = sizeof(cm_thsensor_ChnlReg);
	lstC.val = new uint8_t[lstC.len];

	lstP.lst = 4;																			// setup the peer list with all dependencies
	lstP.reg = (uint8_t*)cm_thsensor_PeerReg;
	lstP.def = (uint8_t*)cm_thsensor_PeerDef;
	lstP.len = sizeof(cm_thsensor_PeerReg);
	lstP.val = new uint8_t[lstP.len];

	l1 = (s_l1*)lstC.val;																	// set list structures to something useful
	l4 = (s_lstPeer*)lstP.val;

	initTH(lstC.cnl);																		// call external init function to set the output pins

	cm_status.message_type = STA_INFO::NOTHING;												// send the initial status info
	cm_status.message_delay.set(0);

	sensTmr.set(3000);																		// wait for first measurement being completed

	DBG(TH, F("cm_thsensor, cnl: "), lstC.cnl, '\n');
}


/**------------------------------------------------------------------------------------------------------------------------
*- user defined functions -
* -------------------------------------------------------------------------------------------------------------------------
*/

/*
* @brief Received message handling forwarded by AS::processMessage
*/
void cm_thsensor::CONFIG_STATUS_REQUEST(s_m01xx0e *buf) {
	cm_status.message_type = STA_INFO::SND_ACTUATOR_STATUS;										// send next time a info status message
	cm_status.message_delay.set(50);														// wait a short time to set status

	DBG(TH, F("TH:CONFIG_STATUS_REQUEST\n"));
}

void cm_thsensor::cm_poll(void) {

	process_send_status_poll(&cm_status, lstC.cnl);											// check if there is some status to send, function call in cmMaster.cpp

	if (!sensTmr.done() ) return;															// step out while timer is still running
	sensTmr.set((calcSendSlot()*250 + 1000));												// set a new measurement time

	measureTH(lstC.cnl, &sensVal);															// call the measurement function
	//DBG(TH, F("TH: tmp: "), _HEX((uint8_t *)&sensVal.temp, 2), F(", hum: "), sensVal.hum, F(", bat:"), _HEX((uint8_t *)&sensVal.bat, 2), F("\n"));
	//DBG(TH, F("TH: lstC.val(l1): "), _HEX((uint8_t*)&l1, 2), F(", lstP.val(l4): "), _HEX((uint8_t*)&l4, 2), '\n');
	//DBG(TH, F("TH: lstC.val: "), _HEX((uint8_t*)&lstC.val, 2), F(", lstP.val: "), _HEX((uint8_t*)&lstP.val, 2), '\n');
	
	hm->send_WEATHER_EVENT(this, (uint8_t *)&sensVal, sizeof(sensVal));							// prepare the message and send, burst if burstRx register is set
}

uint32_t cm_thsensor::calcSendSlot(void) {
	uint8_t a[4];
	a[0] = dev_ident.HMID[2];
	a[1] = dev_ident.HMID[1];
	a[2] = dev_ident.HMID[0];
	a[3] = 0;

	uint32_t result = ((( *(uint32_t*)&a << 8) | (snd_msg.mBody.MSG_CNT)) * 1103515245 + 12345) >> 16;
	result = (result & 0xFF) + 480;

	return result;
}
