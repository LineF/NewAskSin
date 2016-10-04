//- -----------------------------------------------------------------------------------------------------------------------
// AskSin driver implementation
// 2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------
//- AskSin battery status functions ---------------------------------------------------------------------------------------
//- with a lot of support from dirk at FHEM forum
//- -----------------------------------------------------------------------------------------------------------------------

#ifndef _BT_H
#define _BT_H

#include "HAL.h"


class BT {
public:		//---------------------------------------------------------------------------------------------------------

	uint8_t  checkCentiVolt;												// holds the proof point
	uint8_t  measureCentiVolt;												// variable to hold last measured value
	uint8_t  bState        :1;												// holds status bit
	uint8_t  bMode         :2;												// mode variable
	uint32_t bDuration;														// duration for the next check
	
	BT();
	void	set(uint16_t centiVolt, uint32_t duration);
	uint16_t getVolts(void);
	uint8_t getStatus(void);

	void    poll(void);
};

extern BT bat;																// declaration in AS.cpp

#endif
