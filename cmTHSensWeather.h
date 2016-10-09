/**
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin channel module THSensWeather -----------------------------------------------------------------------------------
* - with a lot of support from martin876 at FHEM forum
* - -----------------------------------------------------------------------------------------------------------------------
*/

#ifndef _cmTHSensWeather_H
#define _cmTHSensWeather_H

#include "cmMaster.h"

// default settings are defined in cmTHSensWeather.cpp - updatePeerDefaults


// List 1
const uint8_t cmTHSensWeather_ChnlReg[] PROGMEM = { 0x08, };
const uint8_t cmTHSensWeather_ChnlDef[] PROGMEM = { 0x00, };

// List 4
const uint8_t cmTHSensWeather_PeerReg[] PROGMEM = { 0x01,0x02, };
const uint8_t cmTHSensWeather_PeerDef[] PROGMEM = { 0x00,0x00, };

//#define NOT_USED 255
//namespace ACTION {
	//enum E : uint8_t { INACTIVE, JUMP_TO_TARGET, TOGGLE_TO_COUNTER, TOGGLE_INV_TO_COUNTER };
//};
//namespace JT {
	//enum E : uint8_t { NO_JUMP_IGNORE_COMMAND = 0x00, ONDELAY = 0x01, ON = 0x03, OFFDELAY = 0x04, OFF = 0x06 };
//};
//namespace CT {
	//enum E : uint8_t { X_GE_COND_VALUE_LO, X_GE_COND_VALUE_HI, X_LT_COND_VALUE_LO, X_LT_COND_VALUE_HI, COND_VALUE_LO_LE_X_LT_COND_VALUE_HI, X_LT_COND_VALUE_LO_OR_X_GE_COND_VALUE_HI };
//};
namespace INFO {
	enum E : uint8_t { NOTHING, SND_ACK_STATUS, SND_ACTUATOR_STATUS };
}

class cmTHSensWeather : public cmMaster {
private:  //---------------------------------------------------------------------------------------------------------------

	struct s_l1 {
		uint8_t AES_ACTIVE           : 1;  // 0x08.0, s:1   d: false  
		uint8_t                      : 7;  // 0x08.1, s:7   d:   
	} *l1;  

	struct s_lstPeer {
		uint8_t  peerNeedsBurst:1;			// 0x01, s:0, e:1
		uint8_t  useDHTTemp    :1;			// 0x01, s:1, e:1
		uint8_t                :6;			//
		uint8_t  tempCorr;					// 0x02, max +/- 12,7°C (1/10°C)
	} *l4;

	uint32_t calcSendSlot(void);


public:  //----------------------------------------------------------------------------------------------------------------

	cmTHSensWeather(const uint8_t peer_max);												// constructor

	struct s_sensVal
	{
		uint16_t	temp;
		uint8_t		hum;
		uint16_t	bat;
	} sensVal;

	waitTimer msgTmr;																		// message timer for sending status
	uint8_t	  sendStat;																		// indicator for sendStatus function

	waitTimer sensTmr;																		// delay timer for sensor


	virtual void message_trigger11(uint8_t value, uint8_t *rampTime, uint8_t *duraTime);	// what happens while a trigger11 message arrive
	virtual void message_trigger3E(uint8_t msgLng, uint8_t msgCnt);							// same for switch messages
	virtual void message_trigger40(uint8_t msgLng, uint8_t msgCnt);							// same for peer messages
	virtual void message_trigger41(uint8_t msgLng, uint8_t msgCnt, uint8_t msgVal);			// same for sensor messages

	virtual void request_peer_defaults(uint8_t idx, s_m01xx01 *buf);						// add peer channel defaults to list3/4
	virtual void request_pair_status(void);													// event on status request

	virtual void poll(void);																// poll function, driven by HM loop
	virtual void set_toggle(void);															// toggle the module initiated by config button

	inline void sendStatus(void);															// help function to send status messages
};

extern void initTH(uint8_t channel);														// functions in user sketch needed
extern void measureTH(uint8_t channel, cmTHSensWeather::s_sensVal *sensVal);

#endif
