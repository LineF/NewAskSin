/**
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin battery measurement class --------------------------------------------------------------------------------------
* - -----------------------------------------------------------------------------------------------------------------------
*/

#ifndef _BAT_H
#define _BAT_H

#include "HAL.h"
#include "waittimer.h"


class BAT {
protected:  //-------------------------------------------------------------------------------------------------------------
	uint16_t  default_value;												// value to compare the measured value against
	uint16_t  measure_value;												// measured hundreth volt battery value
	uint32_t  interval;														// duration of the regular check
	waitTimer timer;														// battery timer for duration check

	BAT() {}
	virtual void do_measure(void) {}

public:  //----------------------------------------------------------------------------------------------------------------
	void     set(uint32_t check_interval, uint16_t centi_volt);
	void     poll(void);
	uint8_t  get_status(void);
	uint16_t get_volts() { return measure_value; };

};

class NO_BAT : public BAT {
public:  //----------------------------------------------------------------------------------------------------------------
	/*
	* @brief no battery measurement done by choosing this constructor
	*/
	NO_BAT();
};

class INT_BAT : public BAT {
public:  //----------------------------------------------------------------------------------------------------------------
	INT_BAT(uint32_t check_interval, uint16_t centi_volt);

protected:  //-------------------------------------------------------------------------------------------------------------
	void do_measure(void);

};

class EXT_BAT : public BAT {
public:  //----------------------------------------------------------------------------------------------------------------
	EXT_BAT(uint32_t check_interval, uint16_t centi_volt, uint8_t pin_enable, uint8_t pin_measure, uint8_t z1, uint8_t z2);

protected:  //-------------------------------------------------------------------------------------------------------------
	uint8_t def_enable;
	uint8_t def_measure;
	uint8_t res1, res2;

	void do_measure(void);

};

#endif
