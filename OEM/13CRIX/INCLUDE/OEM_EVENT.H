/*-----------------------------------------------------------------------------
 * TITLE: OEM_EVENT.H
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/

#ifndef OEM_INPUT_H
#define OEM_INPUT_H

//-----------------------------------------------------------------------------
// Function prototype
//-----------------------------------------------------------------------------
extern void EventManager(BYTE device_id);
extern void Auto_NOTIFY_EM(BYTE adata);
extern void Check_notify_em(void);
extern void AdapterIn(void);		//:LM
extern void AdapterOut(void);		//:LM

struct sDebounce
{
	XBYTE	*REG;
	BYTE	Flag;
	XBYTE	*Cunter;
	BYTE	Time;
	FUNCT_PTR_V_V	press;
	FUNCT_PTR_V_V	release;
	FUNCT_PTR_V_V	nochange;	
};

#endif