/*- -----------------------------------------------------------------------------------------------------------------------
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin send function --------------------------------------------------------------------------------------------------
* - with some support from martin876 at FHEM forum, AES implementation from Dirk
* - -----------------------------------------------------------------------------------------------------------------------
*/

#ifndef _SN_H
#define _SN_H

#include "HAL.h"
#include "AS_typedefs.h"



class SN  {
	#define sndStrLen	 snd_msg.mBody->MSG_LEN+1							// amount of bytes in the send buffer
	#define sndAckReq    snd_msg.mBody->FLAG.BIDI							// check if an ACK is requested

public:		//---------------------------------------------------------------------------------------------------------

	SN();																	// constructor
	void poll(void);														// poll function, process if something is to send


	//void prep_msg(MSG_REASON::E reason, MSG_INTENT::E intent, MSG_TYPE::E type, uint8_t len, uint8_t max_retr);

};


/*
* @brief Struct to hold the buffer for any string to send with some flags for further processing
*/
extern s_send snd_msg;

extern SN snd;																// send class is declared in AS.cpp

#endif 
