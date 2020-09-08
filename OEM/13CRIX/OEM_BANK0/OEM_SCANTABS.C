/*-----------------------------------------------------------------
 * TITLE: OEM_SCANTABS.C - RC Table(s) for Scan Matix
 *
 * Copyright (c) 1983-2007, Insyde Software Corporation. All Rights Reserved.
 *
 * You may not reproduce, distribute, publish, display, perform, modify, adapt,
 * transmit, broadcast, present, recite, release, license or otherwise exploit
 * any part of this publication in any form, by any means, without the prior
 * written permission of Insyde Software Corporation.
 *----------------------------------------------------------------*/
 
#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

const unsigned char code Rc_ROM_Tables[] =   
{
//-------------------------------------------------------------------------------------------
//0  	1	   2     3	  4     5	  6  	7	  8     9	  A  	B	  C  	D	  E  	F
//ESC	F4	   NUL	 NUL    G 	  H	     F6    NUL   "'  	NUL   F5	 NUL   L-Alt NUL    U-Arr NUL
  0x76, 0xE3, 0x00, 0x00, 0x34, 0x33, 0xE5, 0x00, 0x52, 0x00, 0xE4, 0x00, 0x8A, 0x00, 0xED, 0x00,
//Tab	F3	   apL	 NUL    T 	  Y	     }]	   F7	 {[ 	L-Sh  Bksps	 NUL   NUL	 NUL    NUL	  NUL
  0x0D, 0xE2, 0x58, 0x00, 0x2C, 0x35, 0x5B, 0xE6, 0x54, 0x88, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00,
//Q  	E	   W 	 NUL    R  	  U	     I     O	 P  	NUL   NUL	 NUL   NUL	 NUL    NUL	  L-Win
  0x15, 0x24, 0x1D, 0x00, 0x2D, 0x3C, 0x43, 0x44, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82,
//~`	F2	   F1	 L-Ctl  %5	  ^6	+=	   F8	 _-	    NUL   F9	 NUL   NUL   Insert    Home  End
  0x0E, 0xE1, 0xE0, 0x8C, 0x2E, 0x36, 0x55, 0xE7, 0x4E, 0x00, 0xE8, 0x00, 0x00, 0xF5, 0xF3, 0xF4,
//A 	D	   S	 NUL	F	  J 	K	   L	 ;: 	NUL   |\	 NUL   NUL	 NUL	Fn	  App
  0x1C, 0x23, 0x1B, 0x00, 0x2B, 0x3B, 0x42, 0x4B, 0x4C, 0x00, 0x5D, 0x00, 0x00, 0x00, 0x8E, 0x84,
//!1	#3	   @2	 NUL   $4	  &7	*8	  (9	 )0 	NUL   F10	 F11   PtSc  F12	P-D   P-U
  0x16, 0x26, 0x1E, 0x00, 0x25, 0x3D, 0x3E, 0x46, 0x45, 0x00, 0xE9, 0xEA, 0xF1, 0xEB, 0xF8, 0xF7,
//Z  	C      X   	 R-Ctl  V  	  M	    ,<	   >.	 NUL	R-Sh  Enter  NUL   NUL	 NUL   Del	  NUL
  0x1A, 0x21, 0x22, 0x8D, 0x2A, 0x3A, 0x41, 0x49, 0x00, 0x89, 0x5A, 0x00, 0x00, 0x00, 0xF6, 0x00,
//NUL	NUL    NUL	 NUL    B  	  N	    NUL	   NUL    ?/	NUL   Space  D-Arr R-Alt R-Arr L-Arr  NUL
  0x00, 0x00, 0x00, 0x00, 0x32, 0x31, 0x00, 0x00, 0x4A, 0x00, 0xFA, 0xEE, 0x8B, 0xF0, 0xEF, 0x00,
};


//-------------------------------------------------------------------------------------------------
const unsigned char code EtKey_ROM_Tables[] =
{
    //16    17    18    Extended - 19X8 Scan Table
    
  //Num/ NumEnter					//+CY_CR016
    0x00,0x00,0x00,     // 00  0
  //Num+ Num1
    0x00,0x00,0x00,     // 03  1
  //Num4 Num2
    0x00,0x00,0x00,     // 01  2
  //Num0 \_(JP)
    0x00,0x00,0x00,     // 05  3
  //PageUp  Num
    0x96,0x00,0x00,     // 02  4
  //Num9 PageDown
    0x00,0x97,0x00,     // 04  5
  //Del    Num6
    0xF6,0x00,0x00,     // 06  6
  //Num3 Num5
    0x00,0x00,0x00,     // 07  7		//-CY_CR016
};

const unsigned char code KB_S3WakeUP_Tables[] =
{
    // Scanner Table 1 
	//00   01   02   03   04   05   06   07   08   09   10   11   12   13   14   15
	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55, //
 	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55, //
   	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55, //
 	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55, //
  	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55, //
  	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55, //
  	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55, //
  	0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55  //
};

const unsigned char code EtKey_S3WakeUP_Tables[] =
{	//16   17   18		// 19X8 Scan Table 
	0x55,0x55,0x55,
	0x55,0x55,0x55,
	0x55,0x55,0x55,
	0x55,0x55,0x55,
	0x55,0x55,0x55,
	0x55,0x55,0x55,
	0x55,0x55,0x55,
	0x55,0x55,0x55,
};

BYTE OEM_SetBackBT_Value_Fn(BYTE Up_Down_f)
{

	if (((Oem_Globe_Flag & Brightness_Up_Pressed) == 0) && (Up_Down_f !=0))
	{
		Oem_Globe_Flag |= Brightness_Up_Pressed;

		if (LCD_BRIGHTNESS<MAX_LEVEL)
		{					
			LCD_BRIGHTNESS = LCD_BRIGHTNESS +1;
		}

		return 1;
	}

	if (((Oem_Globe_Flag & Brightness_Down_Pressed) == 0) &&(Up_Down_f ==0))
	{
		Oem_Globe_Flag |= Brightness_Down_Pressed;
		
		if (LCD_BRIGHTNESS > 0)
		{
			LCD_BRIGHTNESS = LCD_BRIGHTNESS - 1;
		}	

		return 1;
	}
	
		return 0;

}

void OEM_SetBackBT_Value_Fn_NoEM_linux(BYTE Up_Down_f)
{
	if(Up_Down_f !=0)
	{
		if (LCD_BRIGHTNESS<MAX_LEVEL_linux)
			{
			LCD_BRIGHTNESS = LCD_BRIGHTNESS +1;
			//J80_013--	ECQEvent(0xd2,SCIMode_Normal);
			}

	}
	else
	{
	
		if (LCD_BRIGHTNESS > 0)
			{
			LCD_BRIGHTNESS = LCD_BRIGHTNESS - 1;
			//J80_013--	ECQEvent(0xd1,SCIMode_Normal);
			}

	}
}

void OEM_SetBackBT_Value_Fn_NoEM(BYTE Up_Down_f)
{
	if(Up_Down_f !=0)
	{
		if (LCD_BRIGHTNESS<MAX_LEVEL)
			LCD_BRIGHTNESS = LCD_BRIGHTNESS +1;

	}
	else
	{
	
		if (LCD_BRIGHTNESS > 0)
			LCD_BRIGHTNESS = LCD_BRIGHTNESS - 1;

	}
}
void OEM_Win7_SetBackBt_Fn(BYTE Up_Down_f)
{
	if(OEM_SetBackBT_Value_Fn(Up_Down_f))
	{		
		VPC_Lenovo |= Brightness_Bit_vpc;
		//J80_013--	ECQEvent(FN_SETBK_EVT,SCIMode_Normal);
	}
}


void OEM_Win7_SetBackBt_Fn_NoEM(BYTE Up_Down_f)
{
	OEM_SetBackBT_Value_Fn_NoEM(Up_Down_f);
	//J80_013--	ECQEvent(FN_SETBK_EVT,SCIMode_Normal);
}
void OEM_WinXP_SetBackBt_Fn(BYTE Up_Down_f) 
{
	if(OEM_SetBackBT_Value_Fn(Up_Down_f))
	{
		VPC_Lenovo |= Brightness_Bit_vpc;
		//J80_013--	ECQEvent(FN_SETBK_EVT,SCIMode_Normal);
		Setup_Brightness(LCD_BRIGHTNESS);
	}
}



void OEM_Fn_SetBK_NO_ACPI(BYTE Up_Down_f)
{
	
	OEM_SetBackBT_Value_Fn_NoEM(Up_Down_f);
	Setup_Brightness(LCD_BRIGHTNESS);

}







//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//----------------------------------------------------------------------------
// The function of hotkey Fn + F1
//----------------------------------------------------------------------------
void HotKey_Fn_F1(BYTE event)  //no use
{
	if(event == MAKE_EVENT)
	{
		e0_prefix_code(0x23, MAKE_EVENT);
	}
	else if(event == BREAK_EVENT)
	{
		e0_prefix_code(0x23, BREAK_EVENT);
	}
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F2
//----------------------------------------------------------------------------
void HotKey_Fn_F2(BYTE event)  //no use
{
	e0_prefix_code(0x21,event);
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F3
//----------------------------------------------------------------------------
void HotKey_Fn_F3(BYTE event)
{
	e0_prefix_code(0x32,event);
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F4
//----------------------------------------------------------------------------
void HotKey_Fn_F4(BYTE event)      
{
	if(event != BREAK_EVENT)
		return;
		
	#if Lenovo_Support	
	if(IS_MASK_SET(OemAllDevices_state,Pm_Em_exist))
	{
		SET_MASK(VPC2_Lenovo,MIC_VPC2);
		//J80_013--	ECQEvent(Fn_CAM_EVT,SCIMode_Normal);
	}

	if(IS_MASK_SET(Device_Status_AllRF,MIC_S_B))
	{
		CLEAR_MASK(Device_Status_AllRF,MIC_S_B);
		Buffer_Key(0x38);	// SEt2 MAKE 0x38(SET1 6A)  windows not used
		Buffer_Key(0xEA);	// SEt1 BREAK 0xEA windows not used
	}
	else
	{	
		SET_MASK(Device_Status_AllRF,MIC_S_B);
		Buffer_Key(0x40);	// SEt2 MAKE 0x40(SET1 6B)  windows not used
		Buffer_Key(0xEB);	// SEt1 BREAK 0xEB windows not used
	}
	#endif
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F5
//----------------------------------------------------------------------------
void HotKey_Fn_F5(BYTE event)
{
	e0_prefix_code(0x20,event);
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F6
//----------------------------------------------------------------------------
void HotKey_Fn_F6(BYTE event)
{
	//TF_011-->>
	/*
	if(event != MAKE_EVENT)
	{
		return;
	}
#if Lenovo_Support
	if( 0 != (Device_Status_Lenovo & TouchPad_S_B))
	{
		Device_Status_Lenovo &= ~TouchPad_S_B;
	
		if(1)//OS_Type == 3)
		{
			Hook_DisablePS2Port_0();
		}			
		
	}
	else
	{	
		Device_Status_Lenovo |= TouchPad_S_B;

		if(1)//OS_Type == 3)
		{
			Hook_EnablePS2Port_0();
		}
	}

	VPC_Lenovo |= TouchPad_Bit_vpc;
#endif
	*/
	//TF_011--<<
	//J80_013--	ECQEvent(FN_TP_EVT,SCIMode_Normal); // touchpad
	//TF_011++>>
	if (event == MAKE_EVENT)
		e0_prefix_code(0x03,event);
	else if (event == BREAK_EVENT)
		e0_prefix_code(0x03,event);
	//TF_011++<<
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F7
//----------------------------------------------------------------------------
void HotKey_Fn_F7(BYTE event)
{
	if(event != BREAK_EVENT)
		return;
	
	switch(OS_Type)
	{
		case 3:
			Buffer_Key(0x08);
			Buffer_Key(0xE4);			
			break;
		default:
			break;
	}
				
	#if Lenovo_Support	
		VPC2_Lenovo |= All_RF_VPC2;
		VPC2_Lenovo |= App_Control_RF_VPC2;
		//J80_013--	ECQEvent(FN_RF_EVT,SCIMode_Normal);
	#endif
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F8
//----------------------------------------------------------------------------
void HotKey_Fn_F8(BYTE event)
{
	if (event != BREAK_EVENT)
		return;
		
#if 0
		
	if( 0 != (Device_Status_Lenovo & Camera_S_B))
	{
		Device_Status_Lenovo &= ~Camera_S_B;
	}
	else
	{
		Device_Status_Lenovo |= Camera_S_B; 
	}
	
	SET_MASK(VPC_Lenovo,Camera_Bit_vpc);
	ECQEvent(Fn_CAM_EVT,SCIMode_Normal);
#endif
	#if Lenovo_Support
	if(IS_MASK_SET(OemAllDevices_state,Pm_Em_exist))  // EM exist, notify EM by BIOS
	{
		SET_MASK(VPC_Lenovo,Camera_Bit_vpc);
		//J80_013--	ECQEvent(Fn_CAM_EVT,SCIMode_Normal);
	}
	if(IS_MASK_SET(Oem_Device_Status,Camera_IN))
	{
		CLEAR_MASK(Device_Status_Lenovo,Camera_S_B);
		CLEAR_MASK(Oem_Device_Status,Camera_IN);
		CAM_PWR_EN_OFF();
		Buffer_Key(0x10);// SET2  MAKE 10 (SET1 65)
		Buffer_Key(0xE5);// SET1  BREAK E5 (SET2 F0 10)
	}
	else
	{
		SET_MASK(Device_Status_Lenovo,Camera_S_B);
		SET_MASK(Oem_Device_Status,Camera_IN);
		CAM_PWR_EN_ON();
		Buffer_Key(0x18);// SET2  MAKE 18 (SET1 66)
		Buffer_Key(0xE6);// SET1  BREAK E6 (SET2 F0 18)
	}
	#endif
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F9
//----------------------------------------------------------------------------
void HotKey_Fn_F9(BYTE event)
{
	if(event == MAKE_EVENT)
	{
		e0_prefix_code(0x14, MAKE_EVENT);
		e0_prefix_code(0x11, MAKE_EVENT);
		simple_code(0x4B,event);//L
	}
	else if (event == BREAK_EVENT)
	{ 
		simple_code(0x4B,event);//L
		//J80_001-->>
		/*
		e0_prefix_code(0x11, MAKE_EVENT);
		e0_prefix_code(0x14, MAKE_EVENT);
		*/
		//J80_001--<<
		//J80_001++>>
		e0_prefix_code(0x11, event);
		e0_prefix_code(0x14, event);
		//J80_001++<<
	}
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F10
//----------------------------------------------------------------------------
void HotKey_Fn_F10(BYTE event)
{
	if(event == MAKE_EVENT)
	{
		//J80_001A-->>
		/*
		Buffer_Key(0xE0);	 // win  make code
		Buffer_Key(0x1f);
		Buffer_Key(0x4d);	 // P make code
		SET_MASK(Oem_Globe_Flag2,Oem_F10_Make_f);	
		*/
		//J80_001A--<<
		e0_prefix_code(0x08, MAKE_EVENT);	//J80_001A++
	}
	else if(event == BREAK_EVENT)
	{
		//J80_001A-->>
		/*
		CLEAR_MASK(Oem_Globe_Flag2,Oem_F10_Make_count);
		CLEAR_MASK(Oem_Globe_Flag2,Oem_F10_Make_f);
		Buffer_Key(0x99);    //p break
		Buffer_Key(0xE0);  //WIN break code
		Buffer_Key(0xDB);
		*/
		//J80_001A--<<
		e0_prefix_code(0x08, BREAK_EVENT);	//J80_001A++
	}
	
	//J80_013--	ECQEvent(FN_DisplayMode_EVT,SCIMode_Normal);	
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F11
//----------------------------------------------------------------------------
void HotKey_Fn_F11(BYTE event)
{
	if (event == MAKE_EVENT)
		e0_prefix_code(0x01,event);
	else if (event == BREAK_EVENT)
		e0_prefix_code(0x01,event);
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + F12
//----------------------------------------------------------------------------
void HotKey_Fn_F12(BYTE event)
{
	if (event == MAKE_EVENT)
		e0_prefix_code(0x02,event);
	else if (event == BREAK_EVENT)
		e0_prefix_code(0x02,event);
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + ESC
//----------------------------------------------------------------------------
void HotKey_Fn_ESC(BYTE event)
{
	//J80_001B-->>
	/*
	if(event == MAKE_EVENT)
	{	
		//J80_001--	;
		//J80_001++>>
		if(LCD_CTRL == 0) 
			LCD_CTRL = 0xAA;
		else
			LCD_CTRL = 0x00;
		//J80_001++<<
	}
	*/
	//J80_001B--<<
	//J80_001B++>>
	if (event == MAKE_EVENT)
		e0_prefix_code(0x07,event);
	else if (event == BREAK_EVENT)
		e0_prefix_code(0x07,event);
	//J80_001B++<<
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + up arrow
//----------------------------------------------------------------------------
void HotKey_Fn_UP(BYTE event)
{RamDebug(0x8C);
	e0_prefix_code(0x7D,event);//page up
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + down arrow
//----------------------------------------------------------------------------
void HotKey_Fn_DOWN(BYTE event)
{RamDebug(0x8D);
	e0_prefix_code(0x7A,event);//page down
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + left arrow
//----------------------------------------------------------------------------
void HotKey_Fn_LEFT(BYTE event)
{RamDebug(0x8E);
	e0_prefix_code(0x6C,event);//home
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + right arrow
//----------------------------------------------------------------------------
void HotKey_Fn_RIGHT(BYTE event)
{RamDebug(0x8F);
	e0_prefix_code(0x69,event);//end
}

//----------------------------------------------------------------------------
// The function of hotkey Fn + Ins (Scr Lk)
//---------------------------------------------------------------------------
void HotKey_Fn_Ins(BYTE event)
{

}

//----------------------------------------------------------------------------
// The function of hotkey Fn + Del (NumLk)
//---------------------------------------------------------------------------
void HotKey_Fn_Del(BYTE event)
{

}

void HotKey_Fn_SPACE(BYTE event)    // Fn + space
{RamDebug(0x90);
	if(event == MAKE_EVENT)
	{	
		if(KBLED_Status == 0)
		{
			Process_KBLED_Control(SET_KBLED_ON,SysPowState);
			
		}
		else if(KBLED_Status == 1)
		{
			Process_KBLED_Control(SET_KBLED_OFF,SysPowState);
		}
	}
}



const FUNCT_PTR_V_B code HotKey_Fn_Fx[] =
{
	HotKey_Fn_F1,    	// Fn + F1
	HotKey_Fn_F2, 		// Fn + F2
	HotKey_Fn_F3, 		// Fn + F3
	HotKey_Fn_F4,       // Fn + F4
	HotKey_Fn_F5,    	// Fn + F5
	HotKey_Fn_F6, 		// Fn + F6
	HotKey_Fn_F7, 		// Fn + F7
	HotKey_Fn_F8,       // Fn + F8
	HotKey_Fn_F9,    	// Fn + F9
	HotKey_Fn_F10, 		// Fn + F10
	HotKey_Fn_F11, 		// Fn + F11
	HotKey_Fn_F12,    	// Fn + F12	
	HotKey_Fn_ESC,    	// Fn + ESC	
	HotKey_Fn_UP,    	// Fn + up arrow	
	HotKey_Fn_DOWN,    	// Fn + down arrow	
	HotKey_Fn_LEFT,    	// Fn + left arrow	
	HotKey_Fn_RIGHT,  	// Fn + right arrow	
	HotKey_Fn_SPACE,	// Fn + space
	//HotKey_Fn_Ins,	// Fn + Ins (Scr Lk)
	//HotKey_Fn_Del,	// Fn + Del (NumLk)
};

//---------------------------------------------------------------------------
//	Numpad 7 Home	[B0h]
//	Numpad 8 UP  	[B1h]
//	Numpad 9 PGUp	[B2h]
//	Numpad *   		[B3h]
//	Numpad 4 Left	[B4h]
//	Numpad 5 		[B5h]
//	Numpad 6 Right	[B6h]
//	Numpad -  		[B7h]
//	Numpad 1 End	[B8h]
// 	Numpad 2 Down	[B9h]
//	Numpad 3 PGDn	[BAh]
//	Numpad + 		[BBh]
//	Numpad 0 Ins	[BCh]
// 	Numpad . Del	[BDh]
//	Numpad /      	[BEh]
//---------------------------------------------------------------------------
#if !NUMPAD_USE_OEM
const BYTE code ss2_9C[] =	// Numpad 7 
{
    0x3D,   // Normal 		// 7
    0xB0,   // Numlock 
    0xB0,   // Fn 
    0xB0    // Fn+Numlock 
};

const BYTE code ss2_9D[] =	// Numpad 8   
{
    0x3E,   // Normal 		// 8
    0xB1,   // Numlock 
    0xB1,   // Fn 
    0xB1    // Fn+Numlock 
};

const BYTE code ss2_9E[] = 	// Numpad 9  
{
    0x46,   // Normal 		// 9
    0xB2,   // Numlock 
    0xB2,   // Fn 
    0xB2    // Fn+Numlock 
};

const BYTE code ss2_9F[] = 	// Numpad 0  
{
    0x45,   // Normal 		// 0
    0xBE,   // Numlock 
    0xBE,   // Fn 
    0xBE    // Fn+Numlock 
};

const BYTE code ss2_A0[] = 	// Numpad U  
{
    0x3C,   // Normal 		// U
    0xB4,   // Numlock 
    0xB4,   // Fn 
    0xB4    // Fn+Numlock 
};

const BYTE code ss2_A1[] = 	// Numpad I  
{
    0x43,   // Normal 		// I
    0xB5,   // Numlock 
    0xB5,   // Fn 
    0xB5    // Fn+Numlock 
};

const BYTE code ss2_A2[] =	// Numpad O 
{
    0x44,   // Normal 		// O
    0xB6,   // Numlock 
    0xB6,   // Fn 
    0xB6    // Fn+Numlock 
};

const BYTE code ss2_A3[] =	// Numpad P 
{
    0x4D,   // Normal 		// P
    0xB3,   // Numlock 
    0xB3,   // Fn 
    0xB3    // Fn+Numlock 
};

const BYTE code ss2_A4[] = 	// Numpad J 
{
    0x3B,   // Normal 		// J
    0xB8,   // Numlock 
    0xB8,   // Fn 
    0xB8    // Fn+Numlock 
};

const BYTE code ss2_A5[] = 	// Numpad K 
{
    0x42,   // Normal 		// K
    0xB9,   // Numlock 
    0xB9,   // Fn 
    0xB9    // Fn+Numlock 
};

const BYTE code ss2_A6[] =	// Numpad L 
{
    0x4B,   // Normal 		// L
    0xBA,   // Numlock 
    0xBA,   // Fn 
    0xBA    // Fn+Numlock 
};

const BYTE code ss2_A7[] = 	// Numpad ;:  
{
    0x4C,   // Normal 		// ;:
    0xB7,   // Numlock 
    0xB7,   // Fn 
    0xB7    // Fn+Numlock 
};

const BYTE code ss2_A8[] =	// Numpad M  
{
    0x3A,   // Normal 		// M
    0xBC,   // Numlock 
    0xBC,   // Fn 
    0xBC    // Fn+Numlock 
};

const BYTE code ss2_A9[] = 	// Numpad ,<  
{
    0x41,   // Normal 		// ,< 
    0x41,   // Numlock 
    0x41,   // Fn 
    0x41    // Fn+Numlock 
};

const BYTE code ss2_AA[] = 	// Numpad .>   
{
    0x49,   // Normal 		// .> 
    0xBD,   // Numlock 
    0xBD,   // Fn 
    0xBD    // Fn+Numlock 
};
#endif

const BYTE code ss2_AB[] = 	// Numpad /?      
{
    0x4A,   // Normal 		// /? 
    0xBB,   // Numlock 
    0xBB,   // Fn 
    0xBB    // Fn+Numlock 
};

const BYTE code ss2_AC[] =   
{
    0x5A,   // Normal 
    0x81,   // Fn 
};
//----------------------------------------------------------------
// Function key F1 ~ F12
//----------------------------------------------------------------
const BYTE code ss2_AD[] = 	// F1  //Mute
{
    0x05,   // Normal 
    0xD0,   // Fn 
};

const BYTE code ss2_AE[] = 	// F2  //Volume Down
{
    0x06,   // Normal 
    0xD1,   // Fn 
};

const BYTE code ss2_AF[] = 	// F3  //Volume Up
{
    0x04,   // Normal 
    0xD2,   // Fn 
};

const BYTE code ss2_B0[] =	// F4  //Disable Mic
{
    0x0C,   // Normal 
    0xD3,   // Fn 
};

const BYTE code ss2_B1[] = 	// F5  
{
    0x03,   // Normal 
    0x03,
};

const BYTE code ss2_B2[] = 	// F6  //Disable/Enable Touchpad
{
    0x0B,   // Normal 
    0xD5,   // Fn 
};

const BYTE code ss2_B3[] = 	// F7  //Airplane
{
    0x80,   // Normal 
    0xD6,   // Fn 
};

const BYTE code ss2_B4[] = 	// F8  //Disable Camera
{
    0x0A,   // Normal 
    0xD7,   // Fn 
};

const BYTE code ss2_B5[] = 	// F9  //Win + P
{
    0x01,   // Normal 
    0xD8,   // Fn 
    //0x01,   // Normal 
};

const BYTE code ss2_B6[] = 	// F10  //Switch display mode
{
    0x09,   // Normal 
	//0x09,   // Fn 
    0xD9,   // Fn 
    //0x09,   // Normal 
};

const BYTE code ss2_B7[] = 	// F11  //Brightness dn
{
    0x78,   // Normal 
    0xDA,   // Fn 
    //0x78,   // Normal 
};

const BYTE code ss2_B8[] = 	// F12  //Brightness up
{
    0x07,   // Normal 
    0xDB,   // Fn 
    //0x07,
};

const BYTE code ss2_B9[] =  // Fn+Space
{
    0x29,   // Normal
	0x29,   // Fn
};

/* ----------------------------------------------------------------------------
 * FUNCTION: sskey2_A2_table
 *
 * The A2_TABLE structure contains the following.
 *
 * BYTE comb;  Bit   Setting Combination Key
 *            ---   -----------------------
 *             7    Hook_calc_index_comb_BIT7
 *             6    Hook_calc_index_comb_BIT6
 *             5    Fn
 *             4    Numlock
 *             3    Ctrl
 *             2    Alt
 *             1    Shift
 *             0    reserved
 *
 * BYTE *pntr; Pointer to array for key.
 * ------------------------------------------------------------------------- */
const A2_TABLE sskey2_A2_table[] =
{											// Keyboard matrix index
	// Keypad -->
	{0x30, ss2_9C },    /* index = 00h */ 	// 0x9C	
    {0x30, ss2_9D },    /* index = 01h */	// 0x9D
    {0x30, ss2_9E },    /* index = 02h */	// 0x9E	
    {0x30, ss2_9F },    /* index = 03h */	// 0x9F	
    {0x30, ss2_A0 },    /* index = 04h */	// 0xA0
    {0x30, ss2_A1 },    /* index = 05h */	// 0xA1
    {0x30, ss2_A2 },    /* index = 06h */	// 0xA2
    {0x30, ss2_A3 },    /* index = 07h */	// 0xA3
    {0x30, ss2_A4 },    /* index = 08h */	// 0xA4
    {0x30, ss2_A5 },    /* index = 09h */	// 0xA5
    {0x30, ss2_A6 },    /* index = 0Ah */	// 0xA6
    {0x30, ss2_A7 },    /* index = 0Bh */	// 0xA7
    {0x30, ss2_A8 },    /* index = 0Ch */	// 0xA8
    {0x30, ss2_A9 },  	/* index = 0Dh */	// 0xA9
    {0x30, ss2_AA },  	/* index = 0Eh */	// 0xAA
    {0x30, ss2_AB },   	/* index = 0Fh */	// 0xAB
    {0x20, ss2_AC },  	/* index = 10h */	// 0xAC
	// Keypad <--
	
	// F1~F12 hotkeys -->
    {0x20, ss2_AD },    /* index = 11h */   // 0xAD=>F1
    {0x20, ss2_AE },    /* index = 12h */   // 0xAE=>F2
    {0x20, ss2_AF },    /* index = 13h */   // 0xAF=>F3
    {0x20, ss2_B0 },    /* index = 14h */   // 0xB0=>F4
    {0x20, ss2_B1 },    /* index = 15h */   // 0xB1=>F5
    {0x20, ss2_B2 },    /* index = 16h */   // 0xB2=>F6
    {0x20, ss2_B3 },    /* index = 17h */   // 0xB3=>F7
    {0x20, ss2_B4 },    /* index = 18h */   // 0xB4=>F8
    {0x20, ss2_B5 },    /* index = 19h */   // 0xB5=>F9
    {0x20, ss2_B6 },    /* index = 1Ah */   // 0xB6=>F10
    {0x20, ss2_B7 },    /* index = 1Bh */   // 0xB7=>F11
    {0x20, ss2_B8 },    /* index = 1Ch */   // 0xB8=>F12
    // F1~F12 hotkeys <--

	// Other hotkeys
	{0x80, ss2_B9 }     /* index = 1Dh */   // 0xB9=>Space
};
											// Reserved 0xB9 ~ 0xDF
 
/* ----------------------------------------------------------------------------
 * FUNCTION: sskey2_overlay_table
 * ------------------------------------------------------------------------- */
const BYTE code  sskey2_overlay_table[] =
{
//index 	            Normal	        Fn
  0x86,0x05,	// E0H	sleep			F1
  0xD8,0x06,	// E1H	lock      		F2	
  0xD9,0x04,	// E2H	display mode       F3	
  0xD5,0x0C,	// E3H	TP              	F4
  0x03,0x03,	// E4H					F5	
  0xDC,0x0B,	// E5H   BKLT				F6
  0xE1,0x80,	// E6H   KBLT				F7
  0xD0,0x0A,	// E7H	mute				F8	
  0xA2,0x01,	// E8H   vol down			F9
  0xA1,0x09,	// E9H   vol up			F10
  0xDA,0x78,	// EAH	Brightness Down	F11  
  0xDB,0x07,	// EBH	Brightness Up		F12
  0x00,0x76,	// ECH					ESC
  0x98,0x96,	// EDH	UpArrow			PageUp
  0x99,0x97,	// EEH	DnArrow			PageDown
  0x9A,0x94,	// EFH	LeftArr			Home
  0x9B,0x95,	// F0H	RightArr			End
  0xC3,0xC3,	// F1H	PrtSc
  0x2D,0x4F,	// F2H	R                 Fn+R
  0x94,0x00,	// F3h	Home		<-> pause
  0x95,0x00,	// F4h	End 		<-> break	
  0xC2,0xC2,	// F5h	Insert	<-> Null
  0xC0,0xC0,	// F6h	Delete	<-> Null
  0x96,0x94,	// F7h	PageUp	<-> Home
  0x97,0x95,	// F8h	PageDown	<-> end
  0x92,0x91,   // F9h	Pause	<-> Break
  0x29,0xE1,   // FAh   SPACE              Fn+Space
};
				// Reserved0xFA~ 0xFF		

const BYTE code  sskey2_overlay_table1[] =
{
//index 	           	 Fn	      		Normal
  0x05,0x86,  // E0H	 F1 	  			sleep 
  0x06,0xD8,  // E1H  F2	  			lock
  0x04,0xD9,  // E2H  F3	  			display mode  
  0x0C,0xD5,  // E3H	 F4				TP
  0x03,0x03,  // E4H	 F5 	
  0x0B,0xDC,  // E5H	 F6 		  		BKLT
  0x80,0xE1,  // E6H	 F7 		  		KBLT
  0x0A,0xD0,  // E7H	 F8 		  		mute	
  0x01,0xA2,  // E8H	 F9 		   		vol down 		
  0x09,0xA1,  // E9H	 F10		   		vol up
  0x78,0xDA,  // EAH	 F11		 		Brightness Down		 
  0x07,0xDB,  // EBH	 F12	  			Brightness Up  
  0x76,0x00,  // ECH	 ESC	   
  0x98,0x96,  // EDH  	UpArrow 		<->	  PageUp								   
  0x99,0x97,  // EEH  	DnArrow 		<->	  PageDown							   
  0x9A,0x94,  // EFH  	LeftArr 		<->	  Home										 
  0x9B,0x95,  // F0H  	RightArr	  	<->	  End	 
  0xC3,0xC3,  // F1H  	PrtSc 	  
  0x2D,0x4F,  // F2H	 R 		  		Fn+R
  0x94,0x00,  // F3h  	Home	  		<-> Null					   
  0x95,0x00,  // F4h  	End		  	<-> Null						   
  0xC2,0x00,  // F5h  	Insert  		<-> Null			   
  0xC0,0x00,  // F6h  	Delete  		<-> Null					
  0x96,0x94,  // F7h  	PageUp  		<-> Home					   
  0x97,0x95,  // F8h  	PageDown	  	<-> End				
  0x91,0x92,  // F9h  	Pause   		<-> Break 
  0x29,0xE1,  // FAh   	SPACE         Fn+Space
};
				


/* ----------------------------------------------------------------------------
 * Insyde Software Key Number 2
 *
 * 00h         Null
 * 01h - 7Fh   Same as IBM Scan Code, Set2
 * 80h - BFh   Pre-Index for generation Scan Code, Set2
 * C0h - DFh   Pre-Index for PPK function
 * C0h - DFh   Pre-Index for SMI function
 * ------------------------------------------------------------------------- */
/* ----------------------------------------------------------------------------
 * FUNCTION: sskey3_80_table - Insyde Software Key Number 2
 * 80h - BFh   Pre-Index for generation Scan Code, Set2
 * ------------------------------------------------------------------------- */
const BYTE code sskey3_80_table[] =
{
//-----------------------------------------------------------------
// Index from keyboard matrix
//-----------------------------------------------------------------	
									// Keyboard matrix index
	0x83, 	DO_SIMPLE_CODE,       	// [80h] F7 (118) 
	0x5A, 	DO_E0_PREFIX_CODE,    	// [81h] Numpad Enter (108) 
    0x1F, 	DO_E0_PREFIX_CODE,    	// [82h] Windows Left 
    0x27, 	DO_E0_PREFIX_CODE,    	// [83h] Windows Right 
    0x2F, 	DO_E0_PREFIX_CODE,    	// [84h] Application 
    0x37, 	DO_E0_PREFIX_CODE,    	// [85h] Power event 
    0x3F, 	DO_E0_PREFIX_CODE,    	// [86h] Sleep event 
    0x5E, 	DO_E0_PREFIX_CODE,		// [87h] Wake event 
    LShift, DO_CONTROL_EFFECT,		// [88h] 12 Shift(L) 
    RShift, DO_CONTROL_EFFECT,		// [89h] 59 Shift(R) 
    LAlt, 	DO_CONTROL_EFFECT,    	// [8Ah] 11 Alt(L) 
    RAlt, 	DO_CONTROL_EFFECT_E0,	// [8Bh] E0 11 Alt(R) 
    LCtrl, 	DO_CONTROL_EFFECT, 		// [8Ch] 14 Crtl(L) 
    RCtrl, 	DO_CONTROL_EFFECT_E0,	// [8Dh] E0 14 Ctrl(R) 
    FN, 	DO_CONTROL_EFFECT,    	// [8Eh] Fn 
    OVLAY,	DO_CONTROL_EFFECT,		// [8Fh] Overlay 
    0x00, 	DO_SIMPLE_CODE,       	// [90h] Reserved 
    0x00, 	DO_SPECIAL_CODE,      	// [91h] Pause (126) 
    0x01, 	DO_SPECIAL_CODE,      	// [92h] Break 
    0x84, 	DO_SIMPLE_CODE,       	// [93h] (124) Alt-Case (SysRq) 
	0x6C, 	DO_E0_PREFIX_CODE,    	// [94h] Home 
    0x69, 	DO_E0_PREFIX_CODE,    	// [95h] End 
    0x7D, 	DO_E0_PREFIX_CODE,  	// [96h] Page up 
    0x7A, 	DO_E0_PREFIX_CODE,    	// [97h] Page down 
	0x75, 	DO_E0_PREFIX_CODE,    	// [98h] UP arrow 
    0x72, 	DO_E0_PREFIX_CODE,    	// [99h] Down arrow 
    0x6B, 	DO_E0_PREFIX_CODE,  	// [9Ah] Left arrow 
    0x74, 	DO_E0_PREFIX_CODE,   	// [9Bh] Right arrow 
    
//-----------------------------------------------------------------
// For MicroSoft enhance keyboard feature.
//-----------------------------------------------------------------
 	0x4D, DO_E0_PREFIX_CODE,    	// [9Ch] Next Track event 
    0x15, DO_E0_PREFIX_CODE,    	// [9Dh] Prev Track event 
    0x3B, DO_E0_PREFIX_CODE,    	// [9Eh] Stop event 
    0x34, DO_E0_PREFIX_CODE,    	// [9Fh] Play/Pause event 
    0x23, DO_E0_PREFIX_CODE,    	// [A0h] Mute event 
    0x32, DO_E0_PREFIX_CODE,    	// [A1h] Volume Up event 
    0x21, DO_E0_PREFIX_CODE,    	// [A2h] Volume Down event 
    0x48, DO_E0_PREFIX_CODE,    	// [A3h] Mail event 
    0x10, DO_E0_PREFIX_CODE,    	// [A4h] Search event 
    0x3A, DO_E0_PREFIX_CODE,    	// [A5h] Web/Home event 
    0x38, DO_E0_PREFIX_CODE,    	// [A6h] Back event 
    0x30, DO_E0_PREFIX_CODE,    	// [A7h] Forward event 
    0x28, DO_E0_PREFIX_CODE,    	// [A8h] Stop event 
    0x20, DO_E0_PREFIX_CODE,    	// [A9h] Refresh event 
    0x18, DO_E0_PREFIX_CODE,    	// [AAh] Favorites event 
    0x2B, DO_E0_PREFIX_CODE,    	// [ABh] Caluator event 
    0x40, DO_E0_PREFIX_CODE,    	// [ACh] My Computer event 
    0x50, DO_E0_PREFIX_CODE,    	// [ADh] Media event 
    0x00, DO_SIMPLE_CODE,       	// [AEh] Reserved 
    0x00, DO_SIMPLE_CODE,      		// [AFh] Reserved 
    
//-----------------------------------------------------------------
// Index from sskey2_A2_table
//-----------------------------------------------------------------
	0x6C, DO_SIMPLE_CODE,     		// Numpad 7 Home	[B0h]
  	0x75, DO_SIMPLE_CODE,      		// Numpad 8 UP  	[B1h]
 	0x7D, DO_SIMPLE_CODE,       	// Numpad 9 PGUp	[B2h]
  	0x7C, DO_SIMPLE_CODE,       	// Numpad *   		[B3h]
	0x6B, DO_SIMPLE_CODE,       	// Numpad 4 Left	[B4h]
 	0x73, DO_SIMPLE_CODE,       	// Numpad 5 		[B5h]
	0x74, DO_SIMPLE_CODE,       	// Numpad 6 Right	[B6h]
	0x7B, DO_SIMPLE_CODE,       	// Numpad -  		[B7h]
	0x69, DO_SIMPLE_CODE,       	// Numpad 1 End		[B8h]
 	0x72, DO_SIMPLE_CODE,       	// Numpad 2 Down	[B9h]
	0x7A, DO_SIMPLE_CODE,       	// Numpad 3 PGDn	[BAh]
	0x79, DO_SIMPLE_CODE,       	// Numpad + 		[BBh]
	0x70, DO_SIMPLE_CODE,       	// Numpad 0 Ins		[BCh]
	0x71, DO_SIMPLE_CODE,      	 	// Numpad . Del		[BDh]
	0x4A, DO_E0_PREFIX_CODE, 		// Numpad /      	[BEh]
	
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[BFh]
	
    0x71, DO_E0_PREFIX_CODE,  		// Delete 			[C0h]  
    0x84, DO_SIMPLE_CODE,    		// SysRq			[C1h]  
    0x70, DO_E0_PREFIX_CODE,    	// Insert			[C2h]  
    0x7C, DO_E0_PREFIX_CODE,  		// Print Screen		[C3h] 
	0x41, DO_E0_PREFIX_CODE,     	// Euro Sign 		[C4h]
	0x49, DO_E0_PREFIX_CODE,    	// Dollar Sign 		[C5h]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[C6h]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[C7h]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[C8h]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[C9h]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[CAh]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[CBh]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[CCh]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[CDh]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[CEh]
	0x00, DO_SIMPLE_CODE,      		// Reserved 		[CFh]
	
// Function key F1 ~ F12     
    0	, DO_COSTOMER_FUNCTION,		// Function key F1	[D0h]  
    1	, DO_COSTOMER_FUNCTION,		// Function key F2	[D1h]  
    2	, DO_COSTOMER_FUNCTION,		// Function key F3	[D2h] 
    3	, DO_COSTOMER_FUNCTION,		// Function key F4	[D3h] 
    4	, DO_COSTOMER_FUNCTION,		// Function key F5	[D4h] 
    5	, DO_COSTOMER_FUNCTION,		// Function key F6	[D5h] 
    6	, DO_COSTOMER_FUNCTION,		// Function key F7	[D6h] 
    7	, DO_COSTOMER_FUNCTION,		// Function key F8	[D7h] 
    8	, DO_COSTOMER_FUNCTION,		// Function key F9	[D8h] 
    9	, DO_COSTOMER_FUNCTION,		// Function key F10	[D9h] 
    10	, DO_COSTOMER_FUNCTION,		// Function key F11	[DAh]  
    11	, DO_COSTOMER_FUNCTION,		// Function key F12	[DBh] 
	12	, DO_COSTOMER_FUNCTION,   	// Function key Esc	[DCh]
	13	, DO_COSTOMER_FUNCTION,   	// Function key Up arrow	[DDh]
	14	, DO_COSTOMER_FUNCTION,   	// Function key Down arrow	[DEh]
	15	, DO_COSTOMER_FUNCTION,  	// Function key Left arrow	[DFh]
	16	, DO_COSTOMER_FUNCTION,  	// Function key Right arrow	[E0h]	
	17  , DO_COSTOMER_FUNCTION,     // Function key Space [E1h]
};

									// Reserved 0xE0 ~ 0xFF		

//-----------------------------------------------------------------
// The function of checking boot hotkey Dino 1024    
//-----------------------------------------------------------------
#define Crisiskey01        0x8E   	// Fn
#define Crisiskey02        0x2D  	// R


//  CrisisKeyStatus 这个参数都是作为左值得，不知道有什么意义？？？ 保留意见
void CrisisHotkey01(BYTE event)
{
    if(event == BREAK_EVENT)
    {
        CrisisKeyStatus=0x00;
    }
    else if(event == MAKE_EVENT)
    {
        CrisisKeyStatus++;
    }
}

void CrisisHotkey02(BYTE event)
{
    if(event == BREAK_EVENT)
    {
        CrisisKeyStatus=0x00;
    }
    else if(event == MAKE_EVENT)
    {
        CrisisKeyStatus++;
    }
}


const sBootHotKeyStruct code asBootHotKeyStruct[]=
{
	{ Crisiskey01, CrisisHotkey01	},
	{ Crisiskey02, CrisisHotkey02	},
};


// matrix 为键值 ， event为 事件(make， break or repeat code)
void CheckBootHotKey(BYTE matrix,BYTE event)
{
	BYTE index;
	//一般笔记本 Fn 为功能按键 保留功能按键R
    for(index=0x00; index<(sizeof(asBootHotKeyStruct)/sizeof(sBootHotKeyStruct)); index++)
    {
		if(matrix==asBootHotKeyStruct[index].matrix)
	    {
	        (asBootHotKeyStruct[index].pfunction)(event);
            return;
        }
    }
    CrisisKeyStatus=0x00;
}

//-----------------------------------------------------------------------------
// Keyboard wake up system from S3
//-----------------------------------------------------------------------------
void CheckKBS3Wake(BYTE BKSI,BYTE BKSO)
{
	BYTE wuv;

    wuv = ((BKSI << 4)|BKSO);
    Tmp_code_pointer = &KB_S3WakeUP_Tables;
    if(*(Tmp_code_pointer + wuv)==0x55)
    {
        PulseSBPowerButton();
		Oem_TriggerS3S0();	//J80_008++
    }
}

void Et_CheckKBS3Wake(BYTE BKSI,BYTE BKSO)
{
	BYTE wuv;

    wuv = ((BKSI*3)+BKSO);
    Tmp_code_pointer = &EtKey_S3WakeUP_Tables;
    if(*(Tmp_code_pointer + wuv)==0x55)
    {
        PulseSBPowerButton();
		Oem_TriggerS3S0();	//J80_008++
    }
}

//----------------------------------------------------------------------------
// table_entry : value of keyboard matrix table. for example Rc_ROM_Tables[]
// event : key MAKE_EVENT or BREAK_EVENT or REPEAT_EVENT
//----------------------------------------------------------------------------
void OEM_Hook_Send_Key(BYTE table_entry, BYTE event)
{
    if(SystemNotS0)  // 系统不为S0状态
    {  	
        CheckBootHotKey(table_entry,event);
    }
}

//----------------------------------------------------------------------------
// Return : 0xFF --> Send_Key function will be break; (no any scan code to host)
//----------------------------------------------------------------------------
BYTE OEM_Hook_Skip_Send_Key(void)
{
    return(0x00);
}
								
//-----------------------------------------------------------------------------
// Keyboard hook function (KSO0 - KSO15)
// KSIValus : keyboard KSI
// KSOValus : keyboard KSO
//-----------------------------------------------------------------------------
void Hook_keyboard(BYTE KSIValus, BYTE KSOValus)
{
#if S3KeyboardWakeUp 
	if(SystemIsS3)
	{
		CheckKBS3Wake(KSIValus,KSOValus);
	}
#endif	
}		

//-----------------------------------------------------------------------------
// Keyboard hook function (KSO16 - KSO17 and GPIO KSO)
// KSIValus : keyboard KSI
// KSOValus : keyboard KSO
//-----------------------------------------------------------------------------
void Et_Hook_keyboard(BYTE KSIValus, BYTE KSOValus)
{
#if S3KeyboardWakeUp 
	if(SystemIsS3)
	{
		Et_CheckKBS3Wake(KSIValus,KSOValus);  // 做按键唤醒
	}
#endif	
}

/* ---------------------------------------------------------------------------
 * FUNCTION: Hook_Setup_Scanner_Pntr - to define the possible scanner tables
 *
 *     0   Rc_RAM_Table
 *     1   ROM Table 1
 *     ...
 * ------------------------------------------------------------------------ */
void Hook_Setup_Scanner_Pntr(void)
{                                        	
	Scanner_Table_Pntr = &Rc_ROM_Tables;
	Extendkey_Table_Pntr = &EtKey_ROM_Tables;
}

//-----------------------------------------------------------------------------
// The function of setting GPIO scan pin to high
//-----------------------------------------------------------------------------
void Hook_SetGPIOScanPinH(void)
{
	#if 0
	if(ExternMatrixGPO)
	{
		GPOKeyPin1H;
	}
	#endif
}

//-----------------------------------------------------------------------------
// The function of setting GPIO scan pin to low
//-----------------------------------------------------------------------------
void Hook_SetGPIOScanPinL(void)
{
	#if 0
	if(ExternMatrixGPO)
	{
		GPOKeyPin1L;
	}
	#endif
}

//-----------------------------------------------------------------------------
// The function of setting GPIO scan pin output
//-----------------------------------------------------------------------------
void Hook_SetGPIOScanPinCtrl(void)
{
	#if 0
	if(ExternMatrixGPO)
	{
		GPOKeyPin1Ctl = 0x40;		// output
	}
	#endif
}

//-----------------------------------------------------------------------------
// Fn key make
//-----------------------------------------------------------------------------
void Hook_Fn_Key_Make(void)
{

}

//-----------------------------------------------------------------------------
// Fn key break
//-----------------------------------------------------------------------------
void Hook_Fn_Key_Break(void)
{

}

//-----------------------------------------------------------------------------
// Return : 0xFF --> sskey2_A2_table contains bit6
//-----------------------------------------------------------------------------
BYTE Hook_calc_index_comb_BIT6(void)
{
    return(0x00);
}

//-----------------------------------------------------------------------------
// Return : 0xFF --> sskey2_A2_table contains bit7
//-----------------------------------------------------------------------------
BYTE Hook_calc_index_comb_BIT7(void)
{
    return(0x00);
}
