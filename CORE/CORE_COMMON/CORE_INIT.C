/*-----------------------------------------------------------------------------
 * TITLE: CORE_INIT.C
 *
 * Author : Dino
 *
 * Copyright (c) ITE INC. All Rights Reserved.
 *
 *---------------------------------------------------------------------------*/

#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

//----------------------------------------------------------------------------
// FUNCTION: cold_reset - Initialization when power goes on
//----------------------------------------------------------------------------
void cold_reset(void)
{
	Ext_Cb2.byte = EXT_CB2_INIT;    		/* Update Ext_Cb3 a little further down */   
	Save_Typematic = SAVE_TYPEMATIC_INIT;   
	Save_Kbd_State = (SAVE_KBD_STATE_INIT & ~maskLEDS) | ( 0& maskLEDS);   

    Led_Ctrl = LED_CTRL_INIT;
    Led_Data = 0;
    Gen_Info = 0;   /* Clear general control. */

    Int_Var.Ticker_10 = 10;

    Flag.PASS_READY = 0;    /* Password not loaded. */
    Int_Var.Ticker_100 = 50;

    Flag.SCAN_INH = 1;      /* Scanner transmission inhibited. */
    Flag.VALID_SCAN = 0;

    Flag.NEW_PRI_K = 0;
    Flag.NEW_PRI_M = 0;

    Flag.LED_ON = 1;
    Ccb42 = CCB42_INIT; // 禁能键盘

    Pass_On = 0;      /* Password enable send code. */
    Pass_Off = 0;     /* Password disable send code. */
    Pass_Make1 = 0;   /* Reject make code 1. */
    Pass_Make2 = 0;   /* Reject make code 2. */

    MULPX = 0;

}


//----------------------------------------------------------------------------
// FUNCTION:   general_reset1, general_reset2
// Initialize things common to both Cold/Warm reset.
//----------------------------------------------------------------------------
static void common_reset(void)  /* MCHT, 27/05/04 */ // static void general_reset1(void)
{
    Service  = 0;       /* Clear service requests */
    Service1 = 0;

    Kbd_Response = 0;   /* Clear response codes */
    Timer_A.byte = 0;   /* Clear Timer A events */
    Timer_B.byte = 0;   /* Clear Timer B events */
    Scanner_State = 0;  /* Clear scanner state */
}


//----------------------------------------------------------------------------
// FUNCTION: Init_Mouse - Initialize the Intelligent mouse variables.
//----------------------------------------------------------------------------
void Init_Mouse (void)
{

}

void Core_Initialization(void)
{
	//TF_006A++>>
	/*
	KSO16CtrlReg = 0x00;
	KSO17CtrlReg = 0x00;
	*/
	//TF_006A++<<
    Init_Kbd();
	cold_reset();        // 所谓的冷复位 我猜想只是初始化自定义的一些参数
	common_reset();      // 初始化一些自定义的一些寄存器参数状态
	Init_Mouse();        // 初始化鼠标设备，但是好像没用
	Init_Timers();       // 初始化51 内部的定时器1 与定时器2
    Core_Init_Regs();    // 初始化部分寄存器的处置，并初始化 主模块
    Core_Init_SMBus();   // 初始化自定义一个数值
}

//----------------------------------------------------------------------------
// The function of clearing internal and external ram for kernel code
// Clear external ram (0x000~0x0FF)
// Clear internal ram (0x20~0xCF)
//----------------------------------------------------------------------------
void Core_Init_ClearRam(void)
{
	IIBYTE *IdataIndex;
	PORT_BYTE_PNTR byte_register_pntr;
	IdataIndex = 0x20;
    byte_register_pntr=0;
    while(byte_register_pntr<0x100)// Clear external ram (0x000~0x0FF)        not  (0x000~0xFFF)
    {
        *byte_register_pntr=0;
        byte_register_pntr ++;
    }							
	while(IdataIndex<0xC0)			// Clear internal ram (0x20~0xBF)
    {
        *IdataIndex=0;
        IdataIndex ++;
    }
}

//----------------------------------------------------------------------------
// The function of init. registers of kernel code
//----------------------------------------------------------------------------
const sREG_INIT code Core_reg_init_table[] = 
{
 		{ &FMSSR 		,Core_Init_FMSSR    }, 	// share rom size
// HOST interface
    	{ &SPCTRL1      ,Core_Init_I2EC     },  // Enable I2EC R/W  好像没有使能P80L page411 SPCTRL1
    	{ &BADRSEL      ,Core_Init_BADR     },  // Base address select 0b00:2E 2F 0b01:4E 4F 0b10:User define
                                                // 这里定义的基地址2E, 2F 

    	{ &SWCBALR      ,Core_Init_SWCBALR  },
    	{ &SWCBAHR      ,Core_Init_SWCBAHR  }, // 配置地址为0x002E？？
		{ &RSTS			,0x84				}, // 这个复位不应该在最前面执行的么？	

// KBC and PM interface
		{ &KBIRQR 		,0x00              	}, 	// Disable KBC IRQ
        { &KBHICR 		,IBFCIE+PM1ICIE     }, 	// KBC port control IBFCIE+PMICIE
        { &PM1CTL		,0x81},//IBFIE+SCINP        }, 	// EC port control  IBFCIE+SCI low active
 		{ &PM1IC        ,0x06},
 		{ &PM1IE        ,0x06},
 		
 		
        { &PM2CTL 		,IBFIE+SCINP        }, 	// EC port2 control IBFCIE+SCI low active  
// PS2
        { &PSCTL1 		,PS2_InhibitMode    }, 	// Enable Debounce,Receive mode, Inhibit CLK
        { &PSCTL2 		,PS2_InhibitMode    }, 	// Enable Debounce,Receive mode, Inhibit CLK
        { &PSCTL3 		,PS2_InhibitMode    }, 	// Enable Debounce,Receive mode, Inhibit CLK
        #if TouchPad_only
		{ &PSINT1		,0x04				}, 	// Transation Done Interrupt
        { &PSINT2		,0x04				},	// Transation Done Interrupt 
        { &PSINT3		,0x04				},	// Transation Done Interrupt 
        #else
		{ &PSINT1		,0x06				}, 	// Transation Done,Start Clock Interrupt
        { &PSINT2		,0x06				},	// Transation Done,Start Clock Interrupt 
        { &PSINT3		,0x06				},	// Transation Done,Start Clock Interrupt 
        #endif
		
// Key Scan
        { &KSOCTRL   	,KSOOD + KSOPU      },	// 开漏上拉
  		{ &KSICTRL   	,KSIPU       		},	// 上拉
};

void Core_Init_Regs(void)
{
	BYTE index = 0x00;
    while(index < (sizeof(Core_reg_init_table)/sizeof(sREG_INIT)))
    {
        Tmp_XPntr = Core_reg_init_table[index].address;
		*Tmp_XPntr = Core_reg_init_table[index].initdata;
        index ++;
    }

    CLEAR_MASK(FLHCTRL2R,BIT3);  // BIT3 应该是reserved 什么意义
	DCache = 0x03; // Disable Caches
}