/*-----------------------------------------------------------------------------
 * TITLE: OEM_BATTERY.C
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

//****************************************************************************
// The function of smart battery
//****************************************************************************

const sbatINFO code BATDeviceNameTable[] =
{
//   Id     Device name  
    {0x01,   	"XXXXXXX"       },	    // model 1
    {0x02,   	"YYYYYY"        },	    // model 2
    {0x03,   	"TW932SM"       },      // model 3
};

const sbatFirstData code BAT1_FirstDataTable[] =
{
//   command     address of variable  
    {BATCmd_temp,       &BAT1_temp_L        },	
    {BATCmd_volt,   	&BAT1_volt_L        },
    {BATCmd_current,   	&BAT1_current_L     },
    {BATCmd_RSOC,       &BAT1_RSOC          },	
    {BATCmd_RMcap,      &BAT1_RMcap_L       },
    {BATCmd_FCcap,      &BAT1_FCcap_L       },
    {BATCmd_CC,   	    &BAT1_CC_L          },	
    {BATCmd_CV,   	    &BAT1_CV_L          },
    {BATCmd_BatStatus,  &BAT1_ALARM1        },
    #if Lenovo_Support
	{BATCmd_access,  	&BAT1_ACCESSL       },
    {BATCmd_mode,       &BAT1_MODEL         }, 
    {BATCmd_MfgF2,  	&BAT1_MFGF2L		},
	#endif
};

BYTE AC_OFF(void)
{
	XWORD cctemp;
	XWORD cvtemp;
	
	return 0;
	
	if(SmartCharger_Support)
	{		
		#if Lenovo_Support
		cctemp = (BAT1_CC_H*256)+BAT1_CC_L;
		cvtemp = (BAT1_CV_H*256)+BAT1_CV_L;
		if((cctemp != 0) && (cvtemp != 0))
		{
			CHARGE_CURRENT_LO=cctemp;
	    	CHARGE_CURRENT_HI=cctemp>>8;	
	    	CHARGE_VOLTAGE_LO=BAT1_CV_L;
	    	CHARGE_VOLTAGE_HI=BAT1_CV_H;
		}
		#endif		
		
        if(SetSmartCharger()!=0x00)
        {
  			return FALSE;
        }
		if(bRWSMBus(SmartChargerChannel, SMbusRW, Charger_Addr, _CMD_ChargerOption0, &CHARGER_OPTION_L, 0))
		{
			if(IS_MASK_CLEAR(CHARGER_OPTION_L, BIT0))
			{
				SET_MASK(CHARGER_OPTION_L,BIT0);
				if(bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerOption0, &CHARGER_OPTION_L, 0) == FALSE )
				{
					CLEAR_MASK(Oem_Globe_Flag2,Oem_AC_OFF_f);
					return FALSE;
				}
				else
				{
					SET_MASK(Oem_Globe_Flag2,Oem_AC_OFF_f);
					CLEAR_MASK(Oem_Globe_Flag2,Oem_AC_ON_f);				
					return TRUE;
				}
			}		
			else
				return TRUE;
		}
	}	
	return FALSE;	
}

BYTE AC_ON(void)
{
	XWORD cctemp;
	XWORD cvtemp;

	return 0;
	
	if(SmartCharger_Support)
	{

		#if Lenovo_Support
		 cctemp = (BAT1_CC_H*256)+BAT1_CC_L;
		 cvtemp = (BAT1_CV_H*256)+BAT1_CV_L;
		if((cctemp != 0) && (cvtemp != 0))
		{
			CHARGE_CURRENT_LO=cctemp;
	    	CHARGE_CURRENT_HI=cctemp>>8;	
	    	CHARGE_VOLTAGE_LO=BAT1_CV_L;
	    	CHARGE_VOLTAGE_HI=BAT1_CV_H;
		}
		#endif		

	
		INPUT_CURRENT_LO=InputCurrent;
		INPUT_CURRENT_HI=InputCurrent >> 8; 
		
		if(SetSmartCharger()!=0x00)
		{
			return FALSE;
		}
		if(bRWSMBus(SmartChargerChannel, SMbusRW, Charger_Addr, _CMD_ChargerOption0, &CHARGER_OPTION_L, 0))
		{
			if(IS_MASK_SET(CHARGER_OPTION_L, BIT0))
			{
				CLEAR_MASK(CHARGER_OPTION_L,BIT0);
				if(bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerOption0, &CHARGER_OPTION_L, 0) == FALSE )
				{	
					CLEAR_MASK(Oem_Globe_Flag2,Oem_AC_ON_f);
					return FALSE;
				}
				else
				{
					SET_MASK(Oem_Globe_Flag2,Oem_AC_ON_f);
					CLEAR_MASK(Oem_Globe_Flag2,Oem_AC_OFF_f);
					return TRUE;
					
				}
			}
			else
				return TRUE;
		}
	}
	return FALSE;
}

void Storage_Mode_Exit(void);


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
BYTE Battery_Auth_Lenovo(void)
{
//       old  authentication 
	unsigned int  PC_Npack,BAT_Npack;
	unsigned char AuthStr[4] = {0x00,0x00,0x32,0x44};
	

	if(IS_MASK_CLEAR(Oem_BatCtrl_F,Oem_BatAuth_WBK_f))
	{
		Tmp_XPntr = AuthStr;
		if(bWSMBusBlock(SmartBatChannel,SMbusWBK,SmartBat_Addr,BATCmd_MfgF4,Tmp_XPntr,4,0))
		{		
			SET_MASK(Oem_BatCtrl_F,Oem_BatAuth_WBK_f);
			SetBAT1IDTimeOutCNT();
		}
		else
	    {
	        BAT1_ID_TimeOut_CNT--;
	        if(BAT1_ID_TimeOut_CNT==0x00)           // time out
	        {   
	            SetBAT1CtrlStep(BAT_Step_WC);       // Go to wake up charge
	        }
		}
	}
	else 
	{
		if(bRSMBusBlock(SmartBatChannel,SMbusRBK,SmartBat_Addr,BATCmd_MfgF4,&SMB_DATA))
		{
			PC_Npack=(WORD)(SMB_DATA1[5]*16*SMB_DATA1[7]*16+0x339E);
			BAT_Npack=(WORD)(SMB_DATA1[8]*256+SMB_DATA1[6]);

			if(PC_Npack==BAT_Npack)
			{
				BIOS_OEM_battery_health|=HOST_BAT_Authentication;

				BAT1IdentifyNextStep();         // next step 
			}
			else
			{
				BIOS_OEM_battery_health&=~HOST_BAT_Authentication;
				BAT1_SetFail(Bat_AuthFail); 
				
			}
			
			return 1;
		}
		else
	    {
	        BAT1_ID_TimeOut_CNT--;
	        if(BAT1_ID_TimeOut_CNT==0x00)           // time out
	        {   
	            SetBAT1CtrlStep(BAT_Step_WC);       // Go to wake up charge
	        }
		}
		
	}
	
	return 0;
}



//----------------------------------------------------------------------------
// Get battery device name and checking battery is supported or not
//----------------------------------------------------------------------------
void BAT1ID_CheckDeviceName(void)
{

	XBYTE index;                     // Battery item count
	XBYTE BCunt;                     // Battery name count


#ifdef Lenovo_Support
	 BAT1IdentifyNextStep();         // next step 
	 return;
#endif

    if(bRSMBusBlock(SmartBatChannel, SMbusRBK, SmartBat_Addr, BATCmd_Dname, &SMB_DATA))
    {
	    for(index=0x00;index<(sizeof(BATDeviceNameTable)/sizeof(sbatINFO));index++) // Compare all support list
	    {
		    Tmp_XPntr = &SMB_DATA;
		    for(BCunt=0x00;BCunt<SMB_BCNT;BCunt++)  // compare all char
		    {
			    if(BATDeviceNameTable[index].DeviceName[BCunt]!= *Tmp_XPntr)
			    {
    				break;                      // fail , point to next
			    }
			    Tmp_XPntr++;
		    }                                   // End of For Loop for check device name is meet
		
		    if(BCunt==SMB_BCNT)                 // meet design name
		    {
                BAT1_Number  = BATDeviceNameTable[index].DeviceNO;
                BAT1IdentifyNextStep();         // next step                  
			    return;
		    }                                   // End of meet battery device name
	    }                                       // End of FOR loop for check battery device name 

        BAT1_SetFail(Bat_NoSupport);            // battery isn't supportd model

    }         // End of Read SMBUS Block function
    else
    {
        BAT1_ID_TimeOut_CNT--;
        if(BAT1_ID_TimeOut_CNT==0x00)           // time out
        {   
            SetBAT1CtrlStep(BAT_Step_WC);       // Go to wake up charge
        }
    }
}

//----------------------------------------------------------------------------
// Set battery 1 fail
//----------------------------------------------------------------------------
void BAT1_SetFail(BYTE failcause)
{
    BAT1_FailCause = failcause;
    SetBAT1CtrlStep(BAT_Step_SetFail);
}

//----------------------------------------------------------------------------
// If battery is set fail Clear variables
//----------------------------------------------------------------------------
void BAT1_ClrVariables(void)
{
    BT1_STATUS2 = 0x00;
    BT1_STATUS3 = 0x00;

// clear battery flag, Don't chear bat_in flag
	CLEAR_MASK(BT1_STATUS1,(bat_InCharge+bat_Dischg+bat_Full));

    BAT1_CtrlStep = 0x00;
    BAT1_ID_Step = 0x00;
    BAT1_1SecTimer = 0x00;
//    BAT1_S_Number = 0x00;     // battery cell number
    BAT1_ID_TimeOut_CNT = 0x00;
    BAT1_Number = 0x00;
    BAT1_FirstDataIndex = 0x00;
    Charger_TimeOut = 0x00;
    WakeUpChrCunt_Min = 0x00;
    PreChrCunt_Min = 0x00;
    NormalChrCunt_Min = 0x00;
    WakeUpChrCunt = 0x00;
    PreChrCunt = 0x00;
    NormalChrCunt = 0x00;
    BAT1_OTCunt = 0x00;
    BAT1_OVCunt = 0x00; 
    CHARGE_CURRENT_BK = 0x00;
    CHARGE_VOLTAGE_BK = 0x00;
}


//----------------------------------------------------------------------------
// If battery is pluged out Clear variables
//----------------------------------------------------------------------------
void BAT1_PlugOutClrVariables(void)
{
    BAT1_temp_L= 0x00;
	BAT1_temp_H= 0x00;
	//J80_009A-->>
	/*
	//J80_009++>>
	BAT1_T_L= 0x00;
	BAT1_T_H= 0x00;
	//J80_009++<<
	*/
	//J80_009A--<<
    BAT1_volt_L = 0x00;
	BAT1_volt_H = 0x00;
    BAT1_current_L = 0x00;
	BAT1_current_H = 0x00;
    BAT1_RMcap_L = 0x00;
	BAT1_RMcap_H = 0x00;
	BAT1_FCcap_L = 0x00;
	BAT1_FCcap_H = 0x00;
    BAT1_CC_L = 0x00;
	BAT1_CC_H = 0x00;
    BAT1_CV_L = 0x00;
	BAT1_CV_H = 0x00;
    
    BAT1_RSOC = 0x00;
	
    BAT1_ALARM1 = 0x00;
    BAT1_ALARM2 = 0x00;
    
    BT1_STATUS2 = 0x00;
    BT1_STATUS1 = 0x00;
    BT1_STATUS3 = 0x00;
    
    BAT1_CtrlStep = 0x00;
    BAT1_ID_Step = 0x00;
    BAT1_1SecTimer = 0x00;
    BAT1_S_Number = 0x00;
    BAT1_ID_TimeOut_CNT = 0x00;
    BAT1_Number = 0x00;
    BAT1_FirstDataIndex = 0x00;
	BAT1_FailCause = 0x00;
	
    Charger_TimeOut = 0x00;
    WakeUpChrCunt_Min = 0x00;
    PreChrCunt_Min = 0x00;
    NormalChrCunt_Min = 0x00;
    WakeUpChrCunt = 0x00;
    PreChrCunt = 0x00;
    NormalChrCunt = 0x00;
    BAT1_OTCunt = 0x00;
    BAT1_OVCunt = 0x00; 
    CHARGE_CURRENT_BK = 0x00;
    CHARGE_VOLTAGE_BK = 0x00;

	
}

void Adapter_PlugOutClrVariables(void)
{
	R_CHARGE_CTRL0 		= 0x00;	//0x01B7
	R_CHARGE_CTRL1 		= 0x00;	//0x01B9
	R_CHARGE_CTRL2 		= 0x00;	//0x01BB
	R_CHARGE_CURRENT 	= 0x00;	//0x01BD
	R_CHARGE_VOLTAGE 	= 0x00;	//0x01C0
	R_INPUT_CURRENT 	= 0x00;	//0x01C2
	R_MANUFACTURER_ID 	= 0x00;	//0x01C4
	CHG_STATUS			= 0x00;	//0x01C6
	TEMP_CHARGE_INPUT   = 0x00;	//0x01C8
}


//----------------------------------------------------------------------------
// Polling battery 1 data function
//----------------------------------------------------------------------------
void PollingBAT1Data(void)
{
    SMBus1Flag1 |= (B1_RSOC+B1_C+B1_V+B1_RMcap+B1_Status+B1_FCC+B1_Temp+B1_Cycle);
    SMBus1Flag2 |= (B1_CC+B1_CV+B1_ACCESS+B1_Mode+B1_MFGF2+B1_DScap);
	SMBus1Flag5 |= (B1_SerialNo+B1_Mname+B1_Dname+B1_Dvol);
}	

#if Lenovo_Support

void BAT1MFGF2Polling(void)
{
	EC_OptionalMfgFunction2 = BAT1_MFGF2L+ BAT1_MFGF2H * 256;
	
	if((BAT1_MFGF2H&0x01)&&(BAT1_STATUS&OS_AC_IN))
	{
		SET_MASK(BIOS_OEM_battery_health,HOST_STM_NOTIFY);
	}
	else
	{
		CLEAR_MASK(BIOS_OEM_battery_health,HOST_STM_NOTIFY);
	}

	if(temp_guoyong1 == 0x88)
	{
		SET_MASK(BIOS_OEM_battery_health,HOST_STM_NOTIFY);
	}
		
}

#endif


//----------------------------------------------------------------------------
// Polling battery 1 Optional Mfg Function2
//----------------------------------------------------------------------------
void PollingChargerData(void)
{
	SMBus1Flag4 |= (Charge_Ctrl0+Charge_Ctrl1+Charge_Ctrl2+Charge_CC+Charge_CV+Charge_MFID+Charge_Input_CC);
}

//----------------------------------------------------------------------------
// Polling battery 1 battery 0x00 , if Bit1=1   "Bad Battery" be detected. 
//----------------------------------------------------------------------------
#if Lenovo_Support
void BAT1GoodOrBad(void)
{
	if((BAT1_ACCESSL & (bat_GoodOrBad_f+BIT0)) != 0)
	{
		if(++BAT1_BADCunt>BatBadTimeOut)
			{
				SET_MASK(Bios_Battery_Health_2,MBBD);
			}
	}
	else
	{
		BAT1_BADCunt = 0;
		CLEAR_MASK(Bios_Battery_Health_2,MBBD);

	}
}

#endif


//----------------------------------------------------------------------------
// Battery 1 protect function
//----------------------------------------------------------------------------
void BAT1Protection(void)
{
    XWORD voltage;
	XWORD tempTemperature;

#if Lenovo_Support
	tempTemperature = BAT1_temp_H*256+ BAT1_temp_L;
	if((tempTemperature < CHARGE_INHIBIT_TEMP_LOW_KELVIN) || (tempTemperature > CHARGE_INHIBIT_TEMP_HIGH_KELVIN))
		SET_MASK(Oem_BAT1_ALARM,OverTemp_alarm);
	else if((tempTemperature > CHARGE_RESUME_TEMP_LOW_KELVIN) && (tempTemperature < CHARGE_RESUME_TEMP_HIGH_KELVIN))
		CLEAR_MASK(Oem_BAT1_ALARM,OverTemp_alarm);
	#endif

    if((IS_MASK_SET(BAT1_ALARM2, OverTemp_alarm)) || (IS_MASK_SET(Oem_BAT1_ALARM, OverTemp_alarm)))   // OT protect
    {
        if(++BAT1_OTCunt > BatOTTimeOut)
        {
            BAT1_SetFail(Bat_OverTemperature);      // battery over temperature
        }
    }
    else
    {
        BAT1_OTCunt = 0x00;
    }

    voltage = (BAT1_volt_H*256)+BAT1_volt_L;        // OV protect
    if(voltage > BATOverVoltage)
    {
        if(++BAT1_OVCunt > BatOVTimeOut)
        {
            BAT1_SetFail(Bat_OverVoltage);          // battery over voltage
        }
    }
    else
    {
        BAT1_OVCunt = 0x00;
    }
}

//----------------------------------------------------------------------------
// Set battery 1 identify re-try counter
//----------------------------------------------------------------------------
void SetBAT1IDTimeOutCNT(void)
{
    BAT1_ID_TimeOut_CNT = BatIDTimeOut;
}

//----------------------------------------------------------------------------
// First time to get battery design voltage and checking SMBus communication is OK
//----------------------------------------------------------------------------
void BAT1ID_GetDesignVoltage(void)
{
    WORD DVTemp;
    if(bRWSMBus(SmartBatChannel, SMbusRW, SmartBat_Addr, BATCmd_DVolt, &ByteTemp01, SMBus_NoPEC))
    {
        DVTemp = (ByteTemp02*256)+ByteTemp01;   // Get Main battery desgin voltage
        if(bRWSMBus(SmartBatChannel, SMbusRW, SmartBat_Addr, BATCmd_DCap, &ByteTemp01, SMBus_NoPEC))
        {
        	RamDebug(ByteTemp02);
			RamDebug(ByteTemp01);
        }
        if(DVTemp<8400)
        {
            BAT1_S_Number = 0x02;       // Main battery is 2s
        }
        else if(DVTemp<12600)
        {
            BAT1_S_Number = 0x03;       // Main battery is 3s
        }
        else
        {
            BAT1_S_Number = 0x04;       // Main battery is 4s
        } 
        BAT1IdentifyNextStep();         // next step 
        ByteTemp01 = 0x00;
        ByteTemp02 = 0x00;
    }
    else
    {
        BAT1_ID_TimeOut_CNT--;
        if(BAT1_ID_TimeOut_CNT==0x00)   // time out
        {   
            SetBAT1CtrlStep(BAT_Step_WC);// Go to wake up charge
        }
    }
}

//----------------------------------------------------------------------------
// Battery Authentication if necessary
//----------------------------------------------------------------------------
void BAT1ID_CheckBatAuth(void)
{

#if Lenovo_Support
	//Battery_Auth_Lenovo();
	#if Sha1Support
		if(Sha1Authentication() == TRUE)
		{
			BAT1IdentifyNextStep();
			SET_MASK(BIOS_OEM_battery_health,HOST_BAT_Authentication);
		}	
		else
		{
			BAT1_SetFail(Bat_AuthFail);
			CLEAR_MASK(BIOS_OEM_battery_health,HOST_BAT_Authentication);
		}
	#else

		BAT1IdentifyNextStep();         // next step 
			
	#endif
#else
	BAT1IdentifyNextStep();         // next step 
#endif  

}

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
void BAT1ID_PollingFirstData(void)
{
    if(BAT1_FirstDataIndex < (sizeof(BAT1_FirstDataTable)/sizeof(sbatFirstData)))
    {
        if(bRWSMBus(SmartBatChannel, SMbusRW, SmartBat_Addr, BAT1_FirstDataTable[BAT1_FirstDataIndex].Cmd, 
            BAT1_FirstDataTable[BAT1_FirstDataIndex].Var, SMBus_NoPEC))
        {
            BAT1_FirstDataIndex++;
        }
        else
        {
            BAT1_ID_TimeOut_CNT--;
            if(BAT1_ID_TimeOut_CNT==0x00)       // time out
            {   
                SetBAT1CtrlStep(BAT_Step_WC);   // Go to wake up charge
            }
        }
    }
    else
    {
        BAT1IdentifyNextStep();                 // next step 
    }
}

//----------------------------------------------------------------------------
// Battery 1 init. OK
//----------------------------------------------------------------------------
void BAT1ID_InitOK(void)
{
	SET_MASK(BT1_STATUS1, bat_valid);
    if(IS_MASK_SET(POWER_FLAG1, adapter_in))
    {
        SetBAT1CtrlStep(BAT_Step_PC);           // Go to Pre-Charge
    }
    else                                        //Battery only
    {
        SetBAT1CtrlStep(BAT_Step_DC);           // Go to discharge
    }
}

//----------------------------------------------------------------------------
// The function of identify main battery
//----------------------------------------------------------------------------
void IdentifyBAT1(void)
{
    switch(BAT1_ID_Step)
    {
        case BATID_Step_GetDV:      // First time to get battery design voltage and checking SMBus communication is OK
            BAT1ID_GetDesignVoltage();
            break;

        case BATID_Step_Auth:       // Battery Authentication if necessary
            BAT1ID_CheckBatAuth();
			SET_MASK(BAT1_STATUS,Oem_BAT1_valid);
            break;

        case BATID_Step_MFName:     // Get manufacturer name and checking battery is supported
            BAT1ID_CheckDeviceName();
            break;

        case BATID_Step_FirstData:  // Polling first data
            BAT1ID_PollingFirstData();
            break;

        case BATID_Step_InitOK:     // Init_OK,prepare Discharge/Pre-Charge ***(always at last step)***
            BAT1ID_InitOK();
            break;
            
        default:                   // Restart Identify
			BAT1_ID_Step = BATID_Step_GetDV;
            break;
    }
}

//----------------------------------------------------------------------------
// The function of setting battery 1 control step
//----------------------------------------------------------------------------
void SetBAT1CtrlStep(BYTE ctrlstep)
{
    BAT1_CtrlStep = ctrlstep;
    Charger_TimeOut = 0x00;
    
    switch(BAT1_CtrlStep)
    {
        case BAT_Step_ID:
            SetBAT1IDTimeOutCNT();
            BAT1_ID_Step = BATID_Step_GetDV;
            break;

        case BAT_Step_WC:
			#if Lenovo_Support
			CLEAR_MASK(Oem_BatCtrl_F,Oem_BatAuth_WBK_f);
			#endif

			CLEAR_MASK(BT1_STATUS1,(bat_InCharge+bat_Dischg+bat_Full));
            BT1_STATUS2 = 0x00;
            WakeUpChrCunt = 0x0000;
            WakeUpChrCunt_Min = 0x00;
            CLEAR_MASK(BT1_STATUS1, bat_valid);
            SET_MASK(BT1_STATUS2, bat_WC);
            break;
            
        case BAT_Step_PC:
			CLEAR_MASK(BT1_STATUS1,(bat_InCharge+bat_Dischg+bat_Full));
            BT1_STATUS2 = 0x00;
            PreChrCunt = 0x0000;
            PreChrCunt_Min = 0x0000;
            SET_MASK(BT1_STATUS2, bat_PC);
            break;
            
        case BAT_Step_NC:
			CLEAR_MASK(BT1_STATUS1,(bat_InCharge+bat_Dischg+bat_Full));
            BT1_STATUS2 = 0x00;
            NormalChrCunt = 0x0000;
            NormalChrCunt_Min = 0x0000;
            CHARGE_CURRENT_BK = 0x00;
            CHARGE_VOLTAGE_BK = 0x00;
            SET_MASK(BT1_STATUS2, bat_NC);
            break;
            
        case BAT_Step_DC:
			CLEAR_MASK(BT1_STATUS1,(bat_InCharge+bat_Dischg+bat_Full));
            BT1_STATUS2 = 0x00;
            SET_MASK(BT1_STATUS2, bat_DC);
            break;            

        case BAT_Step_FullyChg:
			CLEAR_MASK(BT1_STATUS1,(bat_InCharge+bat_Dischg+bat_Full));
            SET_MASK(BT1_STATUS1, bat_Full); 
            BT1_STATUS2 = 0x00;
            break;
    }
}

//----------------------------------------------------------------------------
// The function of going to next step of identify control step
//----------------------------------------------------------------------------
void BAT1IdentifyNextStep(void)
{
    BAT1_ID_Step++;
    if(BAT1_ID_Step==BATID_Step_FirstData)
    {
        BAT1_FirstDataIndex = 0x00;
    }
    SetBAT1IDTimeOutCNT();
}

//----------------------------------------------------------------------------
// The function of on / off charge pin
//----------------------------------------------------------------------------
void Hook_BAT1_ChargePinON(BYTE on_off)
{

}

void BAT1_CheckFullyCharged(void)
{
    if((BAT1_CtrlStep==BAT_Step_PC)||(BAT1_CtrlStep==BAT_Step_NC))
    {
        if(IS_MASK_SET(BAT1_ALARM1, FullyChg))
        {
            if(DisableCharger()==0x01)          // Disable charger first, then go to fully charged step
            {
            	//J80_013--	ECQEvent(BAT1_HOT_PLUG_EVT,SCIMode_Normal);
                SetBAT1CtrlStep(BAT_Step_FullyChg);
            }
        }

    }
}

void BAT1_CheckFullyDisCharged(void)
{
    if(BAT1_CtrlStep==BAT_Step_DC)
    {
        if(IS_MASK_SET(BAT1_ALARM1, FullyDsg))
        {
            SetBAT1CtrlStep(BAT_Step_FullyDisChg);
        }
    }
}

//----------------------------------------------------------------------------
// The function of pre-charge
//----------------------------------------------------------------------------
void BAT1_SETPreCharge(void)
{

XWORD cctemp;
XWORD cvtemp;

    if(SmartCharger_Support)
    {

	#if Lenovo_Support
	 cctemp = (BAT1_CC_H*256)+BAT1_CC_L;
	 cvtemp = (BAT1_CV_H*256)+BAT1_CV_L;
		if((cctemp != 0) && (cvtemp != 0))
		{
			CHARGE_CURRENT_LO=cctemp;
	    	CHARGE_CURRENT_HI=cctemp>>8;	
	    	CHARGE_VOLTAGE_LO=BAT1_CV_L;
	    	CHARGE_VOLTAGE_HI=BAT1_CV_H;
		}
	#else
	
	    CHARGE_CURRENT_LO=BAT1_CURR_PRECHG;
	    CHARGE_CURRENT_HI=BAT1_CURR_PRECHG>>8;	
	    CHARGE_VOLTAGE_LO=BAT1_VOLT_PRECHG;
	    CHARGE_VOLTAGE_HI=BAT1_VOLT_PRECHG>>8;	
	#endif
	
		INPUT_CURRENT_LO=InputCurrent;
	    INPUT_CURRENT_HI=InputCurrent >> 8;
        if(SetSmartCharger()!=0x00)
        {
            Charger_TimeOut++;
            if(Charger_TimeOut>ChargerSettingTimeOut)
            {
                SetBAT1CtrlStep(BAT_Step_ChargerFail);  // Go to smart charger fail
            }
        }
        else
        {
            Hook_BAT1_ChargePinON(1);
            SET_MASK(BT1_STATUS2, bat_PC_OK);
            SET_MASK(BT1_STATUS1, bat_InCharge);
            Charger_TimeOut = 0x00;
        }
    }
    else
    { 
        // *** Set DAC CC CV value here ***
        Hook_BAT1_ChargePinON(1);
        SET_MASK(BT1_STATUS2, bat_PC_OK);
        SET_MASK(BT1_STATUS1, bat_InCharge);
    }
}

//----------------------------------------------------------------------------
// The function of battery 1 Pre charge
//----------------------------------------------------------------------------
void BAT1_PreCharge(void)
{
    XWORD cctemp;

	XWORD tempTemperature;
	
#if Lenovo_Support
	tempTemperature = BAT1_temp_H*256+ BAT1_temp_L;
	if((tempTemperature < CHARGE_INHIBIT_TEMP_LOW_KELVIN_INITIAL) && (tempTemperature > CHARGE_INHIBIT_TEMP_HIGH_KELVIN_INITIAL))
		return;
	else if((tempTemperature > CHARGE_RESUME_TEMP_LOW_KELVIN_INITIAL) && (tempTemperature < CHARGE_RESUME_TEMP_HIGH_KELVIN_INITIAL))
		{
			// do nothing
		}	
#endif

    if(IS_MASK_SET(POWER_FLAG1, adapter_in))
    { 
        if(IS_MASK_CLEAR(BT1_STATUS2, bat_PC_OK))
        {
            PreChrCunt = 0x00;
            BAT1_SETPreCharge();
        }
        else
        {
            PreChrCunt++;
            if((PreChrCunt%6000)==0)            // 1 min re-enable smart charger
            {
                BAT1_SETPreCharge();
                PreChrCunt = 0x0000;
                PreChrCunt_Min++;
                if(PreChrCunt_Min>BatPCTimeOut) // Over 2hr
                {
                    BAT1_SetFail(Bat_PreChargeFail);    // Pre_charge charge time-out
                }
            }
            else if((PreChrCunt%600)==0)        // 6sec
            {
                cctemp = (BAT1_CC_H*256)+BAT1_CC_L; 
                if(cctemp>Bat1_PC2NC_Cap)
                {
                    SetBAT1CtrlStep(BAT_Step_NC);   // Go to battery normal charge
                }
            }
        }
    }
    else
    {
        SetBAT1CtrlStep(BAT_Step_DC);           // Go to discharge
    }
}

//----------------------------------------------------------------------------
// The function of setting battery 1 normal charge
//----------------------------------------------------------------------------
void BAT1_SETNormalCharge(void)
{
    XWORD temp;
	XWORD cctemp;
	XWORD cvtemp;

    if(SmartCharger_Support)
    {

	#if Lenovo_Support
	 CHARGE_CURRENT_BK = (BAT1_CC_H*256)+BAT1_CC_L;
	 cctemp = (BAT1_CC_H*256)+BAT1_CC_L;
	 cvtemp = (BAT1_CV_H*256)+BAT1_CV_L;
		if((cctemp != 0) && (cvtemp != 0))
		{
			CHARGE_CURRENT_LO=cctemp;
	    	CHARGE_CURRENT_HI=cctemp>>8;	
	    	CHARGE_VOLTAGE_LO=BAT1_CV_L;
	    	CHARGE_VOLTAGE_HI=BAT1_CV_H;
		}
	#else

		temp = (BAT1_CC_H*256)+BAT1_CC_L;
        CHARGE_CURRENT_BK = (BAT1_CC_H*256)+BAT1_CC_L;
        temp *=SC_Current_Sense;
	    CHARGE_CURRENT_LO=(BYTE)temp;
	    CHARGE_CURRENT_HI=temp>>8;
	    CHARGE_VOLTAGE_LO=BAT1_CV_L;
	    CHARGE_VOLTAGE_HI=BAT1_CV_H;
	#endif
	
	    INPUT_CURRENT_LO=InputCurrent;
	    INPUT_CURRENT_HI=InputCurrent >> 8;

		if((SystemWatts_CHRStatus & Change_BatChr_f) != 0)
		{
			CHARGE_CURRENT_LO=(BYTE)Pre_SystemWatts_BatChar_C;
			CHARGE_CURRENT_HI=Pre_SystemWatts_BatChar_C>>8;
    	}
		
        if(SetSmartCharger()!=0x00)
        {
            Charger_TimeOut++;
            if(Charger_TimeOut>ChargerSettingTimeOut)
            {
                SetBAT1CtrlStep(BAT_Step_ChargerFail);  // Go to smart charger fail
            }
        }
        else
        {
            Hook_BAT1_ChargePinON(1);
            SET_MASK(BT1_STATUS2, bat_NC_OK);
            SET_MASK(BT1_STATUS1, bat_InCharge);
            Charger_TimeOut = 0x00;
        }
    }
    else
    { 
        // *** Set DAC CC CV value here ***
        Hook_BAT1_ChargePinON(1);
        SET_MASK(BT1_STATUS2, bat_NC_OK);
        SET_MASK(BT1_STATUS1, bat_InCharge);
    }
}

//----------------------------------------------------------------------------
// The function of battery 1 normal charge
//----------------------------------------------------------------------------
void BAT1_NormalCharge(void)
{
	if(IS_MASK_SET(POWER_FLAG1, adapter_in))
    {   
        if(IS_MASK_CLEAR(BT1_STATUS2, bat_NC_OK))
        {
            CHARGE_CURRENT_BK=((BAT1_CC_H<<8)+BAT1_CC_L);
            NormalChrCunt = 0x0000;
            BAT1_SETNormalCharge();
        }
        else
        {
            NormalChrCunt++;
            if(CHARGE_CURRENT_BK != (BAT1_CC_H*256)+BAT1_CC_L)
            {
                CHARGE_CURRENT_BK=((BAT1_CC_H<<8)+BAT1_CC_L);
                BAT1_SETNormalCharge();
            }
            else
            {
                if((NormalChrCunt%6000)==0)             // 1 min re-enable smart charger
                {
                    NormalChrCunt_Min++;
                    if(NormalChrCunt_Min>BatNCTimeOut)  // 8hr
                    {
                        BAT1_SetFail(Bat_NormalChargeFail); // Normal charge time-out
                    }
                    BAT1_SETNormalCharge();
                    NormalChrCunt = 0x0000;
                }
            }
        }
    }
    else
    {
        SetBAT1CtrlStep(BAT_Step_DC);           // Go to discharge
    }
}

//----------------------------------------------------------------------------
// The function of waking up charge
//----------------------------------------------------------------------------
void BAT1_SETWakeUpCharge(void)
{
    if(SmartCharger_Support)
    {
	    CHARGE_CURRENT_LO=BAT1_CURR_WUCHG;        
	    CHARGE_CURRENT_HI=BAT1_CURR_WUCHG>>8;	
	    CHARGE_VOLTAGE_LO=BAT1_VOLT_WUCHG;
	    CHARGE_VOLTAGE_HI=BAT1_VOLT_WUCHG>>8;	
	    INPUT_CURRENT_LO=InputCurrent;
	    INPUT_CURRENT_HI=InputCurrent >> 8;
        
        if(SetSmartCharger()!=0x00)     // Fail
        {
            Charger_TimeOut++;
            if(Charger_TimeOut>ChargerSettingTimeOut)
            {
                SetBAT1CtrlStep(BAT_Step_ChargerFail);  // Go to smart charger fail
            }
        }
        else                            // OK
        {
            Hook_BAT1_ChargePinON(1);
            SET_MASK(BT1_STATUS2, bat_WC_OK);
            SET_MASK(BT1_STATUS1, bat_InCharge);
            Charger_TimeOut = 0x00;
        }
    }
    else
    {
        // *** Set DAC CC CV value here ***
        Hook_BAT1_ChargePinON(1);
        SET_MASK(BT1_STATUS2, bat_WC_OK);
        SET_MASK(BT1_STATUS1, bat_InCharge);
    }
}

//----------------------------------------------------------------------------
// Battery 1 wake up charge
//----------------------------------------------------------------------------
void BAT1WakeUpCharge(void)
{
    if(IS_MASK_SET(POWER_FLAG1, adapter_in))
    {
        if(IS_MASK_CLEAR(BT1_STATUS2, bat_WC_OK))
        {
            BAT1_SETWakeUpCharge();
        }
        else
        {
            WakeUpChrCunt++;
            if((WakeUpChrCunt%6000)==0)             // 1 min re-enable smart charger
            {
                BAT1_SETWakeUpCharge();
                WakeUpChrCunt = 0x0000;
                WakeUpChrCunt_Min++;
                if(WakeUpChrCunt_Min>BatWCTimeOut)  // Over 1hr
                {
                    BAT1_SetFail(Bat_WakeUpChargeFail); // Wake up charge time-out
                }
            }
            else if((WakeUpChrCunt%600)==0)         // 6 sec Check SMBus OK or not
            {
                if(bRWSMBus(SmartBatChannel, SMbusRW, SmartBat_Addr, BATCmd_RMcap, &BAT1_RMcap_L, SMBus_NoPEC))
                {
                    SetBAT1CtrlStep(BAT_Step_ID);   // Identify main battery
                }
            }
        }
    }
    else
    {
        BAT1_SetFail(Bat_Weak);             // main battery too low and AC doesn't plue-in
    }
}

//----------------------------------------------------------------------------
// Battery 1 discharge
//----------------------------------------------------------------------------
void BAT1DisCharge(void)
{
	if(IS_MASK_SET(POWER_FLAG1, adapter_in))
	{
        if(IS_MASK_SET(BT1_STATUS3, bat_ForceDischg))
        {
    	    if(IS_MASK_CLEAR(BT1_STATUS2,bat_DC_OK))
    	    {
				RamDebug(0x50);
        	    DisableCharger();
    	    }
        }
        else
        {
            SetBAT1CtrlStep(BAT_Step_PC);
        }
    }
    else
    {
    	if(IS_MASK_CLEAR(BT1_STATUS2,bat_DC_OK))
    	{
			RamDebug(0x51);
        	DisableCharger();
    	}
	}
}

//----------------------------------------------------------------------------
// Battery 1 fully charged
//----------------------------------------------------------------------------
void BAT1FullyCharged(void)
{
    if(IS_MASK_CLEAR(POWER_FLAG1, adapter_in))
    {
        SetBAT1CtrlStep(BAT_Step_DC);           // Go to discharge
    }
	else if(IS_MASK_CLEAR(BAT1_ALARM1, FullyChg))
	{
		SetBAT1CtrlStep(BAT_Step_NC);
	}
}

//----------------------------------------------------------------------------
// Battery 1 fully discharged
//----------------------------------------------------------------------------
void BAT1FullyDisharged(void)
{
    if(IS_MASK_SET(POWER_FLAG1, adapter_in))
    {

    }
    else
    {

    }
}

//----------------------------------------------------------------------------
// Check Battery 1 discharge low cap
//----------------------------------------------------------------------------
void BAT1CheckDisChargeLow(void)
{
    if(IS_MASK_SET(BT1_STATUS2,bat_DC_OK))
    {

    }
}

//----------------------------------------------------------------------------
// Battery 1 abnormal
//----------------------------------------------------------------------------
void BAT1SetFail(void)
{
    if(DisableCharger()==0x01)          // Disable charger first, then go to BAT_Step_Fail step
    {
        BAT1_ClrVariables();
        SetBAT1CtrlStep(BAT_Step_Fail);
    }
}

//----------------------------------------------------------------------------
// Force charge Battery 1
//----------------------------------------------------------------------------
void BAT1ForceCharge(void)
{
	SET_MASK(BT1_STATUS3,bat_ForceChg);
}

//----------------------------------------------------------------------------
// Force discharge Battery 1
//----------------------------------------------------------------------------
void BAT1ForceDischarge(void)
{
	SET_MASK(BT1_STATUS3,bat_ForceDischg);
}

//----------------------------------------------------------------------------
// Battery 1 in auto learning
//----------------------------------------------------------------------------
void BAT1AutoLearning(void)
{

}

//----------------------------------------------------------------------------
// Check Battery 1 force charge / force discharge / auto learning
//----------------------------------------------------------------------------
void BAT1ForceStep(void)
{
    if(IS_MASK_SET(BT1_STATUS3, bat_ForceChg))
    {
        if(IS_MASK_SET(POWER_FLAG1, adapter_in))
        {
            if(IS_MASK_SET(BT1_STATUS1, bat_Dischg))
            {
                SetBAT1CtrlStep(BAT_Step_PC);           // Go to Pre-Charge
            }
        }
    }
    else if(IS_MASK_SET(BT1_STATUS3, bat_ForceDischg))
    {
        if(IS_MASK_SET(POWER_FLAG1, adapter_in))
        {
            if(IS_MASK_SET(BT1_STATUS1, bat_InCharge))
            {
                SetBAT1CtrlStep(BAT_Step_DC);           // Go to Discharge
            }
        }
    }
    else if(IS_MASK_SET(BT1_STATUS3, bat_AL))
    {

    }   
}


void BAT1FWUpdata(void)
{
	if (IS_MASK_CLEAR(SYS_MISC1, BAT_FW_UP))
	{
		if(DisableCharger() == FALSE)  
		{
			DisableCharger();
			CLEAR_MASK(SYS_MISC1, BAT_FW_UP);	// Go to FW updata charge
		}
		else
		{
			SET_MASK(SYS_MISC1, BAT_FW_UP);		// Go to FW updata charge
			ChangePLLFrequency(PLLFreqSetting07);
		}
	}
}



//----------------------------------------------------------------------------
// The function of main battery control center  (10ms timer base)
//----------------------------------------------------------------------------
void Battey1ControlCenter(void)
{
    if(IS_MASK_CLEAR(BT1_STATUS1, bat_in))  // If battery 1 isn't exit, return function.
    {
		CLEAR_MASK(BAT1_STATUS,Oem_BAT1_valid);
        return;
    }

	BAT1_1SecTimer++;
	if(BAT1_1SecTimer>150)      // 150*10ms
	{
		BAT1_1SecTimer = 0x00;
		
		#if Lenovo_Support
		BAT1GoodOrBad();
		BAT1MFGF2Polling();
		#endif
		
	    PollingBAT1Data();      // Set the polling battery 1 data flag
	    if(IS_MASK_SET(BT1_STATUS1, bat_valid))
	    {
          	BAT1Protection();       // Check battery normal or abnormal,    OT  and  OV
	    }
	}

	if(IS_MASK_SET(BT1_STATUS1, bat_valid))
	{
        BAT1ForceStep();            // Check Battery 1 force charge / force discharge / auto learning
        
        if(IS_MASK_SET(POWER_FLAG1, adapter_in))
        {
            BAT1_CheckFullyCharged();   // Check battery fully charged or not
        }
        else
        {
            BAT1_CheckFullyDisCharged();// Check battery fully discharged or not
        }
	}

	switch(BAT1_CtrlStep)		// Battery 1 control step
	{
		case BAT_Step_ID:			// Identify main battery
			IdentifyBAT1();
			break;	
    
		case BAT_Step_WC:			// Battery wake up charge
            BAT1WakeUpCharge();
			break;

        case BAT_Step_PC:			// Battery pre-charge
            BAT1_PreCharge();
            break;

        case BAT_Step_NC:			// Battery normal charge
            BAT1_NormalCharge();    
            break;            
				
		case BAT_Step_DC:			// Battery discharge       
		    BAT1DisCharge();
            BAT1CheckDisChargeLow();
			break;

        case BAT_Step_FullyChg:		// Battery fully charged
            BAT1FullyCharged();
            break;

        case BAT_Step_FullyDisChg:	// Battery fully discharged
            BAT1FullyDisharged();
            break;

        case BAT_Step_SetFail:		// Battery abnormal
            BAT1SetFail();
            break;

        case BAT_Step_Fail:			// Battery abnormal need main battery plug-out to reset
			break;     

        case BAT_Step_ForceDC:      // Battery force discharge
            BAT1ForceDischarge();
            break;

        case BAT_Step_ForceC:		// Battery force charge
            BAT1ForceCharge();
            break;            

        case BAT_Step_AutoLN:		// Battery auto learning
            BAT1AutoLearning();
            break; 
            
        case BAT_Step_ChargerFail:  // Smart charger abnormal
            break;

		case BAT_Step_Out:			// Battery plug out
			break;
			
#if Lenovo_Support
		case BAT_Step_keep:			// Battery keep
			break;
#endif 			
            
		default:                    // Unknow step
			break;
	}
}

//****************************************************************************
// The function of smart charger
//****************************************************************************
//----------------------------------------------------------------------------
// The function of setting charger CC CV IC 
// return : 
//          0 : charger setting OK
//          !=0 charger setting fail
//----------------------------------------------------------------------------
BYTE SetSmartCharger(void)
{   //if return 0,it represent as battery will enter charging success.
    BYTE doneindex;
    doneindex = 0x00;

	if(IS_MASK_CLEAR(POWER_FLAG1, adapter_in))
	{
		return(doneindex);
	}

	INPUT_CURRENT_LO = InputCurrent;
	INPUT_CURRENT_HI = InputCurrent >> 8;		
	//J80_002++>>
	if ((Read_ACIN()) && (IS_BATTERY1_IN()) && (BAT1_CV_L == 0) && (BAT1_CV_H == 0))
	{
		CHARGE_CURRENT_LO=BAT1_CURR_PRECHG;
	    CHARGE_CURRENT_HI=BAT1_CURR_PRECHG>>8;	
		CHARGE_VOLTAGE_LO=BAT1_VOLT_WUCHG;
		CHARGE_VOLTAGE_HI=BAT1_VOLT_WUCHG >> 8;
	}
	else if((IS_BATTERY1_OUT()) && (BAT1_CV_L != 0) && (BAT1_CV_H != 0))
	{
		CHARGE_CURRENT_LO=0;
		CHARGE_CURRENT_HI=0;
		CHARGE_VOLTAGE_LO=0;
		CHARGE_VOLTAGE_HI=0;
	}
	else
	{
	//J80_002++<<
		CHARGE_CURRENT_LO=BAT1_CC_L;
		CHARGE_CURRENT_HI=BAT1_CC_H;
		CHARGE_VOLTAGE_LO=BAT1_CV_L;
		CHARGE_VOLTAGE_HI=BAT1_CV_H;
	}	//J80_002++
		
	if(!bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerInputCurrent, &INPUT_CURRENT_LO, SMBus_NoPEC))
    {
        doneindex++;    //SMBUS fail to set input current
    }
	
	if(!bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerVoltage, &CHARGE_VOLTAGE_LO, SMBus_NoPEC))
    {
        doneindex++;    //SMBUS fail to set charge voltage
    }

	if(!bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerCurrent, &CHARGE_CURRENT_LO, SMBus_NoPEC))

    {
        doneindex++;    //SMBUS fail to set charge current
    }	
	
    return(doneindex); 
}

//****************************************************************************
// The function of polling smart charger
//****************************************************************************
//----------------------------------------------------------------------------
// The function of setting charger CC CV IC 
// return : 
//          0 : charger setting OK
//          !=0 charger setting fail
//----------------------------------------------------------------------------

BYTE PollingChargerIC(void)
{
	//if return 0,it represent as battery will enter charging success.
    BYTE chargeindex;
    chargeindex = 0x00;

	if(IS_MASK_CLEAR(POWER_FLAG1, adapter_in))
	{
		return(chargeindex);
	}

	PollingChargerData();
	
	INPUT_CURRENT_LO=InputCurrent;
	INPUT_CURRENT_HI=InputCurrent >> 8;
	//J80_002++>>
	if ((Read_ACIN()) && (IS_BATTERY1_IN()) && (BAT1_CV_L == 0) && (BAT1_CV_H == 0))
	{
		CHARGE_CURRENT_LO=BAT1_CURR_PRECHG;
	    CHARGE_CURRENT_HI=BAT1_CURR_PRECHG>>8;	
		CHARGE_VOLTAGE_LO=BAT1_VOLT_WUCHG;
		CHARGE_VOLTAGE_HI=BAT1_VOLT_WUCHG >> 8;
	}
	else if((IS_BATTERY1_OUT()) && (BAT1_CV_L != 0) && (BAT1_CV_H != 0))
	{
		CHARGE_CURRENT_LO=0;
		CHARGE_CURRENT_HI=0;
		CHARGE_VOLTAGE_LO=0;
		CHARGE_VOLTAGE_HI=0;
	}
	else
	{
	//J80_002++<<
		CHARGE_CURRENT_LO=BAT1_CC_L;
		CHARGE_CURRENT_HI=BAT1_CC_H;
		CHARGE_VOLTAGE_LO=BAT1_CV_L;
		CHARGE_VOLTAGE_HI=BAT1_CV_H;
	}	//J80_002++
    if(!bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerInputCurrent, &INPUT_CURRENT_LO, SMBus_NoPEC))
    {
        chargeindex++;    //SMBUS fail to set input current
    }
	
	if(!bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerVoltage, &CHARGE_VOLTAGE_LO, SMBus_NoPEC))
    {
        chargeindex++;    //SMBUS fail to set charge voltage
    }

	if(!bRWSMBus(SmartChargerChannel, SMbusWW, Charger_Addr, _CMD_ChargerCurrent, &CHARGE_CURRENT_LO, SMBus_NoPEC))

    {
        chargeindex++;    //SMBUS fail to set charge current
    }
    return(chargeindex);
}

//----------------------------------------------------------------------------
// The function of disable charger
// return
//  0 : disable charger fail
//  1 : disable charger OK
//----------------------------------------------------------------------------
BYTE DisableCharger(void)
{
    BYTE result;
    result = 0x00;
    if(SmartCharger_Support)
    {
        CHARGE_CURRENT_LO=00;
	    CHARGE_CURRENT_HI=00;
        if(SetSmartCharger()!=0x00)
        {
            Charger_TimeOut++;
            if(Charger_TimeOut>ChargerSettingTimeOut)
            {
				
                SetBAT1CtrlStep(BAT_Step_ChargerFail);  // Go to smart charger fail
            }
            result = 0x00;
        }
        else
        {                
            if(IS_MASK_SET(POWER_FLAG1, adapter_in))    // AC mode disable charger
            {
                if(IS_MASK_SET(BT1_STATUS3, bat_ForceDischg))
                {
                    Hook_BAT1_ChargePinON(0);
                }
                else
                {
                    Hook_BAT1_ChargePinON(1);
                }
            }
            else
            {
                Hook_BAT1_ChargePinON(0);
                SET_MASK(BT1_STATUS2, bat_DC_OK);       // DC mode disable charger
                SET_MASK(BT1_STATUS1, bat_Dischg);
            }
            Charger_TimeOut = 0x00;
            result = 0x01;
        }
    }
    else
    {
        // *** Set DAC CC CV value here ***
        if(IS_MASK_SET(POWER_FLAG1, adapter_in))    // AC mode disable charger
        {
            if(IS_MASK_SET(BT1_STATUS3, bat_ForceDischg))
            {
                Hook_BAT1_ChargePinON(0);
            }
            else
            {
                Hook_BAT1_ChargePinON(1);
            }
        }
        else                                        // DC mode disable charger
        {
            Hook_BAT1_ChargePinON(0);
            SET_MASK(BT1_STATUS2, bat_DC_OK);
            SET_MASK(BT1_STATUS1, bat_Dischg);
        }
        result = 0x01;
    }	    
    return(result);
}


// 这个函数没有用
void Battey1FrimUp(void)    // 10ms  timebase   in s0
{
	if (IS_MASK_SET(SYS_MISC1, Bat1_Firm_En))
	{
		if (IS_MASK_CLEAR(SYS_MISC1, BAT_FW_UP))
		{
//			SET_MASK(SYS_MISC1, BAT_FW_UP);	// Go to FW updata charge
//			SetBAT1CtrlStep(BAT_Step_FWUP);	
			BAT1FWUpdata( );
		}
	}
	else
	{
		if (IS_MASK_SET(SYS_MISC1, BAT_FW_UP))
		{
			CLEAR_MASK(SYS_MISC1, BAT_FW_UP);	// Go to FW updata charge
			SetBAT1CtrlStep(BAT_Step_ID);	
		}
	}
}


#if Lenovo_Support
//----------------------------------------------------------------------------
// Lenovo  feature       battery 
//----------------------------------------------------------------------------

void Calibrate_Mode_PreDC(void)
{

	if(BAT1_RSOC < 95)
	{
		//BAT1ForceCharge();
		AC_ON();
		BAT_Calibrate_Step = BAT_Calibrate_Step_NC;
	}else
	{
		AC_OFF();		
	}

}

void Calibrate_Mode_NC(void)
{
	if(BAT1_RSOC >= 100)
	{
		//BAT1ForceDischarge();
		AC_OFF();
		BAT_Calibrate_Step = BAT_Calibrate_Step_DC;

	}
}


void Calibrate_Mode_DC(void)
{
	if(BAT1_RSOC < 1) //(BAT1_CtrlStep == BAT_Step_FullyDisChg)
	{
		//BAT1ForceCharge();
		AC_ON();
		BAT_Calibrate_Step = BAT_Calibrate_Step_Exit;

	}
}


void Calibrate_Mode_Exit()
{

	AC_ON();
	SET_MASK(VPC_Lenovo,Genenal_Bit_vpc);
	//J80_013--	ECQEvent(0x44,SCIMode_Normal);
	
	BAT_Calibrate_Step = 0x00;

}


void Battery_Calibrate_Mode()
{
	if(BAT_Calibrate_Step ==0)
	{
		if(BAT1_RSOC >= 95)
			BAT_Calibrate_Step= BAT_Calibrate_Step_PreDC;
		else
			BAT_Calibrate_Step=BAT_Calibrate_Step_NC;

		BAT_Storagemode_Step=BAT_Storagemode_Step_Exit;

	}
	if((Sys_Power_Status & 0x7f) == 5)
	{
		Sys_Power_Status = 0;
		BAT_Calibrate_Step = BAT_Calibrate_Step_Exit;
	}

	//s5 can exit battery calibrate mode
	if(SysPowState == SYSTEM_S5)
	{
		BAT_Calibrate_Step = BAT_Calibrate_Step_Exit;

	}

    //Battery has something wrong
	if(BAT1_FailCause != 0x00)
	{
		BAT_Calibrate_Step = BAT_Calibrate_Step_Exit;

	}



    // if >95% preDisCharge  --->  charge to 100% ----> discharge 0%  ----> exit calibrate mode
    
	switch(BAT_Calibrate_Step)			 // Battery 1 control step
	{
		case BAT_Calibrate_Step_PreDC:		  // battery predischarge
			Calibrate_Mode_PreDC();
			break;	

		case BAT_Calibrate_Step_NC: 		  // normal charge
			Calibrate_Mode_NC();
			break;	

		case BAT_Calibrate_Step_DC: 		  // discharge
			Calibrate_Mode_DC();
			break;

		case BAT_Calibrate_Step_Exit:		  // exit calibrate mode
			Calibrate_Mode_Exit();
			break;	   
			
		default:					          // Unknow step
			break;
	}

	
}

void Storage_Mode_Check(void)
{
	BAT_Storagemode_Step=BAT_Storagemode_Step_DC;
}


void Storage_Mode_DC(void)
{
	if(BAT1_RSOC > 60)
	{
		AC_OFF();
	}
	else
	{
		AC_ON();
		BAT_Storagemode_Step=BAT_Storagemode_Step_NC;
	}
}

void Storage_Mode_NC(void)
{
	if(BAT1_RSOC>=60)
	{
		SET_MASK(BT1_STATUS3,bat_ForceDischg);
	}
	else if(BAT1_RSOC<55)
	{
		CLEAR_MASK(BT1_STATUS3,bat_ForceDischg);
	}
}


void Storage_Mode_Exit(void)
{
	AC_ON();

	BAT_Storagemode_Step = BAT_Storagemode_Check;
	CLEAR_MASK(BT1_STATUS3,bat_ForceDischg);
	CLEAR_MASK(BIOS_OEM_battery_health,HOST_BTCM);
	
	
}

void Battery_Storage_Mode(void)
{
	if(IS_MASK_SET(BIOS_OEM_battery_health,HOST_BTCM))//when host end cycle exit
	{
		BAT_Storagemode_Step = BAT_Storagemode_Step_Exit;		
	}
	
	switch(BAT_Storagemode_Step)			 // Battery 1 control step
	{
		case BAT_Storagemode_Check:				//check if enter storage mode
			Storage_Mode_Check();
			break;
		case BAT_Storagemode_Step_DC:		  // battery discharge
			Storage_Mode_DC();
			break;	
		case BAT_Storagemode_Step_NC: 		  // normal charge
			Storage_Mode_NC();
			break;	
		case BAT_Storagemode_Step_Exit: 		  // normal charge
			Storage_Mode_Exit();
			break;			
		default:
			BAT_Storagemode_Step=0;
			break;
	}


}

// 这个函数没有被使用
void Lenovo_Battery_Health(void)
{	
	if(BAT_Storagemode_Step != BAT_Storagemode_Check)
		BAT_Storagemode_Step = BAT_Storagemode_Step_Exit;
	else
	{
		Battery_Calibrate_Mode();
		return;
	}

	if((Sys_Power_Status & 0x7f) == 5)
	{
		Sys_Power_Status = 0;
	}

	if(BIOS_OEM_battery_health & HOST_end_calibrate)//when host end calibrate exit battery calibrate
	{
		CLEAR_MASK(BIOS_OEM_battery_health,HOST_end_calibrate);
		BAT_Calibrate_Step = BAT_Calibrate_Step_Exit;
	}

	Battery_Storage_Mode();
}

#endif

void CheckBatShipMode(void)
{
	if (IS_MASK_CLEAR(Bat_ShipMode_Cmd,BAST) && IS_MASK_SET(Bat_ShipMode_Cmd,BASM))
	{	
		RamDebug(0x95);
		Delay1MS(10);
		
		Bat_ShipMode_Data_L = 0x10;
		Bat_ShipMode_Data_H = 0x00;
		
		if (bRWSMBus(SMbusCh1, SMbusWW, 0x16, 0x00, &Bat_ShipMode_Data_L, SMBus_NoPEC))		// write 0x0000 to 0x34
		{
			RamDebug(0x96);
			Bat_ShipMode_Data_L = 0x10;
			if (bRWSMBus(SMbusCh1, SMbusWW, 0x16, 0x00, &Bat_ShipMode_Data_L, SMBus_NoPEC))		//write 0x1000 to 0x34
			{
				RamDebug(0x97);
				Bat_ShipMode_Cmd &= ~BASM;
				Bat_ShipMode_Cmd |= BASS;
				/*
				if (bRWSMBus(SMbusCh1, SMbusRW, 0x16, 0x34, &Bat_ShipMode_Data_L, SMBus_NoPEC))			//read data from 0x34
				{
					RamDebug(0x98);
					if (Bat_ShipMode_Data_L==0x00 && Bat_ShipMode_Data_H==0x01)					// return 0x0100, ship mode done!
					{
						Bat_ShipMode_Cmd &= ~BASM;
						Bat_ShipMode_Cmd |= BASS;
					}
				}
				*/
			}
		}
	}
	else
		Bat_ShipMode_Cmd = 0x00;		  
}


