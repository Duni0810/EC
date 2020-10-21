/*-----------------------------------------------------------------------------
 * TITLE: OEM_EVENT.C
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 * 
 *     这个文件好像是说明电源事件的
 *---------------------------------------------------------------------------*/

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>


// 这个函数没有被调用
void Auto_NOTIFY_EM(BYTE adata)
{
	VPC_Lenovo|=Genenal_Bit_vpc;
	
	if(adata)
	{
		Lenvo_general_f|=auto_mode_f;
	}
	else
	{
		Lenvo_general_f&=~auto_mode_f;
	}
	//J80_013--	ECQEvent(NOTIFY_EM_EVT,SCIMode_Normal);
}

// 这个函数没有被调用
void Check_notify_em(void)
{
	if(Device_Status_AllRF&Super_performance_F)
	{
		VPC_Lenovo|=Genenal_Bit_vpc;
		Lenvo_general_f|=super_temp_f;
		//J80_013--	ECQEvent(NOTIFY_EM_EVT,SCIMode_Normal);
	}
}

//-----------------------------------------------------------------------------
// no change function 空事件，空函数
//-----------------------------------------------------------------------------
void NullEvent(void)
{

}

//-----------------------------------------------------------------------------
// Adapter in function  适配器插入
//-----------------------------------------------------------------------------
void AdapterIn(void)
{
	UART_Print_Str("AC plug IN!\n");
	
	SET_MASK(BAT1_STATUS,OS_AC_IN);
	ECQEvent(AC_HOT_PLUG_EVT,SCIMode_Normal);	//J80_013++
}

//-----------------------------------------------------------------------------
// Adapter out function 适配器拔出
//-----------------------------------------------------------------------------
void AdapterOut(void)
{
	UART_Print_Str("AC plug OUT!\n");
	
	CLEAR_MASK(BAT1_STATUS,OS_AC_IN);
	ECQEvent(AC_HOT_PLUG_EVT,SCIMode_Normal);	//J80_013++
}

//-----------------------------------------------------------------------------
//power button was pressed  按下电源按键
//-----------------------------------------------------------------------------
void PSWPressed(void)
{
	if(Read_LID())
       return;
	UART_Print_Str("\nPower button is pressed!\n");
	
	#if BRAM_Debug	
	Copy_BRAM_SRAM();
	#endif

    PSW_COUNTER = T_PSWPAUSE;  //T_PSWOFF;		        // initialize counter

	CLEAR_MASK(Oem_Globe_Flag1,Oem_PCHBTN_En);
     
    switch (SysPowState)
    {
        case SYSTEM_S5 :
 			//if(IS_MASK_SET(POWER_FLAG1, adapter_in))
			{
				Oem_TriggerS5S0();
				SET_MASK(Oem_Globe_Flag1,Oem_PCHBTN_En);
 			}
			break;
        case SYSTEM_S3 :
 			//if(IS_MASK_SET(POWER_FLAG1, adapter_in))
			{
				Oem_TriggerS3S0();
				SET_MASK(Oem_Globe_Flag1,Oem_PCHBTN_En);
 			}
			break;
		case SYSTEM_S0 :
			SHUT_COUNT = 0;
			SET_MASK(Oem_Globe_Flag1,Oem_PCHBTN_En);
			//J80_007A++>>
			if(OS_BIOS & OS_flag)
			{
			//J80_007A++<<
				//J80_007++>>
				e0_prefix_code(0x37,MAKE_EVENT);	// 这个是ACPI扫描码  ，0x37表示的是power
				e0_prefix_code(0x37,BREAK_EVENT);
				//J80_007++<<
			//J80_007A++>>
			}
			else
				Oem_TriggerS0S5(SC_S0SLPOff);
			//J80_007A++<<
			break;
        default :
            break;
    }
}


//-----------------------------------------------------------------------------
//power button was released
//-----------------------------------------------------------------------------
void PSWReleased(void)
{
	UART_Print_Str("\nPower button is released!\n");
}

//-----------------------------------------------------------------------------
//power button was overrided (press or release)   10ms time base
//长按电源按钮， 强制关机
//-----------------------------------------------------------------------------
void PSWOverrided(void)
{
	if(Read_LID())
       return;
	if(PSW_COUNTER != 0 )	
	{									 
		PSW_COUNTER--;   // PSW_COUNTER count down, EC to South Bridge 200ms
	}

	if(SystemIsS0 && (IS_MASK_SET(POWER_FLAG1,wait_PSW_off)))
	{
		if(SHUT_COUNT<400)
		{
			SHUT_COUNT++;
		}
		else
		{
			Oem_TriggerS0S5(SC_PSW_PRESS);
		}
	}
	else
	{
		SHUT_COUNT=0;
	}
}
//-----------------------------------------------------------------------------


void HW_Reset_Press(void)
{
}

// 硬件复位俺就弹起
void HW_Reset_Released(void)
{
	if(SystemIsS0)
	{
		if(IS_MASK_SET(EVT_STATUS2,Sys_ResetPowerOn_f))
		{
			Oem_TriggerS0S5(SC_Sys_Reset);
			SET_MASK(EVT_STATUS2,Sys_ResetPowerOff_f);
			CLEAR_MASK(EVT_STATUS2,Sys_ResetPowerOn_f);
		}
	}
}

//-----------------------------------------------------------------------------
// LID  close  笔记本电脑合上盖子事件
//-----------------------------------------------------------------------------
void LID_CLOSE(void)
{
	SET_MASK(EC_EXT_SOURCE_STATE,LID_CLOSE_Act);
	if(SysPowState == SYSTEM_S0)
	{
		ECQEvent(LID_EVT,SCIMode_Normal);
		Device_Status_Lenovo &= ~Inverter_S_B;
	}
	BL_ENABLE_EC_LO();
}

//-----------------------------------------------------------------------------
// LID  open  打开盖子事件
//-----------------------------------------------------------------------------
void LID_OPEN(void)
{
	EC_EXT_SOURCE_STATE &= ~LID_CLOSE_Act;
				
	ECQEvent(LID_EVT,SCIMode_Normal);		
	Device_Status_Lenovo |= Inverter_S_B;
	
	if(SysPowState == SYSTEM_S3)		
		Oem_TriggerS3S0();
	BL_ENABLE_EC_HI();
}


// ----------------------------------------------------------------------------
// Device insert/remove debounce routine.
// input:   device_id
// ----------------------------------------------------------------------------
const struct sDebounce code Debounce_TBL[] =
{/*
0*/	{&POWER_FLAG1 ,wait_PSW_off      ,&DEBOUNCE_CONT1 ,T_PSW_DEBOUNCE     ,PSWPressed  ,PSWReleased	,PSWOverrided }, /*
1*/	{&POWER_FLAG1 ,adapter_in		 ,&DEBOUNCE_CONT2 ,T_AC_DEBOUNCE      ,AdapterIn   ,AdapterOut	,NullEvent    }, /*
2*/	{&BT1_STATUS1 ,bat_in			 ,&DEBOUNCE_CONT3 ,T_BAT_DEBOUNCE	  ,NullEvent   ,NullEvent	,NullEvent    }, /*
3*/	{&EVT_STATUS1 ,Lid_close_f		 ,&DEBOUNCE_CONT4 ,T_LID_DEBOUNCE     ,LID_CLOSE   ,LID_OPEN    ,NullEvent    },/*
4*/	{&EVT_STATUS1 ,PCI_Wake_f		 ,&DEBOUNCE_CONT5 ,T_PCIWake_DEBOUNCE ,NullEvent   ,NullEvent   ,NullEvent    }, /*
5*/	{&POWER_FLAG1 ,wait_NovoBtn_off	 ,&DEBOUNCE_CONT6 ,T_PSW_DEBOUNCE     ,HW_Reset_Press, HW_Reset_Released   ,NullEvent    }, /*
6*/	{&EVT_STATUS1 ,GPU_BLK_f		 ,&DEBOUNCE_CONT7 ,T_10ms_DEBOUNCE    ,NullEvent   ,NullEvent   ,NullEvent    }, /*
7*/	{&EVT_STATUS1 ,DelKey_f			 ,&DEBOUNCE_CONT8 ,T_10ms_DEBOUNCE    ,NullEvent   ,NullEvent   ,NullEvent    }, /*
8*/	{&EVT_STATUS1 ,Read_VOLUME_DOWN_f,&DEBOUNCE_CONT9 ,T_key_DEBOUNCE     ,NullEvent   ,NullEvent   ,NullEvent    }, /*
9*/	{&EVT_STATUS1 ,Read_VOLUME_UP_f	 ,&DEBOUNCE_CONT10,T_key_DEBOUNCE     ,NullEvent   ,NullEvent   ,NullEvent    },
};


// ----------------------------------------------------------------------------
// EventManager function
// ----------------------------------------------------------------------------
#pragma OT(8, SPEED)
void EventManager(BYTE device_id)
{
	BYTE new_state,old_state;
	BYTE device_flag;

	switch(device_id)	
	{
		case 0 :							
			new_state = Read_NBSW();    // power button  获取按键状态
			break;
			
		case 1 :     							
			new_state = Read_ACIN();   // AC			获取适配器状态
			break;
			
		case 2 :
			new_state = Read_BatIN();  // battery		获取电池状态
			break;
			
		case 3 :
			new_state = Read_LID();    // Lid			获取笔记本盖子状态
			break;
			
		case 4 :
			new_state = 0;
			break;
			
		case 5 :
			new_state = 0;
			break;
			
		case 6 :
			new_state = 0;
			break;
				
		case 7:	
            new_state = 0;
			break;

		case 8:	 
			new_state = 0;
			break;	

		case 9:	
            new_state = 0;
			break;
	
		default :
			new_state = 0x00;
			break;
	}

	Tmp_XPntr = Debounce_TBL[device_id].REG;
	Tmp_XPntr1 = Debounce_TBL[device_id].Cunter;
	device_flag = Debounce_TBL[device_id].Flag;

	old_state = ((*Tmp_XPntr & device_flag) != 0x00);
	if(new_state != old_state)
	{		 
		if((*Tmp_XPntr1) == 0x00)	
		{	
			(*Tmp_XPntr1) = Debounce_TBL[device_id].Time;	
		}	 
		else 
		{	
			(*Tmp_XPntr1) --;  //Keys away shaking
			if((*Tmp_XPntr1) == 0)	
			{	 
				if(new_state)
				{	
					(Debounce_TBL[device_id].press)();
					SET_MASK(*Tmp_XPntr,device_flag);
				}
				else 
				{	
					(Debounce_TBL[device_id].release)();
					CLEAR_MASK(*Tmp_XPntr,device_flag);
				}
			}  
		}  
	}
	else
	{ 	
		*Tmp_XPntr1 = 0x00;
	}
	
	(Debounce_TBL[device_id].nochange)();
}

