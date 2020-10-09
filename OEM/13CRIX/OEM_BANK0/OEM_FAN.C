/*-----------------------------------------------------------------------------
 * TITLE: OEM_FAN.C
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

//-----------------------------------------------------------------------------
// Smart Fan control
//-----------------------------------------------------------------------------
// Fan table 1   2009/08/18 Rev.01
//
// T > on    level +1  ;   T < off ,Level -1
//
// Add function for linear calculation. Aping 2014/03/03, Label:Linear
//-----------------------------------------------------------------------------

//:Linear
const thermal code ThermalTalbe1[]=
{
 //Level CFanOn,CFanOff, CRPM,	IsLinear FanOn,			FanOff,			RPM,
	{0,  40, 	0,		0,		0,		&Fan1On_Step1,	&Fan1Off_Step1, &Fan1RPM_Step1	},	// Level 0	
	{1,  45,	35, 	34, 	0,		&Fan1On_Step2,	&Fan1Off_Step2, &Fan1RPM_Step2	},	// Level 1	
	{2,  55,	45, 	34, 	1,		&Fan1On_Step3,	&Fan1Off_Step3, &Fan1RPM_Step3	},	// Level 2
	{3,  65,	55, 	36, 	0,		&Fan1On_Step4,	&Fan1Off_Step4, &Fan1RPM_Step4	},	// Level 3
	{4,  80,	65, 	36, 	1,		&Fan1On_Step5,	&Fan1Off_Step5, &Fan1RPM_Step5	},	// Level 4
	{5,  100,	80, 	46, 	0,		&Fan1On_Step6,	&Fan1Off_Step6, &Fan1RPM_Step6	},	// Level 5
};

//-----------------------------------------------------------------------------
// Fan table 2
//-----------------------------------------------------------------------------
const thermal code ThermalTalbe2[]=
{
//Level CFanOn,CFanOff, CRPM,	IsLinear FanOn, 		FanOff, 		RPM,
	{0,	 38,    0,		0,		0,		&Fan2On_Step1,	&Fan2Off_Step1,	&Fan2RPM_Step1	},	// Level 0 	
	{1,	 50,   36,		30,		0,		&Fan2On_Step2,	&Fan2Off_Step2,	&Fan2RPM_Step2	},	// Level 1 	
	{2,	 55,   48,		34,		0,		&Fan2On_Step3,	&Fan2Off_Step3,	&Fan2RPM_Step3	},	// Level 2 	
	{3,	 60,   53,		37,		0,		&Fan2On_Step4,	&Fan2Off_Step4,	&Fan2RPM_Step4	},	// Level 3 	
	{4,	 65,   58,		40,		0,		&Fan2On_Step5,	&Fan2Off_Step5,	&Fan2RPM_Step5	},	// Level 4 	
	{5,	 70,   63,		43,		0,		&Fan2On_Step6,	&Fan2Off_Step6,	&Fan2RPM_Step6	},	// Level 5 	
	{6,	 75,   68,		46,		0,		&Fan2On_Step7,	&Fan2Off_Step7,	&Fan2RPM_Step7	},	// Level 6 	
	{7, 127,   73,		56,		0,		&Fan2On_Step8,	&Fan2Off_Step8,	&Fan2RPM_Step8	},	// Level 7 		
};

//-----------------------------------------------------------------------------
// The function of check fan rpm 1     1sec-A  time base     
//-----------------------------------------------------------------------------	
void CheckFanRPM1(void)
{
	BYTE FanLevel_t = FanLevel;	
	BYTE Fan1RPM_t = Fan1RPM;

	temperature1 = CpuTm;

	if(Debugtemperature1!=0x00)
	{
		temperature1 = Debugtemperature1;
	}
		
	if(FanLevel_t <((sizeof(ThermalTalbe1)/sizeof(thermal))-1))
	{
		if(temperature1> *ThermalTalbe1[FanLevel_t].FanOn)
		{
			FanLevel_t += 1;
		}
	}
	
	if (FanLevel_t>0)
	{
		if (temperature1< *ThermalTalbe1[FanLevel_t].FanOff)
		{			
			FanLevel_t-=1;
		}
	}
		
	FanLevel = FanLevel_t;	
	Fan1RPM = *ThermalTalbe1[FanLevel_t].RPM;

	if(FanLevel_t < sizeof(ThermalTalbe1)/sizeof(thermal)-1)	// :Linear level check
	{
		if (ThermalTalbe1[FanLevel_t].IsLinear)	// :Linear calc
		{
			BYTE RpmDiff = ThermalTalbe1[FanLevel_t+1].CRPM - ThermalTalbe1[FanLevel_t].CRPM;
			BYTE TmDiff = temperature1 - ThermalTalbe1[FanLevel_t].CFanOff;
			BYTE CFanDiff = ThermalTalbe1[FanLevel_t].CFanOn - ThermalTalbe1[FanLevel_t].CFanOff;
			*ThermalTalbe1[FanLevel_t].RPM = ThermalTalbe1[FanLevel_t].CRPM + RpmDiff * TmDiff / CFanDiff;
			Fan1RPM = *ThermalTalbe1[FanLevel_t].RPM;
		}
	}
	
	if(0x00==OS_mode)
		Fan1RPM=FanDOSValue;
	
}

//-----------------------------------------------------------------------------
// The function of check fan rpm 2
//-----------------------------------------------------------------------------
void CheckFanRPM2(void)
{
	BYTE FanLevel_t = FanLeve2;	
	BYTE Fan1RPM_t = Fan2RPM;

	if(Debugtemperature2==0x00)
	{
		//J80_010--	temperature2 = TSys_temp;
		temperature2 = GPUTm;	//J80_010++
	}
	else
	{
		temperature2 = Debugtemperature2;
	}
		
	if(FanLevel_t <((sizeof(ThermalTalbe2)/sizeof(thermal))-1))
	{
		if(temperature2> *ThermalTalbe2[FanLevel_t].FanOn)
		{
			FanLevel_t += 1;
		}
	}
	
	if (FanLevel_t>0)
	{
		if (temperature2< *ThermalTalbe2[FanLevel_t].FanOff)
		{			
			FanLevel_t-=1;
		}
	}
		
	FanLeve2 = FanLevel_t;	
	Fan2RPM = *ThermalTalbe2[FanLevel_t].RPM;
}

void Fan1MainControl(void)
{
	if(Fan1RPM>=Fan2RPM)
	{	
		if(((Fan1RPM*100)-RPMACC)>RPM1)
		{
			if(FanValue<MaxOutValue)
				FanValue++;	
		}
		else if(((Fan1RPM*100)+RPMACC)<RPM1)
		{
			if(FanValue>0x00)
				FanValue--;	
		}
	}
	else
	{
		if(((Fan2RPM*100)-RPMACC)>RPM1)
		{
			if(FanValue<MaxOutValue)
				FanValue++;	
		}
		else if(((Fan2RPM*100)+RPMACC)<RPM1)
		{
			if(FanValue>0x00)
				FanValue--;	
		}		
	}
}

void Fan2MainControl(void)
{
	if(Fan2RPM>=Fan1RPM)
	{
		if(((Fan2RPM*100)-RPMACC)>RPM1)
		{
			if(FanValue<MaxOutValue)
				FanValue++;	
		}
		else if(((Fan2RPM*100)+RPMACC)<RPM1)
		{
			if(FanValue>0x00)
				FanValue--;	
		}			
	}
    else
	{	
		if(((Fan1RPM*100)-RPMACC)>RPM1)
		{
			if(FanValue<MaxOutValue)
				FanValue++;	
		}
		else if(((Fan1RPM*100)+RPMACC)<RPM1)
		{
			if(FanValue>0x00)
				FanValue--;	
		}
	}
}

//-----------------------------------------------------------------------------
// The function of fan rpm control
//-----------------------------------------------------------------------------
void FanControl(void)
{
	if(	RPM1==0x0000)
	{
		FanValue = FanInitValue;
	}
	
    if(Fan1RPM!=0x00)
    {
        Fan1MainControl();
    }
    else
    {
        Fan2MainControl();
    }
}


//-----------------------------------------------------------------------------
// Init. thermal table 1
//-----------------------------------------------------------------------------
void InitThermalTable1(void)
{
	BYTE index;
	index=0x00;
	while(index<(sizeof(ThermalTalbe1)/sizeof(thermal)))
	{
		*ThermalTalbe1[index].FanOn=ThermalTalbe1[index].CFanOn;
		*ThermalTalbe1[index].FanOff=ThermalTalbe1[index].CFanOff;
		*ThermalTalbe1[index].RPM=ThermalTalbe1[index].CRPM;
		index++;
	}
}

//-----------------------------------------------------------------------------
// Init. thermal table 2
//-----------------------------------------------------------------------------
void InitThermalTable2(void)
{
	BYTE index;
	index=0x00;
	while(index<(sizeof(ThermalTalbe2)/sizeof(thermal)))
	{
		*ThermalTalbe2[index].FanOn=ThermalTalbe2[index].CFanOn;
		*ThermalTalbe2[index].FanOff=ThermalTalbe2[index].CFanOff;
		*ThermalTalbe2[index].RPM=ThermalTalbe2[index].CRPM;
		index++;
	}
}

//-----------------------------------------------------------------------------
// Fan full on control
//-----------------------------------------------------------------------------
void FanFullOn(void)
{

	FanValue = MaxOutValue;
	Fan2Value = MaxOutValue-3;
	FanFullOnCont = FullOnCounter;		    //2sec
}

//-----------------------------------------------------------------------------
// Fan full off control
//-----------------------------------------------------------------------------
void FanFullOff(void)
{
	FanValue = 0x00;
	Fan2Value = 0x00;
}

//-----------------------------------------------------------------------------
// Fan Init on control
//-----------------------------------------------------------------------------
void FanInitOn(void)
{
	FanValue = FanInitValue;
	FanFullOnCont = FullOnCounter;		    //2sec
}

//-----------------------------------------------------------------------------
// Fan Init on control
//-----------------------------------------------------------------------------
void S5S0FanInitOn(void)
{
	FanValue = S5S0FanInitValue;
	Fan2Value = S5S0FanInitValue-3;
	FanFullOnCont = S5S0FullOnCounter;		    //10sec
}

//-----------------------------------------------------------------------------
// Fan Init on control
//-----------------------------------------------------------------------------
void S4S0FanInitOn(void)
{
	FanValue = S4S0FanInitValue;
	Fan2Value = S4S0FanInitValue-3;
	FanFullOnCont = S4S0FullOnCounter;		    //10sec
}


//-----------------------------------------------------------------------------
// Fan Init on control 风扇初始化控制
//-----------------------------------------------------------------------------
void S3S0FanInitOn(void)
{
	FanValue = S3S0FanInitValue;
	Fan2Value = S3S0FanInitValue-3;
	FanFullOnCont = S3S0FullOnCounter;		    //2sec
}


//-----------------------------------------------------------------------------
// Clear fan manual control 
//-----------------------------------------------------------------------------
void Clear_Fan_Debug(void)
{
	DebugFan1RPMT = 0x00;
	DebugFan2RPMT = 0x00;
	CLEAR_MASK(DebugFanStatus,Debug_Fan_enable);
}



//-----------------------------------------------------------------------------
// The function of get fan current RPM
//-----------------------------------------------------------------------------
void GetRPM1(void)
{
    BYTE tach_H;
    BYTE tach_L;

    tach_H = F1TMRR;
    tach_L = F1TLRR;   //{F1TMRR,F1TLRR} is the current RPM of RegValue, read from Reg
    if((tach_H!=0x00)||(tach_L!=0x00))
    {
		if(((tach_H*256) + tach_L)<RPM1Max)
		{
            RPM1=(4312500/TACH1P/((tach_H*256) + tach_L));   //calculate current RPM
		}
        else
        {
            RPM1=RPM1Max;
        }
    }
    else
    {
        RPM1=0x0000;
    }
	
	//J80_010--	FAN1_CurrentRPM = RPM1;
	//J80_010++>>
	FAN1_RPM_L = RPM1;
	FAN1_RPM_H = RPM1>>8;
	//J80_010++<<
}

//-----------------------------------------------------------------------------
// The function of smart fan
// base 100ms-C
//-----------------------------------------------------------------------------
void GetRPM2(void)
{
    BYTE tach_H;
    BYTE tach_L;

    tach_H = F2TMRR;
    tach_L = F2TLRR;   //{F1TMRR,F1TLRR} is the current RPM of RegValue, read from Reg
    if((tach_H!=0x00)||(tach_L!=0x00))
    {
		if(((tach_H*256) + tach_L)<RPM1Max)
		{
            RPM2=(4312500/TACH1P/((tach_H*256) + tach_L));   //calculate current RPM
		}
        else
        {
            RPM2=RPM1Max;
        }
    }
    else
    {
        RPM2=0x0000;
    }
	
	//J80_010--	FAN2_CurrentRPM = RPM2;
	//J80_010++>>
	FAN2_RPM_L = RPM2;
	FAN2_RPM_H = RPM2>>8;
	//J80_010++<<
}


//-----------------------------------------------------------------------------
// The function of smart fan
// base 100ms-C
//-----------------------------------------------------------------------------
void FanManager(void)
{
	if(FanFullOnCont!=0x00)  // Fan full speed count (continue time)
	{
		FanFullOnCont--;
		return;
	}
//--------------------- Fan dust mode control stop -----------------------


	if(((DebugFan1RPMT==0x00) || (DebugFan2RPMT == 0x00)) && (IS_MASK_CLEAR(DebugFanStatus,Debug_Fan_enable)))// In normal control mode
	{
		if((Fan1RPM!=0x00)||(Fan2RPM!=0x00))
		{
			//J80_019A-->>
			/*
            GetRPM1();       //get fan current RPM
            GetRPM2();
            */
            //J80_019A--<<
			FanControl();
		}
		else
		{
			if(FanValue != 0x00)
			{
				RPM1=0x0000;
				FanFullOff();
			}
		}

		if(FanValue > 3)
			Fan2Value = FanValue - 3;
	}
	else//------------------------------------In RPM debug mode
	{	
		//J80_019A-->>
		/*
		GetRPM1();
		GetRPM2();
		*/
        //J80_019A--<<
		SET_MASK(DebugFanStatus,Debug_Fan_enable);
		if((DebugFan1RPMT==0x00) && (DebugFan2RPMT == 0x00))
		{
			FanValue = 0;
			Fan2Value = 0;
			CLEAR_MASK(DebugFanStatus,Debug_Fan_enable);
			return;
		}	
			
		if(RPM1==0x0000)
		{
			FanValue = FanInitValue;
		} 
		else if(RPM2==0x0000)
		{
			Fan2Value = FanInitValue;
		}
		else
		{
			if(((DebugFan1RPMT*100)-RPMACC)>RPM1)
			{
				if(FanValue<MaxOutValue)
					FanValue++;	
			}
			else if(((DebugFan1RPMT*100)+RPMACC)<RPM1)
			{
				if(FanValue>0x00)
					FanValue--;	
			}

			if(((DebugFan2RPMT*100)-RPMACC)>RPM2)
			{
				if(Fan2Value<MaxOutValue)
					Fan2Value++;	
			}
			else if(((DebugFan2RPMT*100)+RPMACC)<RPM2)
			{
				if(Fan2Value>0x00)
					Fan2Value--;	
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Moniter temperature
//-----------------------------------------------------------------------------
const sTooHot code asTooHot[]=
{
	{ &TCPU_temp, 	CpuTooHot, &VeryHotCnt, 	CpuTooHotTimes },
	{ &TPCH_temp,   PCHTooHot, &PCHVeryHotCnt, 	PCHTooHotTimes }	
};

//-----------------------------------------------------------------------------
//T=87  centigrade	Enable  EC throttle	50%  
//T=77  centigrade  Disable EC throttle 50%
//T=89  centigrade  EC shutdown System
// base 1sec-C
//-----------------------------------------------------------------------------
void MoniterTemprature(void)
{
	if(Debugtemperature1!=0x00)
		TCPU_temp = Debugtemperature1;
	else
		TCPU_temp = CpuTm;

	if(TCPU_temp >= ProchotOnTM)  // CPU drop half frequency
	{
		Oem_Globe_Flag1 |= Oem_CPU_Temp_OV_f;
		//PCH_PROCHOT_ON();
	}
	else if(TCPU_temp <= ProchotOffTM) // CPU restore all frequency
	{		
		Oem_Globe_Flag1 &= ~Oem_CPU_Temp_OV_f;
		//PCH_PROCHOT_OFF();
	}
		
	// CPU温度过高 关机
	if(TCPU_temp >= ShutdownTM)    //EC shutdown
	{
		CPU_TooHot_Cnt += 1;
		if(CPU_TooHot_Cnt>3)
		{
			BRAMDebug(TCPU_temp);

			OEM_Debug_flag |= OEM_Temp_Stop_f;

			BRAM_TEMP_OV = 0x55;
			ETWCFG   = 0x20;
        	EWDCNTLR = WATCHDOG_COUNTER_1S;    // for 1s counter
        	EWDKEYR  = 0x5A;     // External WDTreset not match
			while(1);							// Wait for watch dog time-out	
		}
	}
	else
	{
		CPU_TooHot_Cnt = 0x00;
	}
}

//-----------------------------------------------------------------------------
// Function Name : ReadCPUTemp   1sec time base
//-----------------------------------------------------------------------------
#if GetCPUTempSupport
void ReadCPUTemp(void) 
{
    if(SystemIsS0)
    {
        // Use PECI interface to read cpu temperature
        #ifdef PECI_Support
            #if ReadCPUTemperature
            if(CPUTjmax==0x00)         // Need to read CPU Tjmax
            {
                PECI_ReadCPUTJMAX();
            }
            else
            {
                PECI_ReadCPUTemp();     // Use PECI interface to read cpu temperature
            }
            #endif
        #endif 
        
        // Use thermal sensor to read cpu temperature
        //SET_MASK(SMBus2Flag1, PollRemote);
    }
}
#endif

//-----------------------------------------------------------------------------
// Function Name : ReadSysTemp
//-----------------------------------------------------------------------------
#if GetSysTempSupport
void ReadSysTemp(void) 
{
    if(SystemIsS0)
    {

		#if OEM_GPU_ITS_SUPPORT
		if(IS_MASK_SET(GPU_INFO_FLG, GPU_INIT_OK))
		{
			SET_MASK(SMBus2Flag1, GPUPackage);
			CLEAR_MASK(GPU_INFO_FLG, GPU_INIT_OK);
		}	
		#endif
    }
}
#endif

//-----------------------------------------------------------------------------
// Function Name : InitThermalChip
//-----------------------------------------------------------------------------
void InitThermalChip(void)
{
	BYTE LimitTm, ReadData;
	LimitTm = 125;
	TmToSensor = 0xFF;


	bRWSMBus(SMbusCh2, SMbusWB, 0x98, 0x19, &LimitTm, 0);
	bRWSMBus(SMbusCh2, SMbusWB, 0x98, 0x20, &LimitTm, 0);

	if (bRWSMBus(SMbusCh2, SMbusRB, 0x98, 0x19, &ReadData, 0))		// Read for compare
		if (ReadData == LimitTm)
			if (bRWSMBus(SMbusCh2, SMbusRB, 0x98, 0x20, &ReadData, 0))
				if (ReadData == LimitTm)
					TmToSensor = LimitTm;

	if (TmToSensor == 0xFF)		// Write fail, enable CR03 bit 1
	{
		if (bRWSMBus(SMbusCh2, SMbusRB, 0x98, 0x03, &ReadData, 0))
		{
			SET_MASK(ReadData,BIT1);
			bRWSMBus(SMbusCh2, SMbusWB, 0x98, 0x03, &ReadData, 0);	//Set CR03 bit 1

			bRWSMBus(SMbusCh2, SMbusWB, 0x98, 0x19, &LimitTm, 0);
			bRWSMBus(SMbusCh2, SMbusWB, 0x98, 0x20, &LimitTm, 0);

			if (bRWSMBus(SMbusCh2, SMbusRB, 0x98, 0x19, &ReadData, 0))		// Read for compare
				if (ReadData == LimitTm)
					if (bRWSMBus(SMbusCh2, SMbusRB, 0x98, 0x20, &ReadData, 0))
						if (ReadData == LimitTm)
							TmToSensor = LimitTm;
		}
	}
}    


