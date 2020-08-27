/*-----------------------------------------------------------------------------
 * TITLE: OEM_LED.C
 *
 * Copyright (c) 1983-2007, Insyde Software Corporation. All Rights Reserved.
 *
 * You may not reproduce, distribute, publish, display, perform, modify, adapt,
 * transmit, broadcast, present, recite, release, license or otherwise exploit
 * any part of this publication in any form, by any means, without the prior
 * written permission of Insyde Software Corporation.
 *---------------------------------------------------------------------------*/

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

/* ----------------------------------------------------------------------------
 * FUNCTION:   OEM_Write_Leds
 *
 * Write to SCROLL LOCK, NUM LOCK, CAPS LOCK, and any custom LEDs (including
 * an "Fn" key LED).
 *
 * Input: data (LED_ON bit) = Data to write to the LEDs
 *         bit 0 - SCROLL, Scroll Lock
 *         bit 1 - NUM,    Num Lock
 *         bit 2 - CAPS,   Caps Lock
 *         bit 3 - OVL,    Fn LED
 *         bit 4 - LED4,   Undefined
 *         bit 5 - LED5,   Undefined
 *         bit 6 - LED6,   Undefined
 *         bit 7 - LED7,   Undefined
 *
 * Updates Led_Data variable and calls send_leds if appropriate.
 * ------------------------------------------------------------------------- */
void OEM_Write_Leds(BYTE data_byte)
{
    Led_Data = data_byte;
    if ( data_byte & BIT1 )
    {
	   	Hook_NUMLED_ON();
    }
 	else
 	{
		Hook_NUMLED_OFF();
 	}
	
    if ( data_byte & BIT2 )
    {
		Hook_CAPLED_ON();
    }
	else
	{
		Hook_CAPLED_OFF();
	}
}




void ProcessLED(void)
{
	BYTE bat_index, pwr_index;   

	if(ledTestFlag)
		return;

	if(SystemIsS4 || SystemIsS5)
	{
		GPCRA1=OUTPUT;
		PWM1LHE=0x00;
		PWR_LED_OFF();
	}
	else if(SystemIsS3)
	{
		GPCRA1=ALT;
		PWM1LHE = 0x01;				
	}
	else
	{
		//TF_005--	GPCRA1=OUTPUT;
		GPCRA1=ALT;	//TF_005++
		PWM1LHE=0x00;
		//TF_005--	PWR_LED_ON();	
		//TF_005A--	DCR1 = 0x20;	//TF_005++
		DCR1 = 0x2C;	//TF_005A++
	}
	if ((Read_ACIN()) && (IS_BATTERY1_IN()))
	{
		BAT_LED_Cnt_Low = 0;
		BAT_LED_Cnt_CLow = 0;
		if (BAT1_RSOC > 95)
		{
			BAT_LED1_ON();
			BAT_LED2_OFF();
			if (IS_MASK_SET(BAT1_ALARM1, FullyChg))
			{
				if(BAT1_CHG ^ Fully)
					ECQEvent(BAT1_HOT_PLUG_EVT,SCIMode_Normal);
				BAT1_CHG |= Fully;
				BAT1_CHG &= ~RemCap;	
				BAT1_CHG &= ~Charging;	
			}
			else if((BAT1_CHG & Charging) || (BAT1_CHG & Fully))
			{
				ECQEvent(BAT1_HOT_PLUG_EVT,SCIMode_Normal);
				BAT1_CHG &= ~Charging;
				BAT1_CHG &= ~RemCap;	
				BAT1_CHG &= ~Fully;
			}
		}
		else
		{
			BAT_LED2_ON();
			BAT_LED1_OFF();
			if((BAT1_CHG & Charging) || (BAT1_CHG & Fully))
				ECQEvent(BAT1_HOT_PLUG_EVT,SCIMode_Normal);
			BAT1_CHG &= ~Charging;
			BAT1_CHG &= ~RemCap;	
			BAT1_CHG &= ~Fully;	
		}
	}
	else
	{
		BAT_LED1_OFF();
		//TF_013--	if((BAT1_RSOC < 6) && SystemIsS0)
		if((BAT1_RSOC < 6) && SystemIsS0 && IS_BATTERY1_IN())	//TF_013++
		{
			BAT_LED_Cnt_Low = 0;
			BAT_LED_Cnt_CLow++;
			if(BAT_LED_Cnt_CLow == 1)
				BAT_LED2_ON();
			else if (BAT_LED_Cnt_CLow == 3)
				BAT_LED2_OFF();
			else if (BAT_LED_Cnt_CLow > 5)
				BAT_LED_Cnt_CLow = 0;
		}
		//TF_013--	else if((BAT1_RSOC < 11) && SystemIsS0)
		else if((BAT1_RSOC < 11) && SystemIsS0 && IS_BATTERY1_IN())	//TF_013++
		{
			BAT_LED_Cnt_CLow = 0;
			BAT_LED_Cnt_Low++;
			if(BAT_LED_Cnt_Low == 1)
				BAT_LED2_ON();
			else if (BAT_LED_Cnt_Low == 11)
				BAT_LED2_OFF();
			else if (BAT_LED_Cnt_Low > 20)
				BAT_LED_Cnt_Low = 0;
		}
		else
		{
			BAT_LED2_OFF();
			BAT_LED_Cnt_Low = 0;
			BAT_LED_Cnt_CLow = 0;
		}
		if(BAT1_RSOC > 0) 
		{
			if((BAT1_CHG ^ Charging)  || (BAT1_CHG & RemCap))
				ECQEvent(BAT1_HOT_PLUG_EVT,SCIMode_Normal);
			BAT1_CHG &= ~RemCap;
			BAT1_CHG |= Charging;	
			BAT1_CHG &= ~Fully;	
		}
		else
		{
			if(!(BAT1_CHG & RemCap))	
				ECQEvent(BAT1_HOT_PLUG_EVT,SCIMode_Normal);
			BAT1_CHG |= RemCap;
			BAT1_CHG |= Charging;	
			BAT1_CHG &= ~Fully;
		}	
	}
}

void Process_KBLED_Control(unsigned char KBLED_CONTROL,unsigned char POW_STATE)
{
	if(KBLED_CONTROL == SET_KBLED_ON)
	{
		switch(POW_STATE)
		{
			case SYSTEM_S0:
			case SYSTEM_S3_S0:
			case SYSTEM_S4_S0:
			case SYSTEM_S5_S0:
				KBLED_ON();
				KBLED_Status = SET_KBLED_ON;
			break;
			case SYSTEM_S3:
			case SYSTEM_S4:
			case SYSTEM_S5:
				KBLED_OFF();
				KBLED_Status = SET_KBLED_OFF;	//J80_001++
			break;
		}
	}
	else
	{
		switch(POW_STATE)
		{
			case SYSTEM_S0:
			//case _PST_S0_to_S0://reboot
			case SYSTEM_S3_S0:
			case SYSTEM_S4_S0:
			case SYSTEM_S3:
			case SYSTEM_S4:
			case SYSTEM_S5:
			case SYSTEM_S5_S0:
				KBLED_OFF();
				KBLED_Status = SET_KBLED_OFF;
			break;
		}
	}
}