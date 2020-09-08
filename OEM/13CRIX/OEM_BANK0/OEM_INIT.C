/*-----------------------------------------------------------------------------
 * TITLE: OEM_INIT.C
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

//----------------------------------------------------------------------------
// Oem_StartUp
//----------------------------------------------------------------------------
void Oem_StartUp(void)
{

}

//----------------------------------------------------------------------------
// Oem_Initialization - Initialize the registers, data variables, and oem 
// functions after kernel init. function.
//----------------------------------------------------------------------------
void Oem_Initialization(void)
{

	LWORD *pnt;
//	WORD Fvalue = 0x4000;

    // 好像没有 sha1_auth() 这个函数
	pnt = sha1_auth(&HashRandom);           // only for compile
	

	Init_GPIO();  // 初始化所有引脚

	//TF_010++>>
	if(IS_BOARD_ID1_HI())   //  判断GPIOA5 是否为高
		EC_PWR_CTR1_ON();   //  设置GPIOC0 为1

	//TF_010++<<
    #ifdef HSPI
	GPIO_HSPI_INIT();
    #endif

    #ifdef SPIReadMode

    //// 设置SPI模式 这里采用模式0 即：
    //  * SPIReadMode_0  // Read cycle 
    //    SPIReadMode_1  // Fast read cycle
    //    SPIReadMode_2  // Fast read dual output
    //    SPIReadMode_3  // Fast read dual input/output
    ChangeSPIFlashReadMode(SPIReadMode); 
    #endif

    #ifdef HSRSMode
    // 这里根本不支持高速SPI
    ChangeHSRSMode(HSRSMode);
    #endif

    #ifdef PLLFrequency   
    // 设置PLL 默认PLL 为32.3M 否则PLL power down 并关闭所有中断，进入睡眠模式
    ChangePLLFrequency(PLLFrequency);
    #endif

    #ifdef PECI_Support 
    Init_PECI();
    #endif

    // 厂商定义的初始化函数
	Init_Regs();

    #ifdef SMBusChannel4Support
    // 保留chanel 4 
    InitSMBusChannel4();
    #endif

    // 设置SMBus 为100K,并初始化SMBus的三个通道， 设置100K SMbus 速率
    // 关于这个100K参数是怎么算出的，待考究
	Init_SMBus_Regs();
    
    #if ADCFuncSupport    
	Init_ADC();
    #endif

    #if CIRFuncSupport	
	Init_CIR();
    #endif

    // 这两个函数基本是空
	Init_Cache();
	Init_PS2Port();
    
    #ifdef SysMemory2ECRam	
	InitSysMemory2ECRam();
    #endif

	//Label:BLD_TIPIA_001
    // 设置长按8S复位
	#if _8S_Reset_Support
	GCR10 = 0x01;
	GCR8 = 0x10;
	#endif

	
    InitSMBus();    // 实际上初始化一些SMbus 参数
    ResetSCIEvent();
	Init_OEMVariable();

	if(ExtendScanPin==0x02)		// KSO16 and KSO17 are used.
	{  
		if(ExternMatrixGPO)
		{
			ExtendScanPin++;
		}
	}

	ExtWDTInit();	    // 初始化看门狗
	InitThermalChip();  // 初始化电热芯片，这个是啥不懂了？应该和芯片无关

    // 初始化USB-C芯片参数  这个应该属于应用层
	#if Support_ANX7447
	ucsi_init();
	#endif

	Warning_flag = 0xFF;	//TF_004++
}

//----------------------------------------------------------------------------
// The function of init. registers 
//----------------------------------------------------------------------------
//const struct REG_INIT_DEF code reg_init_table[] = 
const sREG_INIT_DEF code reg_init_table[] = 
{
        { &FMSSR 		,Init_FMSSR         }, 	// share rom size
// HOST interface
    	{ &SPCTRL1      ,Init_I2EC+0x80          },  // Enable I2EC R/W
    	{ &BADRSEL      ,Init_BADR          },  // Base address select 0b00:2E 2F 0b01:4E 4F 0b10:User define
    	{ &SWCBALR      ,Init_SWCBALR       },
    	{ &SWCBAHR      ,Init_SWCBAHR       },
		{ &RSTS			,0x84				},
//        { &GCR1         ,0x05               },

// KBC and PM interface
		{ &KBIRQR 		,0x00              	}, 	// Disable KBC IRQ
        { &KBHICR 		,IBFCIE+PM1ICIE     }, 	// KBC port control IBFCIE+PMICIE
//        { &PM1CTL		,IBFIE+SCINP        }, 	// EC port control  IBFCIE+SCI low active
//       { &PM2CTL 		,IBFIE+SCINP        }, 	// EC port2 control IBFCIE+SCI low active  
// KBC and PM3 interface
        #ifdef PMC3_Support
        { &PM3CTL 		,IBFIE         	    }, 	// PMC3 input buffer full interrupt enable
        #endif	

// PS2
        { &PSCTL1 		,PS2_InhibitMode    }, 	// Enable Debounce,Receive mode, Inhibit CLK
        { &PSCTL2 		,PS2_InhibitMode    }, 	// Enable Debounce,Receive mode, Inhibit CLK
        { &PSCTL3 		,PS2_InhibitMode    }, 	// Enable Debounce,Receive mode, Inhibit CLK
		{ &PSINT1		,0x04				}, 	// Transation Done,Start Clock Interrupt
        { &PSINT2		,0x04				},	// Transation Done,Start Clock Interrupt 
        { &PSINT3		,0x04				},	// Transation Done,Start Clock Interrupt 

// Key Scan
        { &KSOCTRL   	,KSOOD + KSOPU      },	//
  		{ &KSICTRL   	,KSIPU       		},	//
	
// ADC		
        // 初始化默认ADC 默认寄存器：
        // 1. 先将 AINITB @ADCSTS 设置为1 并之后设置为0， 仅仅在上电后执行一次，并将ADCCTS1设置为 1
  		// 2. 将ADC配置全部清空  ADCCFG 配置为 0
        // 3. 自动硬件校准启用 AHCE @ KDCTL 配置为 1
        // 4. 将 ADCCTS0 @ ADCCFG 配置为 1，与第一点配合，及 {ADCCTS1, ADCCTS0} = 11b ,Conversion Time = 200*251 us = 52ms
        // 5. 默认配置 选择时钟分频因子 为 0x15 ，实际上启用默认配置时候后，使用0xFA
        { &ADCSTS    	,AINITB             }, 	// Enable Analog accuracy initialization
        //{ &ADCSTS    	,0                 	}, 	// Stop adc accuracy initialization
        { &ADCSTS    	,0x80          		}, 	// Stop adc accuracy initialization
		{ &ADCCFG    	,0         			}, 	//
        { &KDCTL     	,AHCE              	}, 	// Enable Hardware Callibration
		{ &ADCCFG    	,DFILEN  		    },	// Enable Digital Filter enable,
		{ &ADCCTL		,0x15				},

// DAC
        { &DACPWRDN  	,0x00              	}, 	//

//PWM
//========================================================================================
// Setp 1, Group Main Clock Select
//	PCFSi	@ PCFSR    bit 0-3	   PCFSR[0-3]	 = 06h = 0110b
//	G6MSELi @ CLK6MSEL bit 0-3	   CLK6MSEL[0-3] = 00h = 0b
//
//	Group  {G6MSELi, PCFSi}   Value    Main Clock
//	  0    {G6MSEL0, PCFS0}   00	   32.768kHz
//	  1    {G6MSEL1, PCFS1}   01	   9.2MHz
//	  2    {G6MSEL2, PCFS2}   01	   9.2MHz
//	  3    {G6MSEL3, PCFS3}   00	   32.768kHz
//
//========================================================================================
// Setp 2, Group Main clock fractional frequency
// Group		 Prescaler		  Value 	Group Clock
//	 0		C0CPRS				[00]		G0=32768/((0+1)*(63+1)) 	 = 512Hz
//	 1		C4MCPRS   C4CPRS	[00,05] 	G1=9200000/((05+1) *(63+1))  = 23958.33Hz
//	 2		C6MCPRS   C6CPRS	[00,8F] 	G2=9200000/((143+1)*(63+1))  = 998.26Hz
//	 3		C7MCPRS   C7CPRS	[00,00] 	G3=32768/((0+1)*(63+1)) 	 = 512Hz
//
//	CTR0 is 0x3F, and CTR1/CTR2/CTR3 is not write
//========================================================================================
// Setp 3, Channel Select Group Clock

//	Select		Prescaler	 Group Clock
//	 0 0	C0CPRS			  G0
//	 0 1	C4MCPRS  C4CPRS   G1
//	 1 0	C6MCPRS  C6CPRS   G2
//	 1 1	C7MCPRS  C7CPRS   G3
//
//						Channel: C7 C6 C5 C4 C3 C2 C1 C0
// [PCSSGH,CCSSGL] = [00,01]h = [00 00 00 00 00 00 00 01]b
//						  Group: G1 G0 G0 G0 G0 G2 G0 G0
//========================================================================================
//According to the mentioned, Set PWM
//channel 2: Beep		-----> G2	 1KHz
//channel 4: blacklight -----> G0	 512Hz
//channel 7: Fan		-----> G1	 24KHz
//=========================================================================================
		{ &ZTIER        ,0x00       		},   // Disable
        { &CTR      	,0x3F      		    },	
		{ &C4CPRS    	,0x05				},	 // Fan = 24K HZ
		{ &C4MCPRS	    ,0x00				},   //
		{ &C6CPRS    	,0x8F				},	 // Beep =1KHz
		{ &C6MCPRS		,0x00				},	 //
		{ &C7CPRS    	,0x00				},	 // 
		{ &C7MCPRS		,0x00				},   //
		{ &PCFSR        ,0x8A      		    },	
        { &PCSSGL       ,0x00      		    },   // Select channel 0-3 Group
		{ &PCSSGH       ,0x41				},	
		{ &ZTIER        ,0x02       		},   // Enable
        { &DCR0         ,0x00      		    },   // Default 
        { &DCR4         ,0x00      		    },   // Default 
        { &DCR7         ,0x00      			},   // Default

		{&PWM1LCR1		,0x30				},
		{&PWM1LCR2		,0x22				},		
		{&PWM1LHE		,0x00				},
};


void Init_Regs(void)
{
	BYTE index = 0x00;
    while(index < (sizeof(reg_init_table)/sizeof(sREG_INIT_DEF)))
    {
        Tmp_XPntr = reg_init_table[index].address;
		*Tmp_XPntr = reg_init_table[index].initdata;
        index ++;
    }
}

//const struct REG_INIT_DEF code Init_SMBus_table[] = 
const sREG_INIT_DEF code Init_SMBus_table[] = 
{
// SMBus   100K 

	    { &SMB4P7USL	, 0x28				},	// 100K	
	    { &SMB4P0USH  	, 0x25      		},  
        { &SMB300NS     , 0x03      		},  
        { &SMB250NS     , 0x02      		},   
        { &SMB25MS      , 0x19      		},   
        { &SMB45P3USL   , 0xA5      		},  
        { &SMBS4P3USH   , 0x01      		},   
        { &SMB4P7A4P0H	, 0x00      		},  // 这个寄存器有什么意义？


/*
	    { &SMB4P7USL	, 0x71				},	// 20K	
	    { &SMB4P0USH  	, 0xdd      		},  
        { &SMB300NS     , 0x03      		},  
        { &SMB250NS     , 0x71      		},   
        { &SMB25MS      , 0x19      		},   
        { &SMB45P3USL   , 0x5c      		},  
        { &SMBS4P3USH   , 0x01      		},   
        { &SMB4P7A4P0H	, 0x00      		},  
*/

        { &HOCTL2_A 	, 0x01      		},  
        { &HOCTL_A   	, 0x03      		},  
        { &HOCTL_A  	, 0x01      		},  
        { &HOSTA_A    	, 0xFF      		},  
        
        { &HOCTL2_B   	, 0x01      		},  
        { &HOCTL_B    	, 0x03      		},	
        { &HOCTL_B    	, 0x01      		},	
        { &HOSTA_B    	, 0xFF      		},	
        
    	{ &HOCTL2_C		, 0x01				},	 
   	 	{ &HOCTL_C		, 0x03				},	 
   		{ &HOCTL_C		, 0x01				},	 
    	{ &HOSTA_C		, 0xFF				},
            
        #ifdef SMBusChannel4Support
    	{ &HOCTL2_D		, 0x01				},	 
   	 	{ &HOCTL_D		, 0x03				},	 
   		{ &HOCTL_D		, 0x01				},	 
    	{ &HOSTA_D		, 0xFF				},
        #endif
};

void Init_SMBus_Regs(void)
{
	BYTE index = 0x00;
    while(index < (sizeof(Init_SMBus_table)/sizeof(sREG_INIT_DEF)))
    {
        Tmp_XPntr = Init_SMBus_table[index].address;
		*Tmp_XPntr = Init_SMBus_table[index].initdata;
        index ++;
    }
}

//----------------------------------------------------------------------------
// The function of clearing external ram for OEM code
// Clear external ram (0x100~0xFFF)
//----------------------------------------------------------------------------
void Init_ClearRam(void)
{
	PORT_BYTE_PNTR byte_register_pntr;

    byte_register_pntr=0x100;
    while(byte_register_pntr<0x1000)// Clear external ram (0x100~0xFFF)
    {
        *byte_register_pntr=0;
        byte_register_pntr ++;
    }

}

//----------------------------------------------------------------------------
// The function of DMA for scratch sram
//----------------------------------------------------------------------------
const sDMAScratchSRAM code asDMAScratchSRAM[]=    // For 8500 Dino
{
	{	&SCRA1H,	&SCRA1M,	&SCRA1L	},		// 1024 bytes (externl ram 0x800 ~ 0xBFF)
	{	&SCRA2H,	&SCRA2M,	&SCRA2L	},		// 512 bytes  (externl ram 0xC00 ~ 0xDFF)
	{	&SCRA3H,	&SCRA3M,	&SCRA3L	},		// 256 bytes  (externl ram 0xE00 ~ 0xEFF)
	{	&SCRA4H,	&SCRA4M,	&SCRA4L	}		// 256 bytes  (externl ram 0xF00 ~ 0xFFF)

};
void  CacheDma(BYTE sramnum,WORD addr)
{
	*asDMAScratchSRAM[sramnum].scarh = 0x80;
	*asDMAScratchSRAM[sramnum].scarm = (WORD) addr >> 8;   	// high byte of function address
	*asDMAScratchSRAM[sramnum].scarl = (WORD) addr & 0xff;	// low byte of function address
	*asDMAScratchSRAM[sramnum].scarh = 0x00;	// start cache dma     			
}

//----------------------------------------------------------------------------
// Init. DMA for scratch sram
//----------------------------------------------------------------------------
void Init_Cache(void)
{
#ifdef ITE8500					// Dino For 8500 no cache ram 0x800 ~ 0xFFF

#else
    #ifdef HSPI
        #ifdef HSPI_DefferingMode
        CacheDma(3, HSPI_RamCode);  // Cache to memory 0xF00 ~ 0xFFF
        #endif
    #endif
#endif

    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}

//----------------------------------------------------------------------------
// Select touch pad, mouse, and keyboard port number
//----------------------------------------------------------------------------
void Init_PS2Port(void)
{

}

//----------------------------------------------------------------------------
// Function Name : Init_OEMVariable
// Purpose : setup default variable when EC reset
//----------------------------------------------------------------------------
void Init_OEMVariable(void)
{
	Oem_GPU_EC_BLK = Oem_GPU_BLK + Oem_EC_BLK;

	LED_FLASH_CNT = 0x0001;	// for LED control

	ECMainVersionInfo = OEM_Version_MSB;
	VCMD_Lenovo = OEM_Version_MSB;
	ECVersionInfo = OEM_Version_LSB;
	ECVersionPCB = OEM_Version_PCB;
	
	PECI_PowerCtrlStep = 0x01;

	Device_Status_Lenovo |= Wireless_S_B;
	Device_Status_Lenovo |= Bluetooth_S_B;
	Device_Status_Lenovo |= Inverter_S_B;
	Device_Status_Lenovo |= Camera_S_B;	

	Oem_Device_Status |= Wireless_IN;
	Oem_Device_Status |= Bluetooth_IN;
	Oem_Device_Status |= Inverter_IN;
	Oem_Device_Status |= Camera_IN;

	Oem_Globe_Flag2 = 0;
	SysPostTime = 0;
	
    
	SysPowState=SYSTEM_S5;		

	//TF_014++>>
	if(BRAM[63]==0x55)
	{
		BRAM[63]=0x00;
		CLEAR_MASK(KBHISR,SYSF);
        Ccb42_SYS_FLAG=0;
		DelayDoPowerSeq=250;
		Oem_TriggerS5S0();
	}
	//TF_014++<<
	
	if(Read_ACIN())
	{
		BAT1_STATUS |= OS_AC_IN;
		POWER_FLAG1	|= adapter_in;
		ECQEvent(AC_HOT_PLUG_EVT,SCIMode_Normal);
	}
	RamDebug(0x30);
	Hook_EnablePS2Port_0();
	
}

/*-----------------------------------------------------------------------------
 * @subroutine - ucsi_ppm_notify_opm
 * @function - ucsi_ppm_notify_opm
 * @Upstream - By call
 * @input    -
 * @return   -
 * @note     -
 *---------------------------------------------------------------------------*/
void ucsi_ppm_notify_opm(void)
{
    /* Generate SCI/Q-event to notify the OPM. */
    RamDebug(0x79);
    RamDebug(xUCSI_DS_CCI3);
    RamDebug(xUCSI_DS_CCI2);
    RamDebug(xUCSI_DS_CCI1);
    RamDebug(xUCSI_DS_CCI0);
}

//----------------------------------------------------------------------------
// EC leaves follow mode or EC scatch ROM and jump to main function
//  Note:
//      Before jump to main function [WinFlashMark = 0x33;] is necessary.
//----------------------------------------------------------------------------
void Hook_ECExitFollowMode(void)
{
    Init_Cache();  // 初始化高速缓冲存储器  但是这个软件版本号没用到
    
    #ifdef HSPI
        #ifdef HSPI_DefferingMode
        SET_MASK(IER7,Int_DeferredSPI); // Enable HSPI interrupt
        #endif
    #endif 
    
    InitSMBus();            //  只是初始化SMBUS 一些参数
    
    #if Insyde_Bios         //是否支持 insyde bios 系统，  insyde bios 是台湾系微公司开发的bios系统



    if(RamCodePort==0x64)   // for insyde bios
    {
        KBHIKDOR = 0xFA;
    }
    else if(RamCodePort==0x66)
    {
	    PM1DO = 0xFA;
    }	
    else if(RamCodePort==0x6C)
    {
	    PM2DO = 0xFA;
    }  
    #endif  // end Insyde_Bios
}

//----------------------------------------------------------------------------
// EC leaves follow mode or EC scatch ROM and jump to main function
//  Note:
//      1. Before jump to main function if want to keep EC setting [return(0x33);] is necessary.
//      2. If you don't understand the use of Hook_ECRetunrMainFuncKeepCondition function, 
//          don't change anything.
//      3. Always at bank0 (0x9000)
//----------------------------------------------------------------------------
BYTE Hook_ECRetunrMainFuncKeepCondition(void)
{
    if(WinFlashMark==0x33)
    {
        return(0x33);
    }
    else
    {
        return(0x00);
    }
}
