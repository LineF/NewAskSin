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
#include "cmTHSensWeather.h"

cmTHSensWeather::cmTHSensWeather(const uint8_t peer_max) : cmMaster(peer_max) {

	lstC.lst = 1;																			// setup the channel list with all dependencies
	lstC.reg = (uint8_t*)cmTHSensWeather_ChnlReg;
	lstC.def = (uint8_t*)cmTHSensWeather_ChnlDef;
	lstC.len = sizeof(cmTHSensWeather_ChnlReg);
	lstC.val = new uint8_t[lstC.len];

	lstP.lst = 4;																			// setup the peer list with all dependencies
	lstP.reg = (uint8_t*)cmTHSensWeather_PeerReg;
	lstP.def = (uint8_t*)cmTHSensWeather_PeerDef;
	lstP.len = sizeof(cmTHSensWeather_PeerReg);
	lstP.val = new uint8_t[lstP.len];

	l1 = (s_l1*)lstC.val;																	// set list structures to something useful
	l4 = (s_lstPeer*)lstP.val;
	DBG(TH, F("TH: lstC.val(l1): "), _HEX((uint8_t*)&l1, 2), F(", lstP.val(l4): "), _HEX((uint8_t*)&l4, 2), '\n');

	initTH(lstC.cnl);																		// call external init function to set the output pins

	//uint16_t msgDelay = (rand() % 2000) + 1000;												// set message delay
	//msgTmr.set(msgDelay);																	// wait some time to settle the device
	//sendStat = INFO::SND_ACTUATOR_STATUS;													// send the initial status info

	sensTmr.set(3000);																		// wait for first measurement being completed

	DBG(TH, F("cmTHSensWeather, cnl: "), lstC.cnl, '\n');
}


/**------------------------------------------------------------------------------------------------------------------------
*- user defined functions -
* -------------------------------------------------------------------------------------------------------------------------
*
* @brief Function is called on messages coming from a central device.
*
* @param setValue 1 byte containing the value we have to set
* @param rampTime Pointer to 2 byte array containing the encoded ramptime value
* @param duraTime Pointer to 2 byte array containing the encoded durationtime value
*
*/
void cmTHSensWeather::message_trigger11(uint8_t setValue, uint8_t *rampTime, uint8_t *duraTime) {
	DBG(TH, F("trigger11, setValue:"), setValue, F(", rampTime:"), intTimeCvt((uint16_t) rampTime), F(", duraTime:"), intTimeCvt((uint16_t) duraTime), '\n' );
}

/**
* @brief Function is called on messages coming from master, simulating a remote or push button.
*
* @param msgLng 1 byte containing the long message flag
* @param msgCnt 1 byte containing the message counter of the sender
*
*/
void cmTHSensWeather::message_trigger3E(uint8_t msgLng, uint8_t msgCnt) {
	message_trigger40(msgLng, msgCnt);
}

/**
* @brief Function is called on messages coming from a remote or push button.
*
* @param msgLng 1 byte containing the long message flag
* @param msgCnt 1 byte containing the message counter of the sender
*
*/
void cmTHSensWeather::message_trigger40(uint8_t msgLng, uint8_t msgCnt) {
	DBG(TH, F("trigger40, msgLng:"), msgLng, F(", msgCnt:"), msgCnt, '\n' );
}

/**
* @brief Function is called on messages coming from sensors.
*
* @param msgLng 1 byte containing the long message flag
* @param msgCnt 1 byte containing the message counter of the sender
* @param msgVal 1 byte with the value of the sensor
*
*/
void cmTHSensWeather::message_trigger41(uint8_t msgLng, uint8_t msgCnt, uint8_t msgVal) {
	DBG(TH, F("trigger41, msgLng:"), msgLng, F(", msgCnt:"), msgCnt, F(", val:"), msgVal, '\n' );
}

void cmTHSensWeather::sendStatus(void) {

	if (!sendStat) return;																	// nothing to do
	//if (!msgTmr.done()) return;																// not the right time
	
	// check which type has to be send
	if      ( sendStat == INFO::SND_ACK_STATUS )      hm.send_ACK_STATUS(lstC.cnl, 0, 0);
	else if ( sendStat == INFO::SND_ACTUATOR_STATUS ) hm.sendINFO_ACTUATOR_STATUS(lstC.cnl, 0, 0);

	sendStat = INFO::NOTHING;
}

void cmTHSensWeather::poll(void) {

	//adjustStatus();																		// check if something is to be set on the Relay channel
	sendStatus();																			// check if there is some status to send

	if (!sensTmr.done() ) return;															// step out while timer is still running
	
	sensTmr.set((calcSendSlot()*250 + 1000));												// set a new measurement time

	measureTH(lstC.cnl, &sensVal);															// call the measurement function
	DBG(TH, F("TH: tmp: "), _HEX((uint8_t *)&sensVal.temp, 2), F(", hum: "), sensVal.hum, F(", bat:"), _HEX((uint8_t *)&sensVal.bat, 2), F("\n"));
	DBG(TH, F("TH: lstC.val(l1): "), _HEX((uint8_t*)&l1, 2), F(", lstP.val(l4): "), _HEX((uint8_t*)&l4, 2), '\n');
	DBG(TH, F("TH: lstC.val: "), _HEX((uint8_t*)&lstC.val, 2), F(", lstP.val: "), _HEX((uint8_t*)&lstP.val, 2), '\n');
	
	hm.sendINFO_WEATHER_EVENT(lstC.cnl, 0, (uint8_t *)&sensVal, sizeof(sensVal));			// prepare the message and send, burst if burstRx register is set
}


void cmTHSensWeather::set_toggle(void) {
	// setToggle will be addressed by config button in mode 2 by a short key press
	// here we can toggle the status of the actor
	DBG(TH, F("set_toggle\n") );
}


void cmTHSensWeather::request_pair_status(void) {
	// we received a status request, appropriate answer is an InfoActuatorStatus message
	DBG(TH, F("request_pair_status\n") );
	
	//sendStat = INFO::SND_ACTUATOR_STATUS;													// send next time a info status message
	//msgTmr.set(10);																			// wait a short time to set status
}


uint32_t cmTHSensWeather::calcSendSlot(void) {
	uint8_t a[4];
	a[0] = dev_ident.HMID[2];
	a[1] = dev_ident.HMID[1];
	a[2] = dev_ident.HMID[0];
	a[3] = 0;

	uint32_t result = ((( *(uint32_t*)&a << 8) | (snd_msg.mBody.MSG_CNT)) * 1103515245 + 12345) >> 16;
	result = (result & 0xFF) + 480;

	return result;
}


/**
* This function will be called by the eeprom module as a request to update the
* list4 structure by the default values per peer channel for the user module.
* Overall defaults are already set to the list3/4 by the eeprom class, here it 
* is only about peer channel specific deviations.
* As we get this request for each peer channel we don't need the peer index.
* Setting defaults could be done in different ways but this should be the easiest...
*
* Byte     00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21
* ADDRESS  02 03 04 05 06 07 08 09 0a 0b 0c 82 83 84 85 86 87 88 89 8a 8b 8c
* DEFAULT  00 00 32 64 00 ff 00 ff 01 44 44 00 00 32 64 00 ff 00 ff 21 44 44
* to Change                           14 63                            14 63
* OFF                                 64 66                            64 66
* ON                                  13 33                            13 33
* TOGGLE   0B 14  0C 63  8B 14  8C 63
* As we have to change only 4 bytes, we can map the struct to a byte array point
* and address/change the bytes direct. EEprom gets updated by the eeprom class
* automatically.
*/

void cmTHSensWeather::request_peer_defaults(uint8_t idx, s_m01xx01 *buf) {

	// if both peer channels are given, peer channel 01 default is the off dataset, peer channel 02 default is the on dataset
	// if only one peer channel is given, then the default dataset is toogle
	//if (( buf->PEER_CNL[0] ) && ( buf->PEER_CNL[1] )) {		// dual peer add

		//if (idx % 2) {										// odd (1,3,5..) means OFF
			//lstP.val[9] = lstP.val[20] = 0x64;														// set some byte
			//lstP.val[10] = lstP.val[21] = 0x66;
		//} else {											// even (2,4,6..) means ON
			//lstP.val[9] = lstP.val[20] = 0x13;														// set some byte
			//lstP.val[10] = lstP.val[21] = 0x33;
		//}

	//} else  {												// toggle peer channel
		//lstP.val[9]  = lstP.val[20] = 0x14;															// set some byte
		//lstP.val[10] = lstP.val[21] = 0x63;
	//} 

	DBG(TH, F("cmTHSensWeather:request_peer_defaults CNL_A:"), _HEXB(buf->PEER_CNL[0]), F(", CNL_B:"), _HEXB(buf->PEER_CNL[1]), F(", idx:"), _HEXB(idx), '\n' );
}

