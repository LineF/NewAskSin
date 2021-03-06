
#ifndef _REGISTER_H
#define _REGISTER_H


/*
*  @brief libraries needed to run AskSin library, everything is defined within the newasksin.h file 
*/
#include <newasksin.h> 
#include "hmkey.h"


/*
*  @brief definition of all classes which are necassary to run asksin
*/
//NO_AES as_aes;														//   60 byte flash,  69 byte sram
HAS_AES as_aes;															// 2826 byte flash, 277 byte sram
AES *aes = &as_aes;

CC1101 as_cc1101(pinB4, pinB3, pinB5, pinB2, pinD2);					//  546 byte flash, 124 byte sram
COM *com = &as_cc1101;

CBN as_cbn(1, pinB0);													//   80 byte flash,  25 byte sram
CBN *cbn = &as_cbn;

LED as_led(pinD6, pinD4);												//  150 byte flash,  51 byte sram
LED *led = &as_led;

NO_BAT as_bat;															//   34 byte flash,  22 byte sram
//INT_BAT as_bat(3600000, 30);											//  176 byte flash,  22 byte sram
//EXT_BAT as_bat(3600000, 30, pinD7, pinC6, 10, 45);					//  386 byte flash,  56 byte sram
BAT *bat = &as_bat;

POM as_pom(POWER_MODE_NO_SLEEP);										//   68 byte flash,  19 byte sram
POM *pom = &as_pom;


/*
*  @brief cm_maintenance requires this declaration in the user sketch to make registers flexible
*/
const uint8_t cm_maintenance_ChnlReg[] PROGMEM = { 0x02,0x08,0x0a,0x0b,0x0c,0x12, };
const uint8_t cm_maintenance_ChnlDef[] PROGMEM = { 0x80,0x01,0x00,0x00,0x00,0x69, };
uint8_t cm_maintenance_ChnlVal[sizeof(cm_maintenance_ChnlReg)];
const uint8_t cm_maintenance_ChnlLen = sizeof(cm_maintenance_ChnlReg);


/*
*  @brief definition of the device functionallity per channel
*/
CM_MAINTENANCE cm_maintenance(0);										//   24 byte flash, 124 byte sram
CM_REMOTE cm_remote1(10, pinC0);										//  827 byte flash, 100 byte sram
CM_REMOTE cm_remote2(10, pinC1);
CM_REMOTE cm_remote3(10, pinC2);
CM_REMOTE cm_remote4(10, pinC3);
CM_REMOTE cm_remote5(10, pinC4);
CM_REMOTE cm_remote6(10, pinC5); 

CM_MASTER *cmm[7] = {
	&cm_maintenance,
	&cm_remote1,
	&cm_remote2,
	&cm_remote3,
	&cm_remote4,
	&cm_remote5,
	&cm_remote6,
};



/*
* @brief HMID, Serial number, HM-Default-Key, Key-Index
*/
const uint8_t HMSerialData[] PROGMEM = {
	/* HMID */            0x00,0x11,0x22,
	/* Serial number */   'H','B','r','e','m','o','t','e','0','1',		// HBremote01 
	/* Key-Index */       HM_DEVICE_AES_KEY_INDEX,
	/* Default-Key */     HM_DEVICE_AES_KEY,
};


/**
* @brief Settings of HM device
* firmwareVersion: The firmware version reported by the device
*                  Sometimes this value is important for select the related device-XML-File
*
* modelID:         Important for identification of the device.
*                  @See Device-XML-File /device/supported_types/type/parameter/const_value
*
* subType:         Identifier if device is a switch or a blind or a remote
* DevInfo:         Sometimes HM-Config-Files are referring on byte 23 for the amount of channels.
*                  Other bytes not known.
*                  23:0 0.4, means first four bit of byte 23 reflecting the amount of channels.
*/
const uint8_t dev_static[] PROGMEM = {             // testID 
	/* firmwareVersion 1 byte */  0x11,           // or GE 
	/* modelID         2 byte */  0x00,0xa9,
	/* subTypeID       1 byte */  0x40,           // replace __ by a valid type id 
	/* deviceInfo      3 byte */  0x06,0x00,0x00, // device info not found, replace by valid values 
};


/**
* @brief Regular start function
* This function is called by the main function every time when the device starts,
* here we can setup everything which is needed for a proper device operation
*/
void everyTimeStart(void) {
	DBG(SER, F("HMID: "), _HEX(dev_ident.HMID, 3), F(", MAID: "), _HEX(dev_operate.MAID, 3), F(", CNL: "), cnl_max, F("\n\n"));	// some debug

}

/**
* @brief First time start function
* This function is called by the main function on the first boot of a device.
* First boot is indicated by a magic byte in the eeprom.
* Here we can setup everything which is needed for a proper device operation, like cleaning
* of eeprom variables, or setting a default link in the peer table for 2 channels
*/
void firstTimeStart(void) {
	DBG(SER, F("\n\nnew magic!\n\n"));

	/* add some peers to test - peers can be added per default, important if we want to build up combined devices.
	*  by doing this, we can build a light switch by combing a remote and a switch channel.
	*  first we need an array with the peer address and in a second step it is written into the respective channel.
	*  set_peer needs two parameters, the index which reflects the slot where the peer is written to and second the peer 
	*  address as array. please note: no defaults a written into the respective peer list, this has to be done manually */
//	uint8_t temp[] = { 0x01,0x02,0x01,0x01, };												// declare and fill array
//	cmm[1]->peerDB.set_peer(0, temp);														// write it to index 0
//	temp[2] = 0x02;																			// adjust array content
	//cmm[1]->peerDB.set_peer(1, temp);														// write to index 1
	//temp[2] = 0x03;
	//cmm[1]->peerDB.set_peer(2, temp);
	//temp[2] = 0x04;
	//cmm[1]->peerDB.set_peer(3, temp);

	/* this example shows how peer lists (list3/4) could be set manually. in the first example we set default values
	*  to channel 1 list 4 for the first peer. in the second example we write a custom information to channel 1 list 4
	*  peer 2,3 and 4. default values are stored in the respective channel module */
	//cmm[1]->lstP.load_default();															// load the defaults into the list 4
	//cmm[1]->lstP.save_list(0);															// write it to index 0

	//temp[0] = 0x01; temp[1] = 0x01;														// adjust array content
	//cmm[1]->lstP.write_array(temp, 2, 1);													// write 2 bytes into index 1
	//cmm[1]->lstP.write_array(temp, 2, 2);
	//cmm[1]->lstP.write_array(temp, 2, 3);
}





#endif


/**
* @brief Channel structs (for developers)
* Within the channel struct you will find the definition of the respective registers per channel and list.
* These information is only needed if you want to develop your own channel module, for pre defined
* channel modules all this definitions enclosed in the pre defined module.
*/

struct s_cnl0_lst0 {
    uint8_t                       : 7;  // 0x02.0, s:7   d:   
	uint8_t INTERNAL_KEYS_VISIBLE : 1;  // 0x02.7, s:1   d: true  
	uint8_t MASTER_ID             : 24; // 0x0a.0, s:24  d:   
	uint8_t LOW_BAT_LIMIT         : 8;  // 0x12.0, s:8   d: 3 V 
	uint8_t LOCAL_RESET_DISABLE   : 1;  // 0x18.0, s:1   d: false  
    uint8_t                       : 7;  // 0x18.1, s:7   d:   
}; // 6 byte

struct s_cnl1_lst1 {
	uint8_t                       : 4;  // 0x04.0, s:4   d:   
	uint8_t LONG_PRESS_TIME       : 4;  // 0x04.4, s:4   d: 0.4 s 
	uint8_t AES_ACTIVE            : 1;  // 0x08.0, s:1   d: false  
	uint8_t                       : 7;  // 0x08.1, s:7   d:   
	uint8_t DBL_PRESS_TIME        : 4;  // 0x09.0, s:4   d: 0.0 s 
	uint8_t                       : 4;  // 0x09.4, s:4   d:   
}; // 3 byte

struct s_cnl1_lst4 {
	uint8_t PEER_NEEDS_BURST      : 1;  // 0x01.0, s:1   d: false  
    uint8_t                       : 6;  // 0x01.1, s:6   d:   
	uint8_t EXPECT_AES            : 1;  // 0x01.7, s:1   d: false  
}; // 1 byte

   // struct s_cnl2_lst1 linked to 01 01
   // struct s_cnl2_lst4 linked to 01 04
   // struct s_cnl3_lst1 linked to 01 01
   // struct s_cnl3_lst4 linked to 01 04
   // struct s_cnl4_lst1 linked to 01 01
   // struct s_cnl4_lst4 linked to 01 04
   // struct s_cnl5_lst1 linked to 01 01
   // struct s_cnl5_lst4 linked to 01 04
   // struct s_cnl6_lst1 linked to 01 01
   // struct s_cnl6_lst4 linked to 01 04

   /**
   * @brief Message description:
   *
   *        00        01 02    03 04 05  06 07 08  09  10  11   12     13
   * Length MSG_Count    Type  Sender__  Receiver  ACK Cnl Stat Action RSSI
   * 0F     12        80 02    1E 7A AD  23 70 EC  01  01  BE   20     27    dimmer
   * 0E     5C        80 02    1F B7 4A  63 19 63  01  01  C8   00     42    pcb relay
   *
   * Needed frames:
   *
   * <frame id="KEY_EVENT_SHORT" direction="from_device" allowed_receivers="CENTRAL,BROADCAST,OTHER" event="true" type="0x40" channel_field="9:0.6">
   *      <parameter type="integer" index="9.6" size="0.1" const_value="0"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="COUNTER"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="TEST_COUNTER"/>
   * <frame id="KEY_EVENT_LONG" direction="from_device" allowed_receivers="CENTRAL,BROADCAST,OTHER" event="true" type="0x40" channel_field="9:0.6">
   *      <parameter type="integer" index="9.6" size="0.1" const_value="1"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="COUNTER"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="TEST_COUNTER"/>
   * <frame id="KEY_EVENT_LONG_BIDI" direction="from_device" allowed_receivers="CENTRAL,BROADCAST,OTHER" event="true" type="0x40" channel_field="9:0.6">
   *      <parameter type="integer" index="1.5" size="0.1" const_value="1"/>
   *      <parameter type="integer" index="9.6" size="0.1" const_value="1"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="COUNTER"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="TEST_COUNTER"/>
   * <frame id="KEY_SIM_SHORT" direction="from_device" type="0x40" channel_field="9:0.6">
   *      <parameter type="integer" index="9.6" size="0.1" const_value="0"/>
   *      <parameter type="integer" index="9.7" size="0.1" const_value="0"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="SIM_COUNTER"/>
   * <frame id="KEY_SIM_LONG" direction="from_device" type="0x40" channel_field="9:0.6">
   *      <parameter type="integer" index="9.6" size="0.1" const_value="1"/>
   *      <parameter type="integer" index="9.7" size="0.1" const_value="0"/>
   *      <parameter type="integer" index="10.0" size="1.0" param="SIM_COUNTER"/>
   */

