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
// GPDRH6 = 1  set
void PWR_S5_EN_H(void)
{
	PWR_S5_EN_HI();			
	POWER_RAMDEBUG(0x01);
}

// GPDRH6 = 0  clear
void PWR_S5_EN_L(void)
{
	// 不禁用type-C 充电，就必须关闭GPH6 ？？？
	if(IS_MASK_CLEAR(xbBIOS_Notify_Flag,typecChargeDisable))	//TF_009++
		PWR_S5_EN_LO();			 // GPH6 
	POWER_RAMDEBUG(0x02);
}

// GPH5
BYTE STB_PWRGD_OK(void)
{
	// 判断 GPH5 == 1
	if(Read_STB_PWRGD())
	{
		POWER_RAMDEBUG(0x03);
		return(1);
	}
	POWER_RAMDEBUG(0x04);
	return(0);
}

// GPF3 = 1  set
void ALWON_H(void)
{
	ALWON_ON();
	POWER_RAMDEBUG(0x05);
}

// GPF3 = 0  clear
void ALWON_L(void)
{
	ALWON_OFF();			// GPF3
	POWER_RAMDEBUG(0x06);
}

// GPH0 = 1 set
void VCOR_EN(void)
{
	VCORE_EN_HI();			// GPH0
	POWER_RAMDEBUG(0x07);
}

void VCOR_DisEnable(void)
{
	VCORE_EN_LO();			// GPH0
	POWER_RAMDEBUG(0x08);
}

BYTE Wait_VCORE_OK(void)
{
	if(IS_VCORE_OK())		// GPE0
	{
		POWER_RAMDEBUG(0x09);
		return(1);
	}
	POWER_RAMDEBUG(0x0A);
	return(0);
}

void VDD_EN_H(void)
{
	VDD_EN_ON();			// GPA0
	POWER_RAMDEBUG(0x0B);
}

void VDD_EN_L(void)
{
	VDD_EN_OFF();			// GPA0
	POWER_RAMDEBUG(0x0C);
}

BYTE Wait_VDDQ_PG(void)
{
	if(Read_VDDQ_PG())		// GPB6
	{
		POWER_RAMDEBUG(0x0D);
		return(1);
	}
	POWER_RAMDEBUG(0x0E);
	return(0);
}

// 这个函数没有用
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
	EC_VTT_EN_HI();			// GPH3
	POWER_RAMDEBUG(0x11);
}

void VTT_EN_L(void)
{
	EC_VTT_EN_LO();			// GPH3
	POWER_RAMDEBUG(0x12);
}

void PX_EN_H(void)
{
	PX_EN_HI();				// GPH4
	POWER_RAMDEBUG(0x13);
}
	
void PX_EN_L(void)
{
	PX_EN_LO();				// GPH4
	POWER_RAMDEBUG(0x14);
}
void PX_EN2_H(void)
{
	PX_EN2_HI();			// GPH1
	POWER_RAMDEBUG(0x15);
}

void PX_EN2_L(void)
{
	PX_EN2_LO();			// GPH1
	POWER_RAMDEBUG(0x16);
}


// GPD1
void CPU_RST_H(void)
{
	CPU_RST_HI();
	POWER_RAMDEBUG(0x17);
}

// GPD1
void CPU_RST_L(void)
{
	CPU_RST_LO();			//GPD1
	POWER_RAMDEBUG(0x18);
}

void PCIRST_H(void)
{
	PCIRST1_HI();		// GPF4
	PCIRST2_HI();		// GPF5
	//TF_008-->>
	/*
    if(SysPowState==SYSTEM_S0_S3)
		PCIRST3_LO();
	*/
	//TF_008--<<
	PCIRST3_HI();		// GPE5
	POWER_RAMDEBUG(0x19);
}

void PCIRST_L(void)
{
	PCIRST1_LO(); // GPF4		m.2 SSD 电源
	PCIRST2_LO(); // GPF5	
	//TF_008A--	if(SysPowState != SYSTEM_S0_S3)	
	
	// IS_BOARD_ID1_LO = GPA5
	// IS_BOARD_ID1_HI = GPA5

	// 从原理图看 GPA5 连接board ID1 的ECVCC 高电平
	if(((SysPowState != SYSTEM_S0_S3) && (IS_BOARD_ID1_LO())) || (IS_BOARD_ID1_HI()))	//TF_008A++	
		PCIRST3_LO();	// GPE5  = 0
	POWER_RAMDEBUG(0x1A);
}

void V1P8_EN_H(void)
{
	V1P8_EN_HI();				// GPE1
	POWER_RAMDEBUG(0x1D);
}

void V1P8_EN_L(void)
{
	V1P8_EN_LO();				// GPE1
	POWER_RAMDEBUG(0x1E);
}

BYTE Wait_ALWGD(void)
{
	if(Read_ALWGD())  			// GPE6
	{
		POWER_RAMDEBUG(0x1F);
		return(1);
	}
	POWER_RAMDEBUG(0x20);
	return(0);
}

void FUSB3_PWREN_H(void)
{
	FUSB3_PWREN_HI();		// GPB1    --- > 1
 	FUSB3_PWREN1_HI();		// GPF2    --- > 1
 	POWER_RAMDEBUG(0x21);
}

void FUSB3_PWREN_L(void)
{
   FUSB3_PWREN_LO();	// GPB1
   FUSB3_PWREN1_LO();	// GPF2
   POWER_RAMDEBUG(0x22);
}

//TF_008++>>
void PCIRST3_L(void)
{
	// GPA5 这个引脚从原理图上看有上拉电阻 
	// 所以，原则上不会进入if 里面
	if(IS_BOARD_ID1_LO()) //TF_008A++  GPA5
		PCIRST3_LO();		// GPE5
	POWER_RAMDEBUG(0x23);
}
//TF_008++<<

//-------------------------------------------------------------------------------------
//  Do S5 -> S0 power sequnce  开机过程
//-------------------------------------------------------------------------------------

// 上电时序 数组参数  函数   延时时间   是否检查状态引脚
const sPowerSEQ code asPowerSEQS5S0[]=  
{
	{ NULL_FUNCTION,	   10,	     0,	},	// Dummy step always in the top of tabel
	{ PWR_S5_EN_H, 		    0,		 0, },	// 1					GPH6	---> 1
	//TF_007--	{ STB_PWRGD_OK,  20, 1, },	// 2					
	{ STB_PWRGD_OK, 	   80,		 1, },	// 2	//TF_007++		判断 GPH5 STB_PWRGD
	{ ALWON_H, 		       30,		 0, },	// 3					GPF3	---> 1
    { InitSio,		      130,		 0,	},	//						初始化 Super IO
    { VDD_EN_H, 		   10,		 0, },	// 9					GPA0	---> 1
	{ VTT_EN_H, 		    0,		 0, },	// 9					GPH3	---> 1
	{ Wait_VDDQ_PG,	       50,		 1,	},  // 5	//J80_012++		判断 GPB6 VDDQ_PG
	{ VCOR_EN,			   30,		 0,	},  // 4					GPH0	---> 1
    { Wait_VCORE_OK,	   20,		 1,	},  // 5					判断 GPE0 VCORE_PG
    { V1P8_EN_H,		    0,		 0,	},  // 4					GPE1	---> 1
    { PX_EN_H,		        0,		 0,	},  // 4					GPH4	---> 1
    { PX_EN2_H,		       10,		 0,	},  // 4					GPH1	---> 1
    { Wait_ALWGD,		  220,		 1,	},  // 7  					判断 GPE6 ALLSYS_PG	
    { PCIRST_H,		       20,		 0,	},  // 7  					GPF4 GPF5 GPE5 		---> 1
	{ CPU_RST_L, 		   20,		 0, },	// 12 					GPD1				---> 0	
	{ CPU_RST_H, 		    0,		 0, },	// 12 					GPD1				---> 1
	{ S5_S0Variable,		0,		 0,	}, 	// always in the end of tabel
};

//-------------------------------------------------------------------------------------
//  Do S0 -> S5 power sequnce  关机序列
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS0S5[]=  
{
	{ NULL_FUNCTION,		0,		 0, },	// Dummy step always in the top of tabel	
	{ CPU_RST_L, 		    0,		 0, },	// 12				GPD1 0    		---> 0
	{ PCIRST_L,		        0,		 0,	},  // 7				GPF4 GPF5 GPE5  ---> 0
    { PX_EN2_L,		      220,		 0,	},  // 4				GPH1			---> 0
	{ PX_EN_L,		        0,		 0,	},  // 4				GPH4			---> 0
	{ V1P8_EN_L,		   30,		 0, },	// 4	//J80_012++ 				---> 0
	{ VCOR_DisEnable,	   20,		 0, },	// 4				GPH0			---> 0
	{ VTT_EN_L, 		   20,		 0, },	// 9				GPH3			---> 0
	{ VDD_EN_L, 		   60,		 0, },	// 9				GPA0			---> 0
	//TF_010--	{ ALWON_L,			    0,		 0, },	// 3
	{ ALWON_L,			   60,		 0, },	// 3	//TF_010++	GPF3			---> 0
	{ PWR_S5_EN_L, 		    0,		 0, },	// 1	//J80_005++ 如果在bios 中 不禁用typeC充电， 就关闭 GPH6 

	{ S0_S5Variable,		0,		 0,	}, 	// always in the end of tabel
}; 

//-------------------------------------------------------------------------------------
//  Do S4 -> S0 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS4S0[]= 
{
	{ 0x0000, 				0,		 0, },	// Dummy step always in the top of tabel
	{ PWR_S5_EN_H, 		    0,		 0, },	// 1					GPH6	---> 1
	//TF_007--	{ STB_PWRGD_OK, 20,	 1,	},  // 2
	{ STB_PWRGD_OK,	       80,		 1,	},  // 2	//TF_007++		判断 GPH5 STB_PWRGD
	{ ALWON_H, 		       30,		 0, },	// 3					GPF3	---> 1
    { InitSio,		      130,		 0,	},	//						初始化 Super IO
    { VDD_EN_H, 		   10,		 0, },	// 9					GPA0	---> 1
	{ VTT_EN_H, 		    0,		 0, },	// 9					GPH3	---> 1
	{ Wait_VDDQ_PG,	       50,		 1,	},  // 5	//J80_012++		判断 GPB6 VDDQ_PG
	{ VCOR_EN,			   30,		 0,	},  // 4					GPH0	---> 1
    { Wait_VCORE_OK,	   20,		 1,	},  // 5 					判断 GPE0 VCORE_PG
    { V1P8_EN_H,		    0,		 0,	},  // 4					GPE1	---> 1
    { PX_EN_H,		        0,		 0,	},  // 4					GPH4	---> 1
    { PX_EN2_H,		       10,		 0,	},  // 4					GPH1	---> 1
    { Wait_ALWGD,		  220,		 1,	},  // 7					判断 GPE6 ALLSYS_PG
    { PCIRST_H,		       20,		 0,	},  // 7					GPF4 GPF5 GPE5 		---> 1
	{ CPU_RST_L, 		   20,		 0, },	// 12					GPD1				---> 0	
	{ CPU_RST_H, 		    0,		 0, },	// 12					GPD1				---> 1
	{ S4_S0Variable,		0,		 0,	}, 	// always in the end of tabel

}; 

//-------------------------------------------------------------------------------------
//  Do S0 -> S4 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS0S4[]=  
{
	{ 0x0000, 				0,		0,  },	// Dummy step always in the top of tabel
	{ CPU_RST_L, 		    0,		 0, },	// 12					GPD1  			---> 0
	{ PCIRST_L,		        0,		 0,	},  // 7					GPF4 GPF5 GPE5  ---> 0
    { PX_EN2_L,		      220,		 0,	},  // 4					GPH1			---> 0
	{ PX_EN_L,		        0,		 0,	},  // 4					GPH4			---> 0
	{ V1P8_EN_L,		   30,		 0, },	// 4	//J80_012++		GPE1			---> 0
	{ VCOR_DisEnable,	   20,		 0, },	// 4					GPH0			---> 0
	{ VTT_EN_L, 		   20,		 0, },	// 9					GPH3			---> 0
	{ VDD_EN_L, 		   60,		 0, },	// 9					GPA0			---> 0
	{ ALWON_L,			    0,		 0, },	// 3					GPF3			---> 0
	{ PWR_S5_EN_L, 		    0,		 0, },	// 1    J80_005++ 如果在bios 中 不禁用typeC充电， 就关闭 GPH6 
	{ S0_S4Variable,		0,		 0,	}, 	// always in the end of tabel		修改系统电源状态和从S0->Sx的通用掉电配置
}; 

//-------------------------------------------------------------------------------------
//  Do S3 -> S0 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS3S0[]= 
{
	{ 0x0000, 				0,		 0, },	// Dummy step always in the top of tabel
	{ PCIRST3_L, 		    0,		 0, },	// 3		//TF_008++  似乎不会拉低什么
	{ ALWON_H, 		       30,		 0, },	// 3		GPF3 				---> 1
	{ Wait_VDDQ_PG,	       20,		 1,	},  // 5 		判断 GPB6 VDDQ_PG
	{ VCOR_EN,			   30,		 0,	},  // 4		GPH0  				---> 1
    { Wait_VCORE_OK,	   20,		 1,	},  // 5 		判断 GPE0	VCORE_PG
    { V1P8_EN_H,		    0,		 0,	},  // 4		GPE1				---> 1
    { PX_EN_H,		        0,		 0,	},  // 4		GPH4				---> 1
    { PX_EN2_H,		       10,		 0,	},  // 4		GPH1				---> 1
    { Wait_ALWGD,		  220,		 1,	},  // 7		判断 GPE6 ALLSYS_PG
    { PCIRST_H,		       20,		 0,	},  // 7		GPF4 GPF5 GPE5 		---> 1
	{ CPU_RST_L, 		   20,		 0, },	// 12		GPD1				---> 0
	{ CPU_RST_H, 		    0,		 0, },	// 12		GPD1				---> 1
	{ S3_S0Variable,		0,		 0,	}, 	// always in the end of tabel

}; 

//-------------------------------------------------------------------------------------
//  Do S0 -> S3 power sequnce
//-------------------------------------------------------------------------------------
const sPowerSEQ code asPowerSEQS0S3[]=  
{
	{ 0x0000, 				0,		 0, },	// Dummy step always in the top of tabel
	{ CPU_RST_L,			0,		 0, },	// 12	GPD1 			---> 0
	{ PCIRST_L, 			0,		 0, },	// 7	GPF4 GPF5 GPE5  ---> 0
	{ PX_EN2_L, 		  220,		 0, },	// 4	GPH1			---> 0		
	{ PX_EN_L,				0,		 0, },	// 4	GPH4			---> 0	
	{ V1P8_EN_L,		   30,		 0, },	// 4	GPE1			---> 0
	{ VCOR_DisEnable,	   80,		 0, },	// 4	GPH0			---> 0
	{ ALWON_L,			   50,		 0, },	// 3	GPF3			---> 0
	{ S0_S3Variable,		0,		 0,	}, 	// always in the end of tabel
}; 

//-----------------------------------------------------------------------------
//  The OEM variable of Sx -> S0
//-----------------------------------------------------------------------------
void OEM_SX_S0CommVar(void)
{
    CheckPowerStateIndex = 0x00;	// 这个参数好像没被调用，不知道是不是会被共享
	
}

//-----------------------------------------------------------------------------
//  The variable of Sx -> S0
//-----------------------------------------------------------------------------
void SX_S0CommVar(void)
{
	// 修改系统电源状态状态
	SysPowState=SYSTEM_S0;
	
	// 停止ps2设备将数据发送到KBC的功能
	PS2DeviceInactive();    // For Touchpad issue

	// 设置引脚为 复用功能模式(PS2 CLK, data)
    GPCRF0 = ALT;           // PS2 interface
    GPCRF1 = ALT;
	
	// 复位SCI 事件
	ResetSCIEvent();

	// 复位电源状态索引参数 CheckPowerStateIndex = 0
    OEM_SX_S0CommVar();
	
	// 初始化SMBUS
    InitSMBus();

	// 关于Sys_Power_Status 这个参数不清楚他的位实际意义
	Sys_Power_Status = (Sys_Power_Status & 0x0f0);

	// 这个参数与 TouchPad 相关，不明白其定义
	if(Oem_RTP_ID_CMD == 0)
	{
		guoyong003 = 0x99;
		guoyong001 = 0x00;

		Oem_RTP_ID_LOOP_Index = 0;

		Oem_RTP_ID_Step = 0;

		Oem_RTP_ID_1s_EN = 0x01;
		Oem_RTP_ID_1s_T = 0;
	}


	FIN_PWR_EN_ON();			//  GPG2 ---> 1 
	M2PWR_ENT_ON();				// 	GPJ0 ---> 1
	MUTE_ON();					// 	GPJ1 ---> 1
	CAM_PWR_EN_ON();			// 	GPC5 ---> 1
	WIFI_ENABLE_ON();			// 	GPC7 ---> 1
	TPS2546_CTL1_HI();			//  GPB0 ---> 1
	BT_EN_ON();					//	GPG6 ---> 1 
	//TF_002A--	BL_ENABLE_EC_HI();
}

//-----------------------------------------------------------------------------
// The variable of S5 -> S0  开机状态
//-----------------------------------------------------------------------------
void S5_S0Variable(void)
{
	// 通用的Sx -> S0 数据控制
	SX_S0CommVar();

	// 初始化 风扇
	S5S0FanInitOn();

	//==================================
	// USB Power Set on
	//==================================
	// USB postposition port 1-3
	FUSB3_PWREN_H();		// GPB1 GPF2  ---> 1
	EnableADCModule();		// 使能ADC 模块
}

//-----------------------------------------------------------------------------
// The variable of S4 -> S0
//-----------------------------------------------------------------------------
void S4_S0Variable(void)
{
	// 通用的Sx -> S0 数据控制
    SX_S0CommVar();

	// 初始化 风扇
	S5S0FanInitOn();
	BL_ENABLE_EC_HI();	//TF_002A++		GPB2	---> 1
}

//-----------------------------------------------------------------------------
// The variable of S3 -> S0
//-----------------------------------------------------------------------------
void S3_S0Variable(void)
{
	// 通用的Sx -> S0 数据控制
	SX_S0CommVar();

	// 风扇初始化控制
	S3S0FanInitOn();
	BL_ENABLE_EC_HI();	//TF_002A++		GPB2
}

//-----------------------------------------------------------------------------
//  The OEM variable of S0 -> Sx
//-----------------------------------------------------------------------------
void OEM_S0_SXCommVar(void)
{
    CheckPowerStateIndex = 0x00;	// 这个参数好像没有什么用处，没被作为右值或者判断标志
    CLEAR_MASK(SYS_MISC1,ACPI_OS);	// 系统退出ACPI 环境
}

//-----------------------------------------------------------------------------
//  The variable of S0 -> Sx 系统从正常使用状态变化为 Sx 状态的通用函数
//-----------------------------------------------------------------------------
void S0_SXCommVar(void)
{
#ifdef PECI_Support
    PECI_ClearPSTVariables();
#endif
    
#if TouchPad_only
    PowerOffClearTPPendingData();	// 清除PS2 buffer,关闭PS2 中断，关闭PS2 的扫描和发送服务
#endif

	CLEAR_MASK(KBHISR,SYSF);		// 清空状态寄存器中系统标志
    Ccb42_SYS_FLAG=0;
	
#if CIRFuncSupport	
	Init_CIR();
#endif

	InitSMBus();					// 初始化SMBUS 标志位，软件初始化，因为只是一些软件设置的状态位
    OEM_S0_SXCommVar();

	SPCTRL1 = Init_I2EC+0x80;		// 0x43 + 0x80 = 0xC3  200D I2EC 可读写操作，P80L使能， 接受80端口数据

	if(PLLFREQR != PLLFreqSetting03)	// 如果 PLL设置不为 32.3 M 修改PLL，并进入深度睡眠模式
	{
		ChangePLLFrequency(PLLFreqSetting03);
	}	

	FIN_PWR_EN_OFF();				// GPG2 
	M2PWR_ENT_OFF();				// GPJ0	
	MUTE_OFF();						// GPJ1 	
	CAM_PWR_EN_OFF();				// GPC5
	WIFI_ENABLE_OFF();				// GPC7
	TPS2546_CTL1_LO();				// GPB0
	BT_EN_OFF();					// GPG6
	Process_KBLED_Control(SET_KBLED_ON,SYSTEM_S5);  // 控制键盘指示灯
  	LCD_	CTRL = 0;	
	BL_ENABLE_EC_LO();				// GPB2
}

//-----------------------------------------------------------------------------
//  The variable of S0 -> S5  关机状态
//-----------------------------------------------------------------------------
// 第一步: 修改 电源状态
// 第二步: 查看EC刷新标志位是否置位，如果置位清空标志位，并开启看门狗刷新EC阈值

void S0_S5Variable(void)
{
    SysPowState=SYSTEM_S5;

	// ec已刷新，应重置ec域
	if(IS_MASK_SET(Oem_For_Bios_Flag,EC_flashed))
	{
		CLEAR_MASK(Oem_For_Bios_Flag,EC_flashed);
		
		// 这几个是电池的ram空间
		BRAM_FLASH_ID0 = 0x55;
		BRAM_FLASH_ID1 = 0xaa;
		BRAM_FLASH_ID2 = 0x55;
		BRAM_FLASH_ID3 = 0xaa;
		
		_nop_();
		_nop_();
		_nop_();
		
		// 设置外部看门狗
		ETWCTRL = 0x10; 	   //EWDCMS  External WDT stop count mode select   1: External WDT can be stopped by setting EWDSCEN bit,	0: External WDT cannot be stopped
		
		// 设置2s 的看门狗计数
		EWDCNTLR = WATCHDOG_COUNTER_2S;//reset ec domain		
		while(1);				// Wait for watch dog time-out
	
	}

    S0_SXCommVar();				// 从开机到关机的通用控制

	//==================================
	// USB Power Set off
	//==================================
	// USB postposition port 1-3
	FUSB3_PWREN_L();		// GPB1， GPF2  ---> 0

	// 清除状态
	CLEAR_MASK(EVT_STATUS2, Sys_Shutdown_f);		
	CLEAR_MASK(EVT_STATUS2, Sys_ResetPowerOn_f);

	// 关闭ADC 模块
	DisableADCModule();

	// 下面这些操作不明白什么意思 ???  SYS_state 的各个位代表什么意思 ???
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
	// 修改系统电源状态
    SysPowState=SYSTEM_S4;
    S0_SXCommVar();
}

//-----------------------------------------------------------------------------
// The variable of S0 -> S3
//-----------------------------------------------------------------------------
void S0_S3Variable(void)
{
	// 修改系统电源状态， 并电泳 S0-> Sx 之间数据的通用操作
	SysPowState=SYSTEM_S3;
	S0_SXCommVar();
	DRAM_CTRL_LO();	//J80_012++  GPD0

	// 这个状态不大懂
	SYS_state = SYS_state & 0xFE;
}

//----------------------------------------------------------------------------
// 触发 S3 到 S0 
//----------------------------------------------------------------------------
void Oem_TriggerS3S0(void)
{
    if((SystemIsS0) || (SysPowState==SYSTEM_S3_S0))
    {
        return;
    }
	// 设置状态
	SysPowState=SYSTEM_S3_S0;
	PWSeqStep = 1;
	PowSeqDelay = 0x00;

	DRAM_CTRL_HI();			// GPD0  ---> 1
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
//  触发S4 ~ S0
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
	// 如果系统状态处于S0 或者处于S5-S0 之间的话，就不执行函数
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
// S5 to S0 sequnce		S5 -> S0 电源序列
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
// S4 to S0 sequnce  S4 到 S0 电源序列
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
// S3 to S0 sequnce  休眠唤醒
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
		else				// Check input status pin
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
// 触发S0 -> S3 状态，正常工作状态到睡眠模式
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
// 触发S0 -> S4 状态，正常工作状态到休眠模式
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
//	触发 S0 -> S5 
//----------------------------------------------------------------------------
void Oem_TriggerS0S5(BYTE causecode)
{
	// 如果系统本来就处于关机状态，就不执行下面操作
    if((SystemIsS5) || (SysPowState==SYSTEM_S0_S5))
    {
        return;
    }

	// 设置关机事件
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
// S0 to S3 sequnce  开机到睡眠操作
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

	// 电源管理的列表循环操作
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
// S0 to S4 sequnce 开机状态到休眠状态
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

	// 下面操作与 S0-S5 一样
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
// S0 to S5 sequnce  关机序列
//-----------------------------------------------------------------
void Oem_S0S5Sequence(void)
{
	// 延时操作，因为 Oem_SysPowerContrl 是1ms 事件，所以这里的延时基准值为1ms
    if ( PowSeqDelay != 0x00 )
    {
        PowSeqDelay--; 				//1m Sec count base
        if ( PowSeqDelay != 0x00 )
        {
            return;
        }
    }

	// 操作关机的序列表
	while(PWSeqStep<(sizeof(asPowerSEQS0S5)/sizeof(sPowerSEQ)))
	{	
		if(asPowerSEQS0S5[PWSeqStep].checkstatus==0x00)	// Do function  不需要检查状态引脚的操作序列
		{
			(asPowerSEQS0S5[PWSeqStep].func)();
			PowSeqDelay = asPowerSEQS0S5[PWSeqStep].delay;
			PWSeqStep++;			// 时序数组索引值加1

			// 如果该项操作需要延时，跳出while 进行循环操作
			if(PowSeqDelay!=0x00)
			{
				break;
			}
		}
		else											// Check input status pin
		{
			if((asPowerSEQS0S5[PWSeqStep].func)())		// 检查引脚状态操作
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

	//
	if((Sys_Power_Status & 0x7f) == 0x03)
	{
		resault = 0x01;	
	}
	
	// 1: 适配器是否断开
    if(IS_MASK_SET(POWER_FLAG1,adapter_in))//&&(IS_MASK_SET(BT1_STATUS1,bat_in)))		// AC power exist
    {
		BRAM_EnterDeep = 0x22;
		resault = 0x01;	
    }

	// 按下电源开关
    if(IS_MASK_SET(POWER_FLAG1,wait_PSW_off))	// power switch pressed
    {
        resault = 0x01;	
    }
	
	// 电池电量过低，但不是S0
	if(Oem_BatCtrl_F & Oem_BatCapTooLow_NotS0_f)
	{
		resault = 0x01;	
	}

	// 按下一键还原按钮
	if(IS_MASK_SET(POWER_FLAG1,wait_NovoBtn_off))
	{
        resault = 0x01;	
    }

	// 如果都不是以上的状态的话，就执行下面操作
	if(resault == 0x00)
	{
		if(DeepSleepCunt<10)	
		{
			DeepSleepCunt++;
			resault = 0x02;
		}
		else
		{
			// 检查是否进入深度睡眠模式是500ms 事件
			// 上面的判断有可能进这里的是 S4 或者 S5 状态，并且最少是 5s 之后的事情
			resault = 0x00;
			DeepSleepCunt = 0x00;
		}
	}
    else
    {
		// 如果result == 1 的话，清中断状态，并使能中断
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
// 进入深度睡眠模式
void EnterDeepSleep(void)
{
	//If BIOS notify keep always power, then not let EC enter deep sleep
	// 如果BIOS通知始终保持电源，则不要让EC进入深度睡眠
	// 不知道这个指令是不是只是的是系统正处于bios界面状态 ???
	if(IS_MASK_SET(SYS_MISC1,OFF_ALW))
		return;

	BRAM_EnterDeep1 = 0x88; // 设置电池 bram 中的 0x2208地址的数值 为0x88
//++++++++++++++++++++++++++++

	// Oem_AlwaysOn_f 为0x80
	// ~Oem_AlwaysOn_f = ~0x80 = 0x7F
	// 这段代码的意义在哪里 ???
	Oem_Globe_Flag &= ~Oem_AlwaysOn_f; 
	RamDebug(0x81);
//++++++++++++++++++++++++++++

	// 禁止中断
	IER0=0x00;
	IER1=0x00;
	IER2=0x00;
	IER3=0x00;
	IER4=0x00;
	IER9=0x00;
	IER15=0x00;
	
	// 关闭总中断
	DisableAllInterrupt();
							// Disable module
	// 仅能DAC
	DACPWRDN=0x3F;			// Disable DAC	


	// 关闭矩阵按键
	KSOL = 0xFF;
	KSOH1 = 0xFF;
	KSOH2 = 0xFF;

	// 设置各个模块的门限
	CGCTRL1R = 0xFF;
	CGCTRL2R = 0xFF;
	CGCTRL3R = 0xFF;

	//*************************************************************************
	// Set Wake up pin -> alt function
	//*************************************************************************
	GPCRE4 = ALT;			// pwrsw to alternate pin  GPE4 为 PWRSW 指示电源开关按键


	WUEMR2 |= 0x20;			// 设置边缘触发模式 
	WUESR2 |= 0x20;			// clear wakeup status
	WUENR2 |= 0x20;  		// 这个寄存器当中手册里面没有描述， 想必也是一个使能组

	ISR1 |= Int_WKO25;		// clear int14 for  pwrsw ，清唤醒中断
	IER1 |= Int_WKO25;		// enable int14 for  pwrsw  使能唤醒中断

   	WUEMR4 |= 0x01;         // Wake-up Edge WU40 for NOVO	 
   	WUESR4 |= 0x01;
	WUENR4 |= 0x01;         // Wake-up enable WU40 for NOVO Button
	ISR0 |= Int_WKINTAD; 	//  (Int_WKO20+Int_WKINTAD);     // clear int 1 for  SUSB
   							            // clear int 5 for  NOVO button	
   	IER0 |= Int_WKINTAD;    //(Int_WKO20+Int_WKINTAD);    // enable int 1 for  SUSB
   	
	// GPI4 这个引脚表示是否有电池接入， 低电平表示电池接入
	// 下面指示的是适配器接入
	if(Read_BatIN())        // Battery  in system
   	{
		GPCRI3 = INPUT;	
		//WUEMR13 |= 0x04;
		//WUESR13 |= 0x04;
        ISR15 |= Int_WKO122;	
        IER15 |= Int_WKO122;
	}
	else                   //AC in  system  适配器接入
	{	
		GPCRI4 = INPUT;       //battery in   WU20   INT1 

		// 触发方式，中断状态清除，中断使能
		WUEMR7 |= 0x10;
		WUESR7 |= 0x10;
		WUENR7 |= 0x10;

		ISR9 |= Int_WKO74;
		IER9 |= Int_WKO74;
     }


	//note:to check whether need to go to battery shipmode before the system go to G2 
	// 注意：在系统进入G2之前要检查是否需要进入电池出厂模式
	// 这个函数保留，找时间研究下电池控制内容
	 CheckBatShipMode();
	
	_nop_();
	_nop_();

	RamDebug(0x11);

	EnableAllInterrupt();

	// 将PCON中的PD位置1并进入EC掉电模式后，PLL将掉电, 进入 Sleep模式
	// page 206 有程序指南
	PLLCTRL = 1;
	PCON = 2 ;             	// enter sleep mode

	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	_nop_();

	RamDebug(0x22);

	// 禁止中断
	IER0=0x00;				
	IER1=0x00;
	IER2=0x00;
	IER3=0x00;
	IER4=0x00;
	IER15=0x00;
	
	// 禁中断 清中断
	IER1=0x00;
	ISR1=0xff;
	
	IER2=0x00;
	ISR2=0xff;
	
	IER0=0x00;
	ISR0=0xff;
	
	// 设置上升沿触发
	WUENR1 = 0x00;	
	WUENR2 = 0x00;	
	WUENR3 = 0x00;	
	WUENR4 = 0x00;	
	WUENR7 = 0x00;
	
	// 清唤醒中断状态寄存器
	WUESR1 = 0xff;
	WUESR2 = 0xff;
	WUESR3 = 0xff;
	WUESR4 = 0xff;
	WUESR7 = 0xFF;
	WUESR13 = 0xFF;
	
	//*************************************************************************
	//
	//************************************************************************* 
	GPCRI3 = INPUT; 		// ACIN to alternate pin   适配器接入
	GPCRE4 = INPUT; 		// pwrsw to alternate pin  电源按键
	GPCRI4 = INPUT; 		// battery in			   电池接入
	//*************************************************************************

	// 取消门限控制
	CGCTRL1R = 0x00;
	CGCTRL2R = 0x00;
	CGCTRL3R = 0x00;

	// 使能DAC模块
	DACPWRDN=0x00;			// enable DAC	

	// 使能键盘按键中断
	Enable_Any_Key_Irq();	// Enable Keyboard

	// 初始化定时器
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
                (Oem_TriggerS0S5SC_HWPS0Off);
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
	// ECAutoPowerOn 好像没有被赋值过， 参数地址在0x407
	if(ECAutoPowerOn==1)
	{
		ECAutoPowerOn = 0;
		DelayDoPowerSeq = 1000;
		Oem_TriggerS5S0();
	}
	
	// ECAutoPowerOn 好像没有被赋值过， 参数地址在0x408
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
	// 检查当前的系统PowerState，看看我们应该怎么做。
    switch (SysPowState)
    {
        case SYSTEM_S0:    // 系统处于正常运行状态
			MoniterHWPG(); // 空函数, 什么都不做
			break;

        case SYSTEM_S3:			
         	break;

        case SYSTEM_S4:
            break;

        case SYSTEM_S5:		// 系统处于关机状态，这个是检查是否会自动开机
			CheckAutoS5S0();
            break;

        case SYSTEM_S4_S0:	// 休眠状态唤醒
            Oem_S4S0Sequence();
            break;

        case SYSTEM_S5_S0:	// 关机唤醒
			
			Oem_S5S0Sequence();
			break;
				
        case SYSTEM_S3_S0:	// 睡眠唤醒
			Oem_S3S0Sequence();
            break;

        case SYSTEM_S0_S3:	// 电脑设置睡眠
			Oem_S0S3Sequence();
            break;

        case SYSTEM_S0_S4:	// 电脑设置休眠
            Oem_S0S4Sequence();
            break;

        case SYSTEM_S0_S5:	// 电脑关机
			Oem_S0S5Sequence();
         	break;

		case SYSTEM_EC_WDTRST: // 设置看门狗复位
			InternalWDTNow();
			break;

        default:
         	SysPowState=SYSTEM_S5; // 如果都没有状态，就默认是S5状态即为关机状态
         	break;
    }
}


// 检测复位源（掉电复位，内部看门狗或者外部看门狗）
// 关于GPA0 功能为EN_VDDQ，程序中配置的是输出模式，这个和电源指示灯一起的
// 这个函数在startup.A51 中被调用
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
	// 防止持续向芯片组拉低功率信号
	if(0==SBSWReleaseCunt)      // Prevent continued pull low power signal to chipset
	{
		SBSWReleaseCunt = 20;   // decrease timer base is 10ms total time 200ms
	}
}

//-----------------------------------------------------------------------------
// The function of releasing S.B. power button  10ms
//-----------------------------------------------------------------------------
// 这个函数没有被使用
void CheckSBPowerButton(void)
{
	if(SBSWReleaseCunt!=0x00)
	{
		SBSWReleaseCunt--;
	}
}


