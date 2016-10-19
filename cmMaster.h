/**
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin channel master functions ---------------------------------------------------------------------------------------
* - idea for the master class from Dietmar63 at FHEM forum
* - -----------------------------------------------------------------------------------------------------------------------
*/

#ifndef _cmMaster_H
#define _cmMaster_H

#include "AS.h"
#include "HAL.h"
#include "AS_typedefs.h"

const uint8_t list_max = 5;

class cmMaster {
public://------------------------------------------------------------------------------------------------------------------

	/*
	* @brief Every channel has two lists, the first list holds the configuration which is required to drive the channel,
	*        the second list is related to peer messages and holds all information which are required to drive the functionality
	*        of the channel in combination with peer devices.
	*        Therefore we have in every channel two lists - lstC and lstP organized in structs
	*/
	s_list_table *list[list_max] = {};
	s_list_table lstC;
	s_list_table lstP;

	/*
	* @brief Peer Device Table Entry
	*
	* This structure is used to specify the number of possible peers per channel and
	* assign corresponding EEprom memory sections where peer information is to be stored.
	*
	* For each channel and peered device, 4 bytes are written to EEprom memory denoting the
	* peer device HMID (3 bytes) and peer device channel (1 byte). Consequently, the following
	* definition with 6 possible peers for channel 1 will use 24 bytes in EEprom memory.
	*/
	s_peer_table peerDB;

	cmMaster(const uint8_t peer_max);														// constructor

	virtual void message_trigger11(uint8_t value, uint8_t *rampTime, uint8_t *duraTime);	// pair set message
	virtual void message_trigger3E(uint8_t msgLng, uint8_t msgCnt);							// switch message, also config test button in HM
	virtual void message_trigger40(uint8_t msgLng, uint8_t msgCnt);							// remote messages from peer
	virtual void message_trigger41(uint8_t msgLng, uint8_t msgCnt, uint8_t msgVal);			// sensor messages from peer

	virtual void info_config_change(void);													// list1 on registered channel had changed
	virtual void info_peer_add(s_m01xx01 *buf);												// peer was added to the specific channel, 1st 3 bytes shows peer address, 4th and 5th the peer channel

	virtual void request_peer_defaults(uint8_t idx, s_m01xx01 *buf);						// add peer channel defaults to list3/4

	void poll(void);																		// poll function, driven by HM loop
	virtual void cm_poll(void) {}															// poll function for channel modules to overwrite

	virtual void set_toggle(void);															// toggle the module initiated by config button


	/* receive functions to handle requests forwarded by AS:processMessage 
	*  only channel module related requests are forwarded, majority of requests are handled within main AS class */
	virtual void CONFIG_STATUS_REQUEST(s_m01xx0e *buf);

	virtual void INSTRUCTION_INHIBIT_OFF();	
	virtual void INSTRUCTION_INHIBIT_ON();	
	virtual void INSTRUCTION_SET();	
	virtual void INSTRUCTION_STOP_CHANGE();	
	virtual void INSTRUCTION_LED();	
	virtual void INSTRUCTION_LED_ALL();	
	virtual void INSTRUCTION_LEVEL();	
	virtual void INSTRUCTION_SET_TEMP();
	virtual void INSTRUCTION_ADAPTION_DRIVE_SET();

	virtual void SWITCH();	
	virtual void TIMESTAMP();
	virtual void REMOTE();	
	virtual void SENSOR_EVENT();
	virtual void SWITCH_LEVEL();
	virtual void SENSOR_DATA();
	virtual void GAS_EVENT();
	virtual void CLIMATE_EVENT(); 
	virtual void SET_TEAM_TEMP();
	virtual void THERMAL_CONTROL();
	virtual void POWER_EVENT_CYCLE();
	virtual void POWER_EVENT();
	virtual void WEATHER_EVENT();


};


/* as there is no way to get the channel by setting up the pointer array for channel modules we use this
*  byte to identify the channel we are actually setting up, increased in the constructor...
   the overall amount will be kept for runtime to step through the different instances. */
extern uint8_t cnl_max;
extern cmMaster *ptr_CM[];





//- helpers ---------------------------------------------------------------------------------------------------------------
uint16_t cm_prep_default(uint16_t ee_start_addr);											// prepare the defaults incl eeprom address mapping
uint8_t  is_peer_valid(uint8_t *peer);														// search through all instances and ceck if we know the peer, returns the channel

uint16_t cm_calc_crc(void);																	// calculate the crc for lists in the modules
inline uint16_t crc16_P(uint16_t crc, uint8_t len, const uint8_t *buf);						// calculates the crc for a PROGMEM byte array
inline uint16_t crc16(uint16_t crc, uint8_t a);												// calculates the crc for a given byte

#endif