/*-----------------------------------------------------------------------------
 * TITLE: OEM_MAIN.C - OEM specific code
 *
 * Some "HOOKS" from CHIPHOOK.C are copied into this file in order to replace
 * (or augment) core functions.
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

void Oem_Hook_Timer1ms(void)
{
	#ifdef UART_Debug
	if(IS_MASK_SET(UART_Buffer_Status, _Send_Ready))
	{
		UART_Send_Byte();
	}
	#endif
}

//------------------------------------------------------------
// Hook 1ms events
//------------------------------------------------------------
void Hook_Timer1msEvent(IBYTE EventId)
{
    #if MailBoxRWECRam
    HandleMailBox();
    #endif
	
    EventManager(EventId);      // Polling system event, EventId is 0 ~ 9 cycle
    
    Oem_SysPowerContrl();       // System Power Control
    SMBusCenter(); 

	//Label:BLD_TIPIA_20161118_021
	#if Lenovo_Support
	Lenovo_PM_Cmd();
	#endif

	#if Support_ANX7447
	ANX_HOOK_1ms();
	#endif
}
//------------------------------------------------------------
// Hook 5ms events
//------------------------------------------------------------
void Hook_Timer5msEvent(void)
{
    #if ACPI_QEventPending
    Get_PendingECQEvent();
    #endif

	if (EC_EXT_CMD)
	{
		EXT_EC_CMD();
	}

	if(SYS_state & BIT2)
	{
		S3_state = 0;
		Oem_TriggerS0S5(SC_S0SLPOff);
	}	
	else if(SYS_state & BIT0)
	{
		S3_state = 0;	
		Oem_TriggerS0S3(0x30);	
	}	
	else if(SYS_state & BIT3)	
	{
		S3_state = 0;	
	    if(restartStep == 0)
			Oem_TriggerS0S5(SC_S0SLPOff);
		else if(restartStep == 1)
		{
			SYS_state = SYS_state & 0xF7;
			restartStep = 0;
			Oem_TriggerS5S0();
		}
		else
			restartStep--;
	}

	if(LCD_CTRL == 0xAA)
		LCD_BLT_OFF();
	else
		LCD_BLT_ON();

	if((IS_PEXWAKE_LO()) && SystemIsS3 && (!Read_LID()))
		Oem_TriggerS3S0();
}

//------------------------------------------------------------
// Hook 10ms events
//------------------------------------------------------------
void Hook_Timer10msEventA(void)
{
	#if BAT1FuncSupport
    Battey1ControlCenter();
    #endif

	//TF_002++>>
	if(IS_MASK_SET(xbBIOS_Notify_Flag,LANDisable))
		LAN_OFF();
	else
		LAN_ON();
	//TF_002++<<
	//TF_002A++>>
	if(OS_BIOS & LCD_flag)
	{
		BL_ENABLE_EC_HI();
		OS_BIOS &= 0xBF;
	}
	//TF_002A++<<
}

//------------------------------------------------------------
// Hook 10ms events
//------------------------------------------------------------
void Hook_Timer10msEventB(void)
{
	#if Support_ANX7447
	ANX_HOOK_10ms();
	#endif
}

//------------------------------------------------------------
// Hook 50ms events A
//------------------------------------------------------------
void Hook_Timer50msEventA(void)
{
	#if LEDFuncSupport
		ProcessLED();
	#endif
}

//------------------------------------------------------------
// Hook 50ms events B
//------------------------------------------------------------
void Hook_Timer50msEventB(void)
{

	if(Read_ACIN())
	{
		if(PollingChargerIC() == 0x00) //0x00 is polling charge OK
		{
			SET_MASK (CHG_STATUS, Charge_Polling_OK);
			CLEAR_MASK (CHG_STATUS, Charge_Polling_FAIL);
		}
		else
		{
			SET_MASK (CHG_STATUS, Charge_Polling_FAIL);
			CLEAR_MASK (CHG_STATUS, Charge_Polling_OK);
		}	
		if(CHARGE_OP != 0xE008)
		{
			CHARGE_OP = 0xE008;
			CHARGE_OP1 = 0;
			bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerOption0, &CHARGE_OP1, SMBus_NoPEC);
		}		
	}
	else
	{
		if((ACIN_FLAG) && (CHARGE_OP1 != 0xE009))
		{
			CHARGE_OP1 = 0xE009;
			CHARGE_OP = 0;
			bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerOption0, &CHARGE_OP1, SMBus_NoPEC);
		}
	}
}

//------------------------------------------------------------
// Hook 50ms events C
//------------------------------------------------------------
void Hook_Timer50msEventC(void)
{
	if(BAT1_SOC_OLD != BAT1_RSOC)
	{
		BAT1_SOC_OLD = BAT1_RSOC;
		ECQEvent(BAT1_HOT_PLUG_EVT,SCIMode_Normal);
	}
}

//------------------------------------------------------------
// Hook 100ms events A
//------------------------------------------------------------
void Hook_Timer100msEventA(void)
{	
	#if ADCFuncSupport
	if(SystemIsS0)
	{
		ScanAll_ADCChannel();
	}
	#endif
}


//------------------------------------------------------------
// Hook 100ms events B
//------------------------------------------------------------
void Hook_Timer100msEventB(void)
{
	#if SmartFanSupport
	if(SystemIsS0)
	{
		if(FANTEST == 0)	
			FanManager();
		GetRPM1();
		GetRPM2();
	}
	else
	{
		FanFullOff();
	}
    #endif

	//TF_003++>>
	if(IS_MASK_CLEAR(FANerror,fanerror) && SystemIsS0)
	{
		if((RPM1 == 0) && (FanValue != 0))
			SET_MASK(FANerror,fan1error);
		else
			CLEAR_MASK(FANerror,fan1error);
		if((RPM2 == 0) && (Fan2Value != 0))
			SET_MASK(FANerror,fan2error);
		else
			CLEAR_MASK(FANerror,fan2error);
	}
	else
		CLEAR_MASK(FANerror,fanerror);
	//TF_003++<<
}

//------------------------------------------------------------
// Hook 100ms events C
//------------------------------------------------------------
void Hook_Timer100msEventC(void)
{

}

//------------------------------------------------------------
// Hook 500ms events A
//------------------------------------------------------------
void Hook_Timer500msEventA(void)
{
	#if OEM_GPU_ITS_SUPPORT
	CheckGPUInitok();
	#endif
	if((S3_auto == 0) && (SysPowState==SYSTEM_S3))
		S3_auto = S3_auto_L*120; 
	if(S3_auto == 1)	
		Oem_TriggerS3S0();
	else if(S3_auto > 0)	
		S3_auto--;

	//TF_001++>>
	if(auto_S3 == 1)	
		Oem_TriggerS3S0();
	else if((auto_S3 > 0) && (SysPowState==SYSTEM_S3))	
			auto_S3--;
	if(auto_S4 == 1)
		Oem_TriggerS5S0();
	else if((auto_S4 > 0) && (SysPowState==SYSTEM_S5))	
			auto_S4--;
	//TF_001++<<
}

//------------------------------------------------------------
// Hook 500ms events B
//------------------------------------------------------------
void Hook_Timer500msEventB(void)
{
	if(CheckEnterDeepSleep())
		EnterDeepSleep();
}

//------------------------------------------------------------
// Hook 500ms events C
//------------------------------------------------------------
void Hook_Timer500msEventC(void)
{
	temp_cmd[0] = 0x01;
	temp_cmd[1] = 0X04;
	temp_cmd[2] = 0X0F;
	temp_cmd[3] = 0X00;
	temp_cmd[4] = 0X00;
	temp_cmd[5] = 0X8E;
	I2C_WriteStream(2,0x82,&temp_cmd[0],6); 
	temp_cmd[0] = 0x02;
	temp_cmd[1] = 0X04;
	temp_cmd[2] = 0XC0;
	temp_cmd[3] = 0X30;
	temp_cmd[4] = 0X00;
	temp_cmd[5] = 0X14;
	I2C_WriteStream(2,0x82,&temp_cmd[0],6); 
	temp_cmd[0] = 0x01;
	temp_cmd[1] = 0X04;
	temp_cmd[2] = 0X0F;
	temp_cmd[3] = 0X00;
	temp_cmd[4] = 0X00;
	temp_cmd[5] = 0X8F;
	I2C_WriteStream(2,0x82,&temp_cmd[0],6);
	if(bRSMBusBlock(2,SMbusRBK,0x83,0x03,&dGPU_Temp[0]))
		GPUTm = ((dGPU_Temp[2] >> 1) + (dGPU_Temp[1] << 7));
}

//------------------------------------------------------------
// Hook 1sec events A
//------------------------------------------------------------
void Hook_Timer1SecEventA(void)
{
	#if Lenovo_Support

	if(BIOS_OEM_battery_health&HOST_BAT_LIFE_TEST_MODE)
	{
		BIOS_OEM_battery_health&=~HOST_BAT_LIFE_TEST_MODE;

		Bat_Test_Mode_f = 1;//close led except power led and change thermal table
	}
	#endif
    #if SmartFanSupport
    if(SystemIsS0)
    {
        CheckFanRPM1();
    }
    #endif
    #ifdef PECI_Support
    if(SystemIsS0)
    {
        #if ReadDRAMTemperature
        PECI_ReadDRAMTemperature();
        #endif
    }
    #endif
}

//------------------------------------------------------------
// Hook 1sec events B
//-------------------------- ----------------------------------
void Hook_Timer1SecEventB(void)
{
    #if GetCPUTempSupport
    ReadCPUTemp();
    #endif
    #if SmartFanSupport
    if(SystemIsS0)
    {
        CheckFanRPM2();
    }
    #endif
	if(SystemIsS0)
	{
		OEM_2S_COUNT^=0x01;
		if(OEM_2S_COUNT)
		{				
			SMBus1Flag5 |= tempSensorflag;
		}
		DebugTCPU = CpuTm;
	}
}

//------------------------------------------------------------
// Hook 1sec events C
//------------------------------------------------------------
void Hook_Timer1SecEventC(void)
{
	//MoniterTemprature();

	#if ADCFuncSupport
	if(SystemIsS0)  // 正常开机工作状态
	{
		// 获取热敏电阻阻值
		Get_Thermistor0_data();
		Get_Thermistor1_data();
		Get_Thermistor7_data();
	}
	#endif
    #if GetSysTempSupport
    ReadSysTemp();
    #endif
	MoniterTemprature();

	//TF_004++>>
	if((Warning_flag == 0) && SystemIsS0)
	{
		if(EC_beep == 0)
		{
			PC_BEEP_PWM=0x20;
			EC_beep = 1;
		}
		else
		{
			PC_BEEP_PWM=0x00;
			EC_beep = 0;
		}
	}
	else
	{
		PC_BEEP_PWM=0x00;
		Warning_flag = 0xFF;
		EC_beep = 0;
	}
	//TF_004++<<
}

//------------------------------------------------------------
// Hook1min events
//------------------------------------------------------------
void Hook_Timer1MinEvent(void)
{

}

//------------------------------------------------------------
// service_OEM_1
//------------------------------------------------------------ 
void service_OEM_1(void)
{
}

//------------------------------------------------------------
// service_OEM_2
//------------------------------------------------------------
void service_OEM_2(void)
{
}

//------------------------------------------------------------
// service_OEM_3
//------------------------------------------------------------
void service_OEM_3(void)
{
}

//------------------------------------------------------------
// service_OEM_4
//------------------------------------------------------------
void service_OEM_4(void)
{
}

//------------------------------------------------------------
//
//------------------------------------------------------------
void Hook_main_service_H(void) 
{
}

//------------------------------------------------------------
//
//------------------------------------------------------------
void Hook_main_service_L(void) 
{
}

//------------------------------------------------------------
// Timer event control function
// Return :
//  Only_Timer1msEvent : Only Timer1msEvent and Hook_Timer1msEvent function
//  All_TimerEvent : All timer events are OK.
//------------------------------------------------------------
BYTE Hook_Only_Timer1msEvent(void)
{
	// 关机
    if((SysPowState==SYSTEM_S5_S0)||(SysPowState==SYSTEM_S4_S0)||(SysPowState==SYSTEM_S3_S0))
    {
		// 长按 4.4s 关机
        if(++PowerOnWDT >4400)  // > 4.4 sec
        {
            PowerOnWDT = 0x00;
            Oem_TriggerS0S5(SC_PowerOnWatchDog);
			
        }
        return(Only_Timer1msEvent);
    }
    else
    {
        PowerOnWDT = 0x00;
    }
    
    #if PowerSequnceOnly    
    return(Only_Timer1msEvent);
    #endif

// Return :
//  Only_Timer1msEvent : Only Timer1msEvent and Hook_Timer1msEvent function
//  All_TimerEvent : All timer events are OK.
    return(All_TimerEvent);
}

//----------------------------------------------------------------------------
// OEM_SkipMainServiceFunc
//  Note:
//      1. Always return(0xFF|Normal_MainService) to run normal main_service function.
//      2. If you don't understand the use of OEM_SkipMainServiceFunc function, don't change anything.
//----------------------------------------------------------------------------
BYTE OEM_SkipMainServiceFunc(void)
{
    
    // Return :
    return(Normal_MainService);
}
