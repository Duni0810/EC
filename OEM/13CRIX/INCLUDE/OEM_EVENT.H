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
	XBYTE	*REG;				// ram空间的flag参数地址
	BYTE	Flag;				// flag 位置
	XBYTE	*Cunter;			// 像是消抖动用的计数器
	BYTE	Time;				// 重装用的数值
	FUNCT_PTR_V_V	press;		// 按键按下事件
	FUNCT_PTR_V_V	release;	// 按键弹起事件
	FUNCT_PTR_V_V	nochange;	// 按键保持事件
};

#endif