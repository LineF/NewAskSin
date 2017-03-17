/**
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin channel module Remote ------------------------------------------------------------------------------------------
* - -----------------------------------------------------------------------------------------------------------------------
*/

#ifndef _CM_REMOTE_H
#define _CM_REMOTE_H

#include "cm_master.h"


const uint8_t cm_remote_ChnlReg[] PROGMEM = { 0x04,0x08,0x09, };
const uint8_t cm_remote_ChnlDef[] PROGMEM = { 0x40,0x00,0x00, };

const uint8_t cm_remote_PeerReg[] PROGMEM = { 0x01, };
const uint8_t cm_remote_PeerDef[] PROGMEM = { 0x00, };


class CM_REMOTE : public CM_MASTER {
private:  //---------------------------------------------------------------------------------------------------------------

	uint8_t cm_remote_ChnlVal[sizeof(cm_remote_ChnlReg)];									// the register value store in memory
	uint8_t cm_remote_PeerVal[sizeof(cm_remote_PeerReg)];

	struct s_l1 {
		uint8_t                  : 4;  // 0x04.0, s:4   d:   
		uint8_t LONG_PRESS_TIME  : 4;  // 0x04.4, s:4   d: 0.4 s 
		uint8_t AES_ACTIVE       : 1;  // 0x08.0, s:1   d: false  
		uint8_t                  : 7;  // 0x08.1, s:7   d:   
		uint8_t DBL_PRESS_TIME   : 4;  // 0x09.0, s:4   d: 0.0 s 
		uint8_t                  : 4;  // 0x09.4, s:4   d:   
	} *l1; // 3 byte

	struct s_l4 {
		uint8_t PEER_NEEDS_BURST : 1;  // 0x01.0, s:1   d: false  
		uint8_t                  : 6;  // 0x01.1, s:6   d:   
		uint8_t EXPECT_AES       : 1;  // 0x01.7, s:1   d: false  
	} *l4; // 1 byte


	uint8_t def_key;																		// here we store the key definition
	waitTimer timer;																		// timer to detect long press and dbl_short

	struct s_button_check {
		uint8_t configured       : 1;														// poll the pin make only sense if it was configured, store result here
		uint8_t armed            : 1;														// if this is set to 1, poll function should be entered
		uint8_t last_short       : 1;														// if the last key press was a short to detect a double short
		uint8_t last_long        : 1;														// if the last keypress was a long to detect a double long
	    uint8_t                  : 4;
	} button_check;
	
	struct s_button_ref {
		uint8_t status;																		// variable to store current status in polling function
		uint8_t counter = 0;																// channel specific message counter
	} button_ref;

	struct s_button_info {
		uint8_t channel          : 6;														// set in regInHM function, will not change at runtime
		uint8_t longpress        : 1;														// will be set in buttonAction function
		uint8_t lowbat           : 1;														// placeholder here, will be set in as module
		uint8_t counter          : 8;														// will be increased in buttonAction function
	} button_info;																			// holds the details for the send event message


public:  //----------------------------------------------------------------------------------------------------------------

	CM_REMOTE(const uint8_t peer_max, uint8_t pin_key);										// constructor
	CM_REMOTE(const uint8_t peer_max);														// constructor, without pin
	virtual void cm_init();																	// overwrite the init function in cmMaster

	virtual void cm_poll(void);																// poll function, driven by HM loop

	virtual void button_action(uint8_t event);

};

extern void initRemote(uint8_t channel);												// functions in user sketch needed

#endif
