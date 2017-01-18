/**
*  AskSin driver implementation
*  2013-08-03 <trilu@gmx.de> Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
* - -----------------------------------------------------------------------------------------------------------------------
* - AskSin framework main class -------------------------------------------------------------------------------------------
* - with a lot of support from many people at FHEM forum
*   thanks a lot to martin876, dirk, pa-pa, martin, Dietmar63 and all i have not personal named here
*   special thank you to https://git.zerfleddert.de/hmcfgusb/AES/ for bidcos(R) AES explanation
* - -----------------------------------------------------------------------------------------------------------------------
*/


#include "newasksin.h"


static s_dev_ident   dev_ident;																// struct to hold the device identification related information									
static s_dev_operate dev_operate;															// struct to hold all operational variables or pointers

static s_pair_mode   pair_mode;																// helper structure for keeping track of active pairing mode
static s_config_mode config_mode;															// helper structure for keeping track of active config mode

static s_rcv_msg rcv_msg;																	// struct to process received strings
static s_snd_msg snd_msg;																	// same for send strings

static s_peer_msg peer_msg;																	// peer message array as buffer between send function and send processing
static s_list_msg list_msg;																	// holds information to answer config list requests for peer or param lists

static AS *hm = new AS();																	// the newasksin main class
static uint8_t cnl_max = 0;																	// increased by every instance which is initialized
//const uint8_t list_max = 5;																// max 5 lists per channel, list 0 to list 4


