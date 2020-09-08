/*----------------------------------------------------------------------------
 * Filename: OEM_Power.C  For Chipset: ITE.IT85XX
 *
 * Function: Power sequnce control function example
 *
 * Author  : Dino
 * 
 * Copyright (c) 2009 - , ITE Tech. Inc. All Rights Reserved. 
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

#if 1
#define POWER_RAMDEBUG(x)   RamDebug(x)
#else
#define POWER_RAMDEBUG(x)
#endif

void NULL_FUNCTION(void)
{

}

//======================================================
// FT power sequence
// 2018-03-31
//======================================================
void PWR_S5_EN_H(void)
{
	PWR_S5_EN_HI();
	POWER_RAMDEBUG(0x01);
}

void PWR_S5_EN_L(void)
{
	if(IS_MASK_CLEAR(xbBIOS_Notify_Flag,typecChargeDisable))	//TF_009++
		PWR_S5_EN_LO();
	POWER_RAMDEBUG(0x02);
}

BYTE STB_PWRGD_OK(void)
{
	if(Read_STB_PWRGD())
	{
		POWER_RAMDEBUG(0x03);
		return(1);
	}
	POWER_RAMDEBUG(0x04);
	return(0);
}

void ALWON_H(void)
{
	ALWON_ON();
	POWER_RAMDEBUG(0x05);
}

void ALWON_L(void)
{
	ALWON_OFF();
	POWER_RAMDEBUG(0x06);
}

void VCOR_EN(void)
{
	VCORE_EN_HI();
	POWER_RAMDEBUG(0x07);
}

void VCOR_DisEnable(void)
{
	VCORE_EN_LO();
	POWER_RAMDEBUG(0x08);
}

BYTE Wait_VCORE_OK(void)
{
	if(IS_VCORE_OK())
	{
		POWER_RAMDEBUG(0x09);
		return(1);
	}
	POWER_RAMDEBUG(0x0A);
	return(0);
}

void VDD_EN_H(void)
{
	VDD_EN_ON();
	POWER_RAMDEBUG(0x0B);
}

void VDD_EN_L(void)
{
	VDD_EN_OFF();
	POWER_RAMDEBUG(0x0C);
}

BYTE Wait_VDDQ_PG(void)
{
	if(Read_VDDQ_PG())
	{
		POWER_RAMDEBUG(0x0D);
		return(1);
	}
	POWER_RAMDEBUG(0x0E);
	return(0);
}

BYTE Wait_1P8_ON(void)
{
	if(IS_1P8_OK())
	{
		POWER_RAMDEBUG(0x0F);
		return(1);
	}
	POWER_RAMDEBUG(0x10);
	return(0);
}

void VTT_EN_H(void)
{
	EC_VTT_EN_HI();
	POWER_RAMDEBUG(0x11);
}

void VTT_EN_L(void)
{
	EC_VTT_EN_LO();
	POWER_RAMDEBUG(0x12);
}

void PX_EN_H(void)
{
	PX_EN_HI();
	POWER_RAMDEBUG(0x13);
}
	
void PX_EN_L(void)
{
	PX_EN_LO();
	POWER_RAMDEBUG(0x14);
}
void PX_EN2_H(void)
{
	PX_EN2_HI();
	POWER_RAMDEBUG(0x15);
}

void PX_EN2_L(void)
{
	PX_EN2_LO();
	POWER_RAMDEBUG(0x16);
}

void CPU_RST_H(void)
{
	CPU_RST_HI();
	POWER_RAMDEBUG(0x17);
}

void CPU_RST_L(void)
{
	CPU_RST_LO();
	POWER_RAMDEBUG(0x18);
}

void PCIRST_H(void)
{
	PCIRST1_HI();
	PCIRST2_HI();
	//TF_008-->>
	/*
    if(SysPowState==SYSTEM_S0_S3)
		PCIRST3_LO();
	*/
	//TF_008--<<
	PCIRST3_HI();
	POWER_RAMDEBUG(0x19);
}

void PCIRST_L(void)
{
	PCIRST1_LO();
	PCIRST2_LO();
	//TF_008A--	if(SysPowState != SYSTEM_S0_S3)	
	if(((SysPowState != SYSTEM_S0_S3) && (IS_BOARD_ID1_LO())) || (IS_BOARD_ID1_HI()))	//TF_008A++	
		PCIRST3_LO();
	POWER_RAMDEBUG(0x1A);
}

void V1P8_EN_H(void)
{
	V1P8_EN_HI();
	POWER_RAMDEBUG(0x1D);
}

void V1P8_EN_L(void)
{
	V1P8_EN_LO();
	POWER_RAMDEBUG(0x1E);
}

BYTE Wait_ALWGD(void)
{
	if(Read_ALWGD())
	{
		POWER_RAMDEBUG(0x1F);
		return(1);
	}
	POWER_RAMDEBUG(0x20);
	return(0);
}

void FUSB3_PWREN_H(void)
{
	FUSB3_PWREN_HI();
 	FUSB3_PWREN1_HI();
 	POWER_RAMDEBUG(0x21);
}

void FUSB3_PWREN_L(void)
{
   FUSB3_PWREN_LO();
   FUSB3_PWREN1_LO();
   POWER_RAMDEBUG(0x22);
}

//TF_008++>>
void PCIRST3_L(void)
{
	if(IS_BOARD_ID1_LO()) //TF_008A++ 
		PCIRST3_LO();
	POWER_RAMDEBUG(0x23);
}
//TF_008++<<

//-------------------------------------------------------------------------------------
//  Do S5 -> S0 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS5S0[]=  
{
	{ NULL_FUNCTION,	   10,	     0,	},	// Dummy step always in the top of tabel
	{ PWR_S5_EN_H, 		    0,		 0, },	// 1
	//TF_007--	{ STB_PWRGD_OK, 	   20,		 1, },	// 2
	{ STB_PWRGD_OK, 	   80,		 1, },	// 2	//TF_007++
	{ ALWON_H, 		       30,		 0, },	// 3
    { InitSio,		      130,		 0,	},
    { VDD_EN_H, 		   10,		 0, },	// 9
	{ VTT_EN_H, 		    0,		 0, },	// 9
	{ Wait_VDDQ_PG,	       50,		 1,	},  // 5	//J80_012++
	{ VCOR_EN,			   30,		 0,	},  // 4
    { Wait_VCORE_OK,	   20,		 1,	},  // 5 
    { V1P8_EN_H,		    0,		 0,	},  // 4
    { PX_EN_H,		        0,		 0,	},  // 4
    { PX_EN2_H,		       10,		 0,	},  // 4
    { Wait_ALWGD,		  220,		 1,	},  // 7	
    { PCIRST_H,		       20,		 0,	},  // 7
	{ CPU_RST_L, 		   20,		 0, },	// 12
	{ CPU_RST_H, 		    0,		 0, },	// 12
	{ S5_S0Variable,		0,		 0,	}, 	// always in the end of tabel
};

//-------------------------------------------------------------------------------------
//  Do S0 -> S5 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS0S5[]=  
{
	{ NULL_FUNCTION,		0,		 0, },	// Dummy step always in the top of tabel	
	{ CPU_RST_L, 		    0,		 0, },	// 12
	{ PCIRST_L,		        0,		 0,	},  // 7
    { PX_EN2_L,		      220,		 0,	},  // 4
	{ PX_EN_L,		        0,		 0,	},  // 4
	{ V1P8_EN_L,		   30,		 0, },	// 4	//J80_012++
	{ VCOR_DisEnable,	   20,		 0, },	// 4
	{ VTT_EN_L, 		   20,		 0, },	// 9
	{ VDD_EN_L, 		   60,		 0, },	// 9
	//TF_010--	{ ALWON_L,			    0,		 0, },	// 3
	{ ALWON_L,			   60,		 0, },	// 3	//TF_010++
	{ PWR_S5_EN_L, 		    0,		 0, },	// 1	//J80_005++
	{ S0_S5Variable,		0,		 0,	}, 	// always in the end of tabel
}; 

//-------------------------------------------------------------------------------------
//  Do S4 -> S0 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS4S0[]= 
{
	{ 0x0000, 				0,		 0, },	// Dummy step always in the top of tabel
	{ PWR_S5_EN_H, 		    0,		 0, },	// 1
	//TF_007--	{ STB_PWRGD_OK,	       20,		 1,	},  // 2
	{ STB_PWRGD_OK,	       80,		 1,	},  // 2	//TF_007++
	{ ALWON_H, 		       30,		 0, },	// 3
    { InitSio,		      130,		 0,	},
    { VDD_EN_H, 		   10,		 0, },	// 9
	{ VTT_EN_H, 		    0,		 0, },	// 9
	{ Wait_VDDQ_PG,	       50,		 1,	},  // 5	//J80_012++
	{ VCOR_EN,			   30,		 0,	},  // 4
    { Wait_VCORE_OK,	   20,		 1,	},  // 5 
    { V1P8_EN_H,		    0,		 0,	},  // 4
    { PX_EN_H,		        0,		 0,	},  // 4
    { PX_EN2_H,		       10,		 0,	},  // 4
    { Wait_ALWGD,		  220,		 1,	},  // 7	
    { PCIRST_H,		       20,		 0,	},  // 7
	{ CPU_RST_L, 		   20,		 0, },	// 12
	{ CPU_RST_H, 		    0,		 0, },	// 12
	{ S4_S0Variable,		0,		 0,	}, 	// always in the end of tabel

}; 

//-------------------------------------------------------------------------------------
//  Do S0 -> S4 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS0S4[]=  
{
	{ 0x0000, 				0,		0,  },	// Dummy step always in the top of tabel
	{ CPU_RST_L, 		    0,		 0, },	// 12
	{ PCIRST_L,		        0,		 0,	},  // 7
    { PX_EN2_L,		      220,		 0,	},  // 4
	{ PX_EN_L,		        0,		 0,	},  // 4
	{ V1P8_EN_L,		   30,		 0, },	// 4	//J80_012++
	{ VCOR_DisEnable,	   20,		 0, },	// 4
	{ VTT_EN_L, 		   20,		 0, },	// 9
	{ VDD_EN_L, 		   60,		 0, },	// 9
	{ ALWON_L,			    0,		 0, },	// 3
	{ PWR_S5_EN_L, 		    0,		 0, },	// 1	
	{ S0_S4Variable,		0,		 0,	}, 	// always in the end of tabel
}; 

//-------------------------------------------------------------------------------------
//  Do S3 -> S0 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS3S0[]= 
{
	{ 0x0000, 				0,		 0, },	// Dummy step always in the top of tabel
	{ PCIRST3_L, 		    0,		 0, },	// 3		//TF_008++
	{ ALWON_H, 		       30,		 0, },	// 3
	{ Wait_VDDQ_PG,	       20,		 1,	},  // 5 
	{ VCOR_EN,			   30,		 0,	},  // 4
    { Wait_VCORE_OK,	   20,		 1,	},  // 5 
    { V1P8_EN_H,		    0,		 0,	},  // 4
    { PX_EN_H,		        0,		 0,	},  // 4
    { PX_EN2_H,		       10,		 0,	},  // 4
    { Wait_ALWGD,		  220,		 1,	},  // 7	
    { PCIRST_H,		       20,		 0,	},  // 7
	{ CPU_RST_L, 		   20,		 0, },	// 12
	{ CPU_RST_H, 		    0,		 0, },	// 12
	{ S3_S0Variable,		0,		 0,	}, 	// always in the end of tabel

}; 

//-------------------------------------------------------------------------------------
//  Do S0 -> S3 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS0S3[]=  
{
	{ 0x0000, 				0,		 0, },	// Dummy step always in the top of tabel
	{ CPU_RST_L,			0,		 0, },	// 12
	{ PCIRST_L, 			0,		 0, },	// 7
	{ PX_EN2_L, 		  220,		 0, },	// 4
	{ PX_EN_L,				0,		 0, },	// 4
	{ V1P8_EN_L,		   30,		 0, },	// 4
	{ VCOR_DisEnable,	   80,		 0, },	// 4	
	{ ALWON_L,			   50,		 0, },	// 3
	{ S0_S3Variable,		0,		 0,	}, 	// always in the end of tabel
}; 

//-----------------------------------------------------------------------------
//  The OEM variable of Sx -> S0
//-----------------------------------------------------------------------------
void OEM_SX_S0CommVar(void)
{
    CheckPowerStateIndex = 0x00;	
	
}

//-----------------------------------------------------------------------------
//  The variable of Sx -> S0
//-----------------------------------------------------------------------------
void SX_S0CommVar(void)
{
	SysPowState=SYSTEM_S0;
	
	PS2DeviceInactive();    // For Touchpad issue
    GPCRF0 = ALT;           // PS2 interface
    GPCRF1 = ALT;
	
	ResetSCIEvent();

    OEM_SX_S0CommVar();
	
    InitSMBus();

	Sys_Power_Status = (Sys_Power_Status & 0x0f0);

	if(Oem_RTP_ID_CMD == 0)
	{
		guoyong003 = 0x99;
		guoyong001 = 0x00;

		Oem_RTP_ID_LOOP_Index = 0;

		Oem_RTP_ID_Step = 0;

		Oem_RTP_ID_1s_EN = 0x01;
		Oem_RTP_ID_1s_T = 0;
	}
	FIN_PWR_EN_ON();
	M2PWR_ENT_ON();
	MUTE_ON();
	CAM_PWR_EN_ON();
	WIFI_ENABLE_ON();
	TPS2546_CTL1_HI();
	BT_EN_ON();
	//TF_002A--	BL_ENABLE_EC_HI();
}

//-----------------------------------------------------------------------------
// The variable of S5 -> S0  开机状态
//-----------------------------------------------------------------------------
void S5_S0Variable(void)
{
	SX_S0CommVar();
	S5S0FanInitOn();

	//==================================
	// USB Power Set on
	//==================================
	// USB postposition port 1-3
	FUSB3_PWREN_H();
	EnableADCModule();
}

//-----------------------------------------------------------------------------
// The variable of S4 -> S0
//-----------------------------------------------------------------------------
void S4_S0Variable(void)
{
    SX_S0CommVar();
	S5S0FanInitOn();
	BL_ENABLE_EC_HI();	//TF_002A++
}

//-----------------------------------------------------------------------------
// The variable of S3 -> S0
//-----------------------------------------------------------------------------
void S3_S0Variable(void)
{
	SX_S0CommVar();
	S3S0FanInitOn();
	BL_ENABLE_EC_HI();	//TF_002A++
}

//-----------------------------------------------------------------------------
//  The OEM variable of S0 -> Sx
//-----------------------------------------------------------------------------
void OEM_S0_SXCommVar(void)
{
    CheckPowerStateIndex = 0x00;
    CLEAR_MASK(SYS_MISC1,ACPI_OS);
}

//-----------------------------------------------------------------------------
//  The variable of S0 -> Sx
//-----------------------------------------------------------------------------
void S0_SXCommVar(void)
{
#ifdef PECI_Support
    PECI_ClearPSTVariables();
#endif
    
#if TouchPad_only
    PowerOffClearTPPendingData();
#endif

	CLEAR_MASK(KBHISR,SYSF);
    Ccb42_SYS_FLAG=0;
	
#if CIRFuncSupport	
	Init_CIR();
#endif

	InitSMBus();
    OEM_S0_SXCommVar();

	SPCTRL1 = Init_I2EC+0x80;

	if(PLLFREQR != PLLFreqSetting03)
	{
		ChangePLLFrequency(PLLFreqSetting03);
	}	
	FIN_PWR_EN_OFF();
	M2PWR_ENT_OFF();
	MUTE_OFF();
	CAM_PWR_EN_OFF();
	WIFI_ENABLE_OFF();
	TPS2546_CTL1_LO();
	BT_EN_OFF();
	Process_KBLED_Control(SET_KBLED_ON,SYSTEM_S5);
  	LCD_CTRL = 0;	
	BL_ENABLE_EC_LO();
}

//-----------------------------------------------------------------------------
//  The variable of S0 -> S5  关机状态
//-----------------------------------------------------------------------------
void S0_S5Variable(void)
{
    SysPowState=SYSTEM_S5;

	if(IS_MASK_SET(Oem_For_Bios_Flag,EC_flashed))
	{
		CLEAR_MASK(Oem_For_Bios_Flag,EC_flashed);
		
		BRAM_FLASH_ID0 = 0x55;
		BRAM_FLASH_ID1 = 0xaa;
		BRAM_FLASH_ID2 = 0x55;
		BRAM_FLASH_ID3 = 0xaa;
		
		_nop_();
		_nop_();
		_nop_();
		
		ETWCTRL = 0x10; 	   //EWDCMS  External WDT stop count mode select   1: External WDT can be stopped by setting EWDSCEN bit,	0: External WDT cannot be stopped
		EWDCNTLR = WATCHDOG_COUNTER_2S;//reset ec domain		
		while(1);				// Wait for watch dog time-out
	
	}

    S0_SXCommVar();

	//==================================
	// USB Power Set off
	//==================================
	// USB postposition port 1-3
	FUSB3_PWREN_L();
	CLEAR_MASK(EVT_STATUS2,Sys_Shutdown_f);
	CLEAR_MASK(EVT_STATUS2,Sys_ResetPowerOn_f);

	DisableADCModule();

	if(SYS_state & BIT2)
		SYS_state = SYS_state & 0xFB;
	else if(SYS_state & BIT3)
		restartStep = 200;
	S3_state = 0;	//J80_004++
 	OS_BIOS = 0;//J80_007A++
 	xbBIOS_Notify_Flag &= 0xEF;	//TF_002A++
}



//-----------------------------------------------------------------------------
//  The variable of S0 -> S4
//-----------------------------------------------------------------------------
void S0_S4Variable(void)
{
    SysPowState=SYSTEM_S4;
    S0_SXCommVar();
}

//-----------------------------------------------------------------------------
// The variable of S0 -> S3
//-----------------------------------------------------------------------------
void S0_S3Variable(void)
{
	SysPowState=SYSTEM_S3;
	S0_SXCommVar();
	DRAM_CTRL_LO();	//J80_012++
	SYS_state = SYS_state & 0xFE;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void Oem_TriggerS3S0(void)
{
    if((SystemIsS0) || (SysPowState==SYSTEM_S3_S0))
    {
        return;
    }
	SysPowState=SYSTEM_S3_S0;
	PWSeqStep = 1;
	PowSeqDelay = 0x00;
	DRAM_CTRL_HI();	
	if(S3_auto == 1)
	{
		S3_auto = 0;
		S3_state = 0xAA;
	}
	//TF_001++>>
	else if(auto_S3 == 1)
	{
		auto_S3 = 0;
		//TF_001A--	S3_state = 0xBB;
		//TF_008--	S3_state = 0xAA;	//TF_001A++
		S3_state = 0x55;	//TF_008++
	}
	//TF_001++<<
	else
		S3_state = 0x55;	
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void Oem_TriggerS4S0(void)
{
    if((SystemIsS0) || (SysPowState==SYSTEM_S4_S0))
    {
        return;
    }
    SysPowState=SYSTEM_S4_S0;
    PWSeqStep = 1;
    PowSeqDelay = 0x00;
}



//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void Oem_TriggerS5S0(void)
{
    if((SystemIsS0) || (SysPowState==SYSTEM_S5_S0))
    {
        return;
    }
    SysPowState=SYSTEM_S5_S0;
	PWSeqStep = 1;
	PowSeqDelay = 0x00;
	auto_S4 = 0;	//TF_001++	
}

//----------------------------------------------------------------------------
// S5 to S0 sequnce
//----------------------------------------------------------------------------
void Oem_S5S0Sequence(void)
{
    if ( PowSeqDelay != 0x00 )
    {
        PowSeqDelay--; 				//1m Sec count base
        if ( PowSeqDelay != 0x00 )
        {
            return;
        } 
    }

	while(PWSeqStep<(sizeof(asPowerSEQS5S0)/sizeof(sPowerSEQ)))
	{	
		if(asPowerSEQS5S0[PWSeqStep].checkstatus==0x00)	// Do function
		{
			(asPowerSEQS5S0[PWSeqStep].func)();
			PowSeqDelay = asPowerSEQS5S0[PWSeqStep].delay;
			PWSeqStep++;
			if(PowSeqDelay!=0x00)
			{
				break;
			}
		}
		else											// Check input status pin
		{
			if((asPowerSEQS5S0[PWSeqStep].func)())
			{
				PowSeqDelay = asPowerSEQS5S0[PWSeqStep].delay;
			    PWSeqStep++;
			}  
			break;
		}	
	}
}

//----------------------------------------------------------------------------
// S4 to S0 sequnce
//----------------------------------------------------------------------------
void Oem_S4S0Sequence(void)
{
    if ( PowSeqDelay != 0x00 )
    {
        PowSeqDelay--; 				//1m Sec count base
        if ( PowSeqDelay != 0x00 )
        {
            return;
        }
    }

    while(PWSeqStep<(sizeof(asPowerSEQS4S0)/sizeof(sPowerSEQ)))
    {	
        if(asPowerSEQS4S0[PWSeqStep].checkstatus==0x00)	// Do function
        {
            (asPowerSEQS4S0[PWSeqStep].func)();
            PowSeqDelay = asPowerSEQS4S0[PWSeqStep].delay;
            PWSeqStep++;
            if(PowSeqDelay!=0x00)
            {
                break;
            }
        }
        else											// Check input status pin
        {
            if((asPowerSEQS4S0[PWSeqStep].func)())
            {
                PowSeqDelay = asPowerSEQS4S0[PWSeqStep].delay;
                PWSeqStep++;
            }
            break;
        }	
    }
}

//----------------------------------------------------------------------------
// S3 to S0 sequnce
//----------------------------------------------------------------------------
void Oem_S3S0Sequence(void)
{
    if ( PowSeqDelay != 0x00 )
    {
        PowSeqDelay--; 				//1m Sec count base
        if ( PowSeqDelay != 0x00 )
        {
            return;
        }
    }

	while(PWSeqStep<(sizeof(asPowerSEQS3S0)/sizeof(sPowerSEQ)))
	{	
		if(asPowerSEQS3S0[PWSeqStep].checkstatus==0x00)	// Do function
		{
			(asPowerSEQS3S0[PWSeqStep].func)();
			PowSeqDelay = asPowerSEQS3S0[PWSeqStep].delay;
			PWSeqStep++;
			if(PowSeqDelay!=0x00)
			{
				break;
			}
		}
		else											// Check input status pin
		{
			if((asPowerSEQS3S0[PWSeqStep].func)())
			{
				PowSeqDelay = asPowerSEQS3S0[PWSeqStep].delay;
				PWSeqStep++;
			}
			break;
		}	
	} 
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void Oem_TriggerS0S3(BYTE causecode)
{
    if((SystemIsS3) || (SysPowState==SYSTEM_S0_S3))
    {
        return;
    }
    
	ShutDnCause = causecode;		// setup shutdown case
	SysPowState=SYSTEM_S0_S3;
	PWSeqStep = 1;
	PowSeqDelay = 0x00;
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void Oem_TriggerS0S4(BYTE causecode)
{
    if((SystemIsS4) || (SysPowState==SYSTEM_S0_S4))
    {
        return;
    }
    
    ShutDnCause = causecode;		// setup shutdown case
    SysPowState=SYSTEM_S0_S4;
    PWSeqStep = 1;
    PowSeqDelay = 0x00;
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void Oem_TriggerS0S5(BYTE causecode)
{
    if((SystemIsS5) || (SysPowState==SYSTEM_S0_S5))
    {
        return;
    }

	ShutDnCause = causecode;		// setup shutdown case 
    SysPowState=SYSTEM_S0_S5;
	PWSeqStep = 1;
	PowSeqDelay = 0x00;
}

//-----------------------------------------------------------------------------
// process POWER OFF sequence
// entry: 	1�Bpower shutdown case
//			2�BS5 off or not
// Shutdown cause:
//	1:	0x8A command of 66 port
// 	2:	0x8B command of 66 port
//	3:	4secs Overrided
// 	4:	HWPG low
//	5:	0x8C command of 66 port
// 	6:	0x8D command of 66 port
//	7:	temperature to high
// 	8:
//	9:	Init. thermal chip fail
// 	10:	Check SLP // S0 -> S4
//	11:	Check SLP // S3 -> S5
// 	12:	Check SLP // S0 -> S5

//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// S0 to S3 sequnce
//----------------------------------------------------------------------------
void Oem_S0S3Sequence(void)
{
    if ( PowSeqDelay != 0x00 )
    {
        PowSeqDelay--; 				//1m Sec count base
        if ( PowSeqDelay != 0x00 )
        {
            return;
        }
    }

	while(PWSeqStep<(sizeof(asPowerSEQS0S3)/sizeof(sPowerSEQ)))
	{	
		if(asPowerSEQS0S3[PWSeqStep].checkstatus==0x00)	// Do function
		{
			(asPowerSEQS0S3[PWSeqStep].func)();
			PowSeqDelay = asPowerSEQS0S3[PWSeqStep].delay;
			PWSeqStep++;
			if(PowSeqDelay!=0x00)
			{
				break;
			}
		}
		else											// Check input status pin
		{
			if((asPowerSEQS0S3[PWSeqStep].func)())
			{
				PowSeqDelay = asPowerSEQS0S3[PWSeqStep].delay;
				PWSeqStep++;
			}
			break;
		}	
	}           
}


//----------------------------------------------------------------------------
// S0 to S4 sequnce
//----------------------------------------------------------------------------
void Oem_S0S4Sequence(void)
{
    if ( PowSeqDelay != 0x00 )
    {
        PowSeqDelay--; 				//1m Sec count base
        if ( PowSeqDelay != 0x00 )
        {
            return;
        }
    }

    while(PWSeqStep<(sizeof(asPowerSEQS0S4)/sizeof(sPowerSEQ)))
    {	
        if(asPowerSEQS0S4[PWSeqStep].checkstatus==0x00)	// Do function
        {
            (asPowerSEQS0S4[PWSeqStep].func)();
            PowSeqDelay = asPowerSEQS0S4[PWSeqStep].delay;
            PWSeqStep++;
            if(PowSeqDelay!=0x00)
            {
                break;
            }
        }
        else											// Check input status pin
        {
            if((asPowerSEQS0S4[PWSeqStep].func)())
            {
                PowSeqDelay = asPowerSEQS0S4[PWSeqStep].delay;
                PWSeqStep++;
            }
            break;
        }	
    }       
}

//-----------------------------------------------------------------
// S0 to S5 sequnce
//-----------------------------------------------------------------
void Oem_S0S5Sequence(void)
{
    if ( PowSeqDelay != 0x00 )
    {
        PowSeqDelay--; 				//1m Sec count base
        if ( PowSeqDelay != 0x00 )
        {
            return;
        }
    }

	while(PWSeqStep<(sizeof(asPowerSEQS0S5)/sizeof(sPowerSEQ)))
	{	
		if(asPowerSEQS0S5[PWSeqStep].checkstatus==0x00)	// Do function
		{
			(asPowerSEQS0S5[PWSeqStep].func)();
			PowSeqDelay = asPowerSEQS0S5[PWSeqStep].delay;
			PWSeqStep++;
			if(PowSeqDelay!=0x00)
			{
				break;
			}
		}
		else											// Check input status pin
		{
			if((asPowerSEQS0S5[PWSeqStep].func)())
			{
				PowSeqDelay = asPowerSEQS0S5[PWSeqStep].delay;
				PWSeqStep++;
			}
			break;
		}	
	}  
}

//J80_011++>>
/* ----------------------------------------------------------------------------
 * FUNCTION: check Enter Deep Sleep mode
 *      Timer base : 500 ms
 * ------------------------------------------------------------------------- */
BYTE CheckEnterDeepSleep(void)
{
	BYTE resault;
	resault = 0x00;
    
    if(SystemIsS0)		                        // if system in S0
    {
		BRAM_EnterDeep = 0x11;
		resault = 0x01;
    }	
    if(SystemIsS3)		                        // if system in S3
    {
		BRAM_EnterDeep = 0x33;
		resault = 0x01;	
    }
	if((SystemNotS5) &&	(SystemNotS4))	
		resault = 0x01;	
	//TF_001++>>
	if(auto_S4 > 0)
		resault = 0x01;
	//TF_001++<<
	if((Sys_Power_Status & 0x7f) == 0x03)
	{
		resault = 0x01;	
	}
	
    if(IS_MASK_SET(POWER_FLAG1,adapter_in))//&&(IS_MASK_SET(BT1_STATUS1,bat_in)))		// AC power exist
    {
		BRAM_EnterDeep = 0x22;
		resault = 0x01;	
    }
    if(IS_MASK_SET(POWER_FLAG1,wait_PSW_off))	// power switch pressed
    {
        resault = 0x01;	
    }
	
	if(Oem_BatCtrl_F & Oem_BatCapTooLow_NotS0_f)
	{
		resault = 0x01;	
	}

	if(IS_MASK_SET(POWER_FLAG1,wait_NovoBtn_off))
	{
        resault = 0x01;	
    }
	
	if(resault == 0x00)
	{
		if(DeepSleepCunt<10)	
		{
			DeepSleepCunt++;
			resault = 0x02;
		}
		else
		{
			resault = 0x00;
			DeepSleepCunt = 0x00;
		}
	}
    else
    {
        DeepSleepCunt = 0x00;
		ISR15 = ISR15 & 0xFB;	
        IER15 = IER15 & 0xFB;
    }

	switch(resault)
	{
		case 0:
			return(1);
			break;

		case 1:
			DeepSleepCunt=0x00;
			return(0);
			break;

		case 2:
			return(0);
			break;
	}
	return(0);
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Enter Deep Sleep mode
 * ------------------------------------------------------------------------- */
void EnterDeepSleep(void)
{
	//If BIOS notify keep always power, then not let EC enter deep sleep
	if(IS_MASK_SET(SYS_MISC1,OFF_ALW))
		return;

	BRAM_EnterDeep1 = 0x88;
//++++++++++++++++++++++++++++
	Oem_Globe_Flag &= ~Oem_AlwaysOn_f;
	RamDebug(0x81);
//++++++++++++++++++++++++++++

	IER0=0x00;
	IER1=0x00;
	IER2=0x00;
	IER3=0x00;
	IER4=0x00;
	IER9=0x00;
	IER15=0x00;
	
	DisableAllInterrupt();
							// Disable module
	DACPWRDN=0x3F;			// Disable DAC	

	KSOL = 0xFF;
	KSOH1 = 0xFF;
	KSOH2 = 0xFF;

	CGCTRL1R = 0xFF;
	CGCTRL2R = 0xFF;
	CGCTRL3R = 0xFF;

	//*************************************************************************
	// Set Wake up pin -> alt function
	//*************************************************************************
	GPCRE4 = ALT;			// pwrsw to alternate pin
	WUEMR2 |= 0x20;
	WUESR2 |= 0x20;			// clear wakeup status
	WUENR2 |= 0x20;  
	ISR1 |= Int_WKO25;		// clear int14 for  pwrsw
	IER1 |= Int_WKO25;		// enable int14 for  pwrsw
   	WUEMR4 |= 0x01;         // Wake-up Edge WU40 for NOVO	 
   	WUESR4 |= 0x01;
	WUENR4 |= 0x01;         // Wake-up enable WU40 for NOVO Button
	ISR0 |= Int_WKINTAD; 	//  (Int_WKO20+Int_WKINTAD);     // clear int 1 for  SUSB
   							            // clear int 5 for  NOVO button	
   	IER0 |= Int_WKINTAD;    //(Int_WKO20+Int_WKINTAD);    // enable int 1 for  SUSB
   	
	if(Read_BatIN())        // Battery  in system
   	{
		GPCRI3 = INPUT;	
		//WUEMR13 |= 0x04;
		//WUESR13 |= 0x04;
        ISR15 |= Int_WKO122;	
        IER15 |= Int_WKO122;
	}
	else                   //AC in  system 
	{	
		GPCRI4 = INPUT;       //battery in   WU20   INT1 
		WUEMR7 |= 0x10;
		WUESR7 |= 0x10;
		WUENR7 |= 0x10;
		ISR9 |= Int_WKO74;
		IER9 |= Int_WKO74;
     }


	//note:to check whether need to go to battery shipmode before the system go to G2 
	 CheckBatShipMode();
	
	_nop_();
	_nop_();

	RamDebug(0x11);

	EnableAllInterrupt();
	PLLCTRL = 1;
	PCON =2 ;             	// enter sleep mode

	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	RamDebug(0x22);

	
	IER0=0x00;				
	IER1=0x00;
	IER2=0x00;
	IER3=0x00;
	IER4=0x00;
	IER15=0x00;
	
	IER1=0x00;
	ISR1=0xff;
	
	IER2=0x00;
	ISR2=0xff;
	
	IER0=0x00;
	ISR0=0xff;
	

	WUENR1 = 0x00;	
	WUENR2 = 0x00;	
	WUENR3 = 0x00;	
	WUENR4 = 0x00;	
	WUENR7 = 0x00;
	
	WUESR1 = 0xff;
	WUESR2 = 0xff;
	WUESR3 = 0xff;
	WUESR4 = 0xff;
	WUESR7 = 0xFF;
	WUESR13 = 0xFF;
	
	//*************************************************************************
	//
	//************************************************************************* 
	GPCRI3 = INPUT; 		// ACIN to alternate pin
	GPCRE4 = INPUT; 		// pwrsw to alternate pin
	GPCRI4 = INPUT; 		// battery in
	//*************************************************************************

	CGCTRL1R = 0x00;
	CGCTRL2R = 0x00;
	CGCTRL3R = 0x00;

	DACPWRDN=0x00;			// enable DAC	
	Enable_Any_Key_Irq();	// Enable Keyboard
	Init_Timers();
}
//J80_011++<<

//-----------------------------------------------------------------------------
// The function of checking HWPG
//-----------------------------------------------------------------------------
void MoniterHWPG(void)
{

	return;
	/*
    if(SystemIsS0)
    {
        if(!Read_ALWGD()) 
        {
            HWPG_TIMER++;
            if(HWPG_TIMER>T_HWPG)
            {
                Oem_TriggerS0S5(SC_HWPS0Off);
                HWPG_TIMER = 0x00;
            }
        }
        else
        {
             HWPG_TIMER = 0x00;
        }
    }
    */
}

//----------------------------------------------------------------------------
// EC auto power on function
//----------------------------------------------------------------------------
void CheckAutoS5S0(void)
{
	if(ECAutoPowerOn==1)
	{
		ECAutoPowerOn = 0;
		DelayDoPowerSeq = 1000;
		Oem_TriggerS5S0();
	}
	
	if(ECWDTRSTNow)
	{
		ECWDTRSTNow = 0;
		SysPowState = SYSTEM_EC_WDTRST;
	}
}


//-----------------------------------------------------------------------------
// Handle System Power Control
// Called by service_1mS -> Timer1msEvent -> Hook_Timer1msEvent
//-----------------------------------------------------------------------------
void Oem_SysPowerContrl(void)
{
    // Check if delay doing PowerSeq is needed.
    if ( DelayDoPowerSeq != 0x00 )
    {
        DelayDoPowerSeq--;
        if ( DelayDoPowerSeq != 0x00 )
        {
            return;
        }
    }

    // Check current System PowerState to see what should we do.
    switch (SysPowState)
    {
        case SYSTEM_S0: 
			MoniterHWPG();
			break;

        case SYSTEM_S3:			
         	break;

        case SYSTEM_S4:
            break;

        case SYSTEM_S5:
			CheckAutoS5S0();
            break;

        case SYSTEM_S4_S0:
            Oem_S4S0Sequence();
            break;

        case SYSTEM_S5_S0:
			
			Oem_S5S0Sequence();
			break;
				
        case SYSTEM_S3_S0:
			Oem_S3S0Sequence();
            break;

        case SYSTEM_S0_S3:
			Oem_S0S3Sequence();
            break;

        case SYSTEM_S0_S4:
            Oem_S0S4Sequence();
            break;

        case SYSTEM_S0_S5:
			Oem_S0S5Sequence();
         	break;

		case SYSTEM_EC_WDTRST:
			InternalWDTNow();
			break;

        default:
         	SysPowState=SYSTEM_S5;
         	break;
    }
}


void CheckResetSource(void)
{
    switch(RSTStatus&0x03)
    {
        case 0:
        case 1:
            if(GPCRA0==0x84)
            {
                ShutDnCause = SC_ECColdBoot;
				ClearBRam();
            }
            else
            {
                ShutDnCause = SC_EC_0x0000;
            }
            break;
            
        case 2:
            if(GPCRA0==0x84)
            {
                ShutDnCause = SC_IntWatchDog;
            }
            else
            {
                ShutDnCause = SC_EC_0x0000;
            }
            break;         

        case 3:
            if(GPCRA0==0x84)
            {
                ShutDnCause = SC_ExtWatchDog;
				#if BRAM_Debug
				Copy_BRAM_SRAM();
				#endif				
            }
            else
            {
                ShutDnCause = SC_EC_0x0000;
            }
            
            break;
    }
}

//-----------------------------------------------------------------------------
// The function of pulsing S.B. power button 
//-----------------------------------------------------------------------------
void PulseSBPowerButton(void)
{
	if(0==SBSWReleaseCunt)      // Prevent continued pull low power signal to chipset
	{
		SBSWReleaseCunt = 20;   // decrease timer base is 10ms total time 200ms
	}
}

//-----------------------------------------------------------------------------
// The function of releasing S.B. power button  10ms
//-----------------------------------------------------------------------------
void CheckSBPowerButton(void)
{
	if(SBSWReleaseCunt!=0x00)
	{
		SBSWReleaseCunt--;
	}
}


