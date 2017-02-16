/**
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin channel module THSensWeather -----------------------------------------------------------------------------------
* - with a lot of support from martin876 at FHEM forum
* - -----------------------------------------------------------------------------------------------------------------------
*/

#ifndef _cm_thsensor_H
#define _cm_thsensor_H

#include "cm_master.h"

// default settings are defined in cm_thsensor.cpp - updatePeerDefaults

// List 1
const uint8_t cm_thsensor_ChnlReg[] PROGMEM = { 0x08, };
const uint8_t cm_thsensor_ChnlDef[] PROGMEM = { 0x00, };

// List 4
const uint8_t cm_thsensor_PeerReg[] PROGMEM = { 0x01,0x02, };
const uint8_t cm_thsensor_PeerDef[] PROGMEM = { 0x00,0x00, };


class cm_thsensor : public CM_MASTER {
private:  //---------------------------------------------------------------------------------------------------------------

	struct s_l1 {
		uint8_t AES_ACTIVE		: 1;		// 0x08.0, s:1   d: false  
		uint8_t					: 7;		// 0x08.1, s:7   d:   
	} *l1;  

	struct s_lstPeer {
		uint8_t peerNeedsBurst	:1;			// 0x01, s:0, e:1
		uint8_t useDHTTemp		:1;			// 0x01, s:1, e:1
		uint8_t					:6;			//
		uint8_t tempCorr;					// 0x02, max +/- 12,7°C (1/10°C)
	} *l4;

	uint32_t calcSendSlot(void);


public:  //----------------------------------------------------------------------------------------------------------------

	cm_thsensor(const uint8_t peer_max);												// constructor

	struct s_sensVal
	{
		uint16_t	temp;
		uint8_t		hum;
		uint16_t	bat;
	} sensVal;

	s_cm_status cm_status;																	// defined in cmMaster.h, holds current status and set_satatus

	waitTimer sensTmr;																		// delay timer for sensor

	virtual void cm_poll(void);																// poll function, driven by HM loop
	virtual void CONFIG_STATUS_REQUEST(s_m01xx0e *buf);
	virtual void info_peer_add(s_m01xx01 *buf);
	virtual void info_peer_remove(s_m01xx02 *buf);

};

extern void initTH(uint8_t channel);														// functions in user sketch needed
extern void measureTH(uint8_t channel, cm_thsensor::s_sensVal *sensVal);

#endif
