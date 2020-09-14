/*-----------------------------------------------------------------------------
 * TITLE: CORE_XLT.C - Scan Matrix handler at CORE level
 *
 * The procedure Send_Scan2 of this module is called to translate keyboard
 * scanner contact events into set 2 scan codes.
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
 * FUNCTION: Send_Scan2
 *
 * Input: key   = Row/Column number
 *                key.field.output = 0 - 15
 *                key.field.input  = 0 - 7
 *
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 *
 *        Scanner_State byte
 *
 *        Ext_Cb3_OVL_STATE
 *
 * On return, Scanner_State byte and Ext_Cb3_OVL_STATE are updated.
 *
 * Combination flag for each row/column
 *     0 = Insyde Software key number 2 in Row/Column Table
 *     1 = index for control_ptr_table in Row/Column Table
 *
 * Insyde Software Key Number 2
 *     01-7F = Same as Scan Code, Set 2
 *     80-FF = E0-prefix, cursor, printscreen, pause/break, special key
 * ------------------------------------------------------------------------- */
void Send_Scan2(union KEY key, BYTE event)
{
    // 似乎UtilityFlag中KBDebug 没有被设置过，所以应该一直为0
    // 所以应该是一直到 else 中执行
	if(IS_MASK_SET(UtilityFlag,KBDebug))
	{
		if(KeyScanACK==KU_ACK_Start)
		{
			if(event==MAKE_EVENT)
			{
				KeyScanKSI = key.field.input;
				KeyScanKSO = key.field.output;
				KeyScanACK = KU_ACK_Keyboard;
			}
		}
	}
	else // 应该是一直进到这里面
	{
        // key.field.input 表示的是高8位，但是实际上key.field.input 只占用3bit
        //刚好 RC_rom_tables 是128的数组，所以key.field.input 表示的是行信息
    	ITempB04 = (key.field.input << 4) | key.field.output;

        // 这里做的是基址 + 偏移量的事情， 其中Scanner_Table_Pntr 为键表的基址，这样就能唯一锁定键盘键值
        // 这里可能有键盘的映射关系
		ITempB03 = *(Scanner_Table_Pntr + ITempB04);    //matrix code To Scan Code Set 2

        // 如果系统处于睡眠模式，则按键唤醒
		Hook_keyboard(key.field.input, key.field.output); // wake up System from S3
		
        //ITempB03 表示的是RC_rom_tables的键值信息，event表示的是按键的事件
    	Send_Key(ITempB03, event);
	}
}


/* ----------------------------------------------------------------------------
 * FUNCTION: Send_Key
 * ------------------------------------------------------------------------- */
// table 表示RC_table数值， event 表示通码，断码或者repeat
void Send_Key(BYTE table_entry, BYTE event)
{
    BYTE temp;
    const BYTE code *pntr;
    BYTE temp_table_entry;

    // 这个函数里面会判断是不是S0状态，而后又热键判断，但是这个热键不知道有什么用
    OEM_Hook_Send_Key(table_entry,event);

    // 不会执行这个if，因为返回值为0x00
    if(OEM_Hook_Skip_Send_Key()==0xFF)
    {
        return;
    }

    // 键值赋值
    temp_table_entry = table_entry;

    // 标记缓冲区 以防止溢出
    Buffer_Mark();									// Mark Buffer in case of overrun.

    // 保存键盘的扫描状态
    temp_scanner_state.byte = Scanner_State;

    // 功能按键 fn键操作
    if (table_entry >= SSKEY2_OVL_CODE)				// Fn key + any key.
    {   
        /*
        temp = (table_entry - SSKEY2_OVL_CODE);
        temp = temp << 1;   						// Multiply 2 because each entry takes 2 bytes 
		if (temp_scanner_state.field.tFN)
		{   										// Fn key
            temp++; // Increment index to get the odd byte of table entry 
        }
        table_entry = sskey2_overlay_table[temp];	// Get a sskey2 value. 
        */
        
        temp = (table_entry - SSKEY2_OVL_CODE);
        temp = temp << 1;// Multiply 2 because each entry takes 2 bytes 

        #if KB_FnStickKey
        if(FnStickKey)
        {  
            temp++; // Increment index to get the odd byte of table entry 
        }
        else
        #endif
        {
            // 判断FN是否被按下
    		if (temp_scanner_state.field.tFN)
    		{// Fn key
                temp++; // Increment index to get the odd byte of table entry 
    			table_entry_bk =temp_table_entry;
            }
            else
            {
                if(event==BREAK_EVENT)
                {
                    // 与前一个按键键值相同
    				if(table_entry_bk==temp_table_entry)				
                    {
                        temp++;     // Increment index to get the odd byte of table entry 
                    }
                    table_entry_bk = 0x00;
                }
                else	//MAKE_EVENT
                {
                    if(table_entry_bk==temp_table_entry)
    				{
                        temp++;     // Increment index to get the odd byte of table entry 
                    }
                    else
                    {
                        table_entry_bk = 0x00;
                    }
                }
            }
        }
		//TF_012++>>
        // 这个bios好像没有被定义过
		if(IS_MASK_SET(OS_BIOS,F1_F12_Mode))
			table_entry = sskey2_overlay_table1[temp];	// Get a sskey2 value, normal F1-F12. 
		else
		//TF_012++<<
            // 这个操作实际上已经将键盘上的键码值转化为特殊功能的键码值转化为标准键码2的键码值
            // 返回的是标准键码2的键值
        	table_entry = sskey2_overlay_table[temp];	// Get a sskey2 value.      
    }

    // 复合按键 好像不会进到这里
    else if (table_entry >= SSKEY2_SPE_CODE)		// Combination Key
    {  
        temp = (table_entry - SSKEY2_SPE_CODE); 	// Get index into table. 
        table_entry = sskey2_A2_table[temp].comb;	// table_entry contains the combination table entry. 
        pntr = sskey2_A2_table[temp].pntr;			// pntr points to the first key byte. 
        //pntr += calc_index_comb(table_entry);		// Check combination.  Point to the correct byte. 
        pntr += calc_index_comb(table_entry,temp_table_entry,event);
        table_entry = *pntr;    				// Get a sskey2. 
    }

    // 这里传入的是键码2的键值
    sskey3_proc(table_entry, event);    		// Generate scan code, set 2.

	Scanner_State = temp_scanner_state.byte;	// Update scanner state. 

    #if KB_FnStickKey
    if(FnStickKey)
    {   
        if(event==BREAK_EVENT)
        {
            if(temp_table_entry!=0x8E)  // Fn key
            {
                FnStickKey=0;
            }
        }
    }

    if(event==BREAK_EVENT)
    {
        if(table_entry_bk==temp_table_entry)
        {
            table_entry_bk=0x00;
        }
    }
    #endif
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Clear_Fn_Keys
 * ------------------------------------------------------------------------- */
void Clear_Fn_Keys(void)
{
    //Fn_Make_Key1 = 0;
}

/* ----------------------------------------------------------------------------
 * FUNCTION: sskey3_proc
 *
 * Input: sskey2 = Insyde Software Key 2
 *        event =  Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 *
 * Note: FUNCT_PTR_V_BS is a pointer to a function that returns nothing (V for
 *       void) and takes a BYTE and a BYTE for parameters (B for BYTE, S for
 *       BYTE).
 * ------------------------------------------------------------------------- */
// Key Code Process Table.
const FUNCT_PTR_V_BS code kcp_vector_table[] =
{
    simple_code,        // index 00h DO_SIMPLE_CODE IBM key number 83,84 
    e0_prefix_code,     // index 01h DO_E0_PREFIX_CODE
    cursor_shift,       // index 02h DO_CURSOR_SHIFT
    cursor_numlock,     // index 03h DO_CURSOR_NUMLOCK
    special_code,       // index 04h DO_SPECIAL_CODE IBM key number 126 
    control_effect,     // index 05h DO_CONTROL_EFFECT Shift, Ctrl, Alt, Numlock 
    control_effect_e0,	// index 06h DO_CONTROL_EFFECT_E0 Ctrl, Alt 
    costomer_function	// index 07h DO_COSTOMER_FUNCTION costomer function key
};

static void sskey3_proc(BYTE sskey2, BYTE event)
{
    BYTE code_byte;
    BYTE index;
    FLAG quick_key = 0;

    if (sskey2 == 0)				// Null code 
    {   			
        ;					 
    }
    else if ((sskey2 & 0x80) == 0)
    {
     	simple_code(sskey2, event);	// 01h through 7Fh = scan code. 
    }
    else  							// 80h through FFh. 
    {   
        sskey2 &= 0x7F;
        sskey2 = sskey2 << 1;

        // 这里返回的 code_byte 为标准键值
        code_byte = sskey3_80_table[sskey2+0];

        // 返回的index 为 操作码
        index = sskey3_80_table[sskey2+1];
        (kcp_vector_table[index])(code_byte, event); // Do procedure 
    } 
}


/* ----------------------------------------------------------------------------
 * FUNCTION: calc_index_comb
 *
 * Input: combination setting
 *             Bit   Meaning
 *             ---   --------
 *             7     reserved
 *             6     reserved
 *             5     Fn
 *             4     Num Lock
 *             3     Ctrl
 *             2     Alt
 *             1     Shift
 *             0     reserved
 *
 *       temp_scanner_state
 *
 * Return: offset.
 * ------------------------------------------------------------------------- */
//static BYTE calc_index_comb(BYTE comb)
static BYTE calc_index_comb(BYTE comb, BYTE table_entry, BYTE event)
{
    BYTE offset;
    BYTE bit_num;

    offset  = 0;
    bit_num = 0;

    if (comb & BIT(1))
    {   									// Combination has Shift. 
    	if (temp_scanner_state.field.tLEFT | temp_scanner_state.field.tRIGHT)
		{   								// Either left or right shift is pressed. 
            offset |= Byte_Mask(bit_num);
        }
        bit_num++;  						// Increment bit position. 
    }

    if (comb & BIT(2))
    {   									// Combination has Alt. 
		if (temp_scanner_state.field.tALT)
        {   								// Alt is pressed. 
            offset |= Byte_Mask(bit_num);
        }
        bit_num++;  						// Increment bit position. 
    }

   	if (comb & BIT(3))
    {   									// Combination has Ctrl. 
		if (temp_scanner_state.field.tCONTROL)
        {  	 								// Ctrl is pressed. 
            offset |= Byte_Mask(bit_num);
        }
        bit_num++;  						// Increment bit position. 
    }

    if (comb & BIT(4))
    {   									// Combination has Num Lock. 
        if (temp_scanner_state.field.tNUM_LOCK)
        {   								// NumLock has been pressed. 
            offset |= Byte_Mask(bit_num);
        }
        bit_num++; 	 						// Increment bit position. 
    }

    if (comb & BIT(5))
    {   									// Combination has Fn. 
        //if (temp_scanner_state.field.tFN)
        //{   								// Fn is pressed. 
        //    offset |= Byte_Mask(bit_num);
        //}
        
        if (temp_scanner_state.field.tFN)
        {   								// Fn is pressed. 
            offset |= Byte_Mask(bit_num);
            table_entry_bk = table_entry;
        }
        else
        {
            if(event==BREAK_EVENT)
            {
                if(table_entry_bk==table_entry)
                {
                    offset |= Byte_Mask(bit_num);  
                }
                table_entry_bk = 0x00;
            }
            else
            {
                if(table_entry_bk==table_entry)
                {
                    offset |= Byte_Mask(bit_num);
                }
                else
                {
                    table_entry_bk = 0x00;
                }
            }
        }

        #if KB_FnStickKey
        if(FnStickKey)
        {   								// Fn is pressed. 
            offset |= Byte_Mask(bit_num);
        }
        #endif
        
        bit_num++;  						// Increment bit position. 
    }

    if (comb & BIT(6))
    {   									// Combination has Hook_calc_index_comb_BIT6 
        if(Hook_calc_index_comb_BIT6()==0xFF)
        {
            offset |= Byte_Mask(bit_num);
        }
        bit_num++;  						// Increment bit position. 
    }

    if (comb & BIT(7))
    {   									// Combination has Hook_calc_index_comb_BIT7 
        if(Hook_calc_index_comb_BIT7()==0xFF)
        {
            offset |= Byte_Mask(bit_num);
        }
        bit_num++;  						// Increment bit position. 
    }
    
    return(offset);
}

/* ----------------------------------------------------------------------------
 * FUNCTION: simple_code
 *
 * Input: scan_code, Set 2
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 * ------------------------------------------------------------------------- */
// 该函数表示将单个按键 放入buffer  单个按键不是当个字符的意思，因为在键码2 定义的断码是两个字符，第一个字符为0xF0
void simple_code(BYTE scan_code, BYTE event)
{
    // 如果是断码 进行发送数据
    if (event == BREAK_EVENT)
    {  
        Buffer_Key(0xF0);	// Buffer break prefix for break contact. F0h is break prefix. 
    }
    Buffer_Key(scan_code);  // Buffer base code. 
}

/* ----------------------------------------------------------------------------
 * FUNCTION: e0_prefix_code
 *
 * Input: scan_code, Set 2
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 * ------------------------------------------------------------------------- */

// 这个是特殊码
void e0_prefix_code(BYTE scan_code, BYTE event)
{
    Buffer_Key(0xE0);   	// Buffer E0h prefix. 
    if (event == BREAK_EVENT)
    {   					// Buffer break prefix for break contact. F0h is break prefix. 
        Buffer_Key(0xF0);
    }
    Buffer_Key(scan_code);  // Buffer base code. 
}

/* ----------------------------------------------------------------------------
 * FUNCTION: cursor_shift
 *
 * Input: scan_code, Set 2
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 *        temp_scanner_state
 * ------------------------------------------------------------------------- */
/* BREAK_EVENT key string for cursor_shift and cursor_numlock. */
	const BYTE code csr_brk1[] = {0xE0,0xF0,0x00};

// 关于键码2 中的 L SHFT = 12(make code)  L SHFT  = F0,12 (break code) 为什么要加E0
static void cursor_shift(BYTE scan_code, BYTE event)
{
    static const BYTE code csr_sftl_mk[]   = {0xE0,0xF0,0x12,0x00};
    static const BYTE code csr_sftr_mk[]   = {0xE0,0xF0,0x59,0x00};
    static const BYTE code csr_sftl_brk2[] = {0xE0,0x12,0x00};
    static const BYTE code csr_sftr_brk2[] = {0xE0,0x59,0x00};

    if (event == BREAK_EVENT)
    {   /* Key has just been released. This is a "break event". */
        Buffer_String(csr_brk1);    		/* Buffer pre-string. */
        Buffer_Key(scan_code);      		/* Buffer base code. */

        if (temp_scanner_state.field.tLEFT)
        {  									/* Left shift has been pressed. */
            Buffer_String(csr_sftl_brk2);   /* Buffer tail-string. */
        }

        if (temp_scanner_state.field.tRIGHT)
        { 									/* Right shift has been pressed. */
            Buffer_String(csr_sftr_brk2);   /* Buffer tail-string. */
        }
    }   
    else if (event == MAKE_EVENT)
    {   /* Key is pressed for the first time, a "make event". */
        									/* Left and/or Right SHIFT is pressed. */
        if (temp_scanner_state.field.tLEFT)
        {  									/* Left SHIFT is pressed. */
            Buffer_String(csr_sftl_mk); 	/* Buffer pre-string. */
        }

        if (temp_scanner_state.field.tRIGHT)
        { 									/* Right SHIFT is pressed. */
            Buffer_String(csr_sftr_mk); 	/* Buffer pre-string. */
        }

        Buffer_Key(0xE0);       			/* Buffer end of pre-string. */
        Buffer_Key(scan_code);  			/* Buffer base code. */
    }
    else
    {   /* Key is being held pressed for a length of time, a "repeat event". */
        Buffer_Key(0xE0);       			/* Buffer E0h prefix */
        Buffer_Key(scan_code);  			/* Buffer base code */
    } 
}

/* ----------------------------------------------------------------------------
 * FUNCTION: cursor_numlock
 *
 * Input: scan_code, Set 2
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 * ------------------------------------------------------------------------- */
// 这个又是什么
static void cursor_numlock(BYTE scan_code, BYTE event)
{
    static const BYTE code csr_numlock_mk[]   = {0xE0,0x12,0xE0,0x00};
    static const BYTE code csr_numlock_brk2[] = {0xE0,0xF0,0x12,0x00};

    if (event == BREAK_EVENT)
    { 
        Buffer_String(csr_brk1);            // Buffer pre-string. 
        Buffer_Key(scan_code);              // Buffer base code. 
        Buffer_String(csr_numlock_brk2);    // Buffer tail-string. 
    }
    else if (event == MAKE_EVENT)
    {  
        									// The Num Lock key is pressed. 
        Buffer_String(csr_numlock_mk);  	// Buffer pre-string. 
        Buffer_Key(scan_code);          	// Buffer base code. 
    }
    else
    {  
        Buffer_Key(0xE0);       			// Buffer E0h prefix. 
        Buffer_Key(scan_code);  			// Buffer base code. 
    } 
}


/* ----------------------------------------------------------------------------
 * FUNCTION: special_code
 *
 * For IBM Key Number 126. This key generates only make code.
 *
 * Input: code
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 * ------------------------------------------------------------------------- */
// 这个又是什么
void special_code(BYTE code_byte, BYTE event)
{
    static const BYTE code key126_normal_mk[] = {0xE1,0x14,0x77,0xE1,0xF0,0x14,0xF0,0x77,0x00};
    static const BYTE code key126_ctrl_mk[]   = {0xE0,0x7E,0xE0,0xF0,0x7E,0x00};

    if (event == MAKE_EVENT)
    {
        if (code_byte == 0) 
		{   
           	Buffer_String(key126_normal_mk);// Buffer normal code string.
        }
        else 
		{  
            Buffer_String(key126_ctrl_mk);	// Buffer Ctrl case string.
        }
    }
}

/* ----------------------------------------------------------------------------
 * FUNCTION: costomer_function
 *
 * Input: code
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 * ------------------------------------------------------------------------- */

//----------------------------------------------------------------------------
// The function of hotkey Fn + Fx
//----------------------------------------------------------------------------
// 该函数执行的是fn + fx(x:1~12) 的功能按键
static void costomer_function(BYTE code_byte, BYTE event)
{
	(HotKey_Fn_Fx[code_byte])(event);

    // 如果buffer 不为空  使能发送数据
	if (scan.kbf_head != scan.kbf_tail)
	{
        SetServiceSendFlag();
	}
}

/* ----------------------------------------------------------------------------
 * FUNCTION: control_effect
 *
 * Generate scan code set 2 and update scanner status.
 *
 * Input: state flags
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 *        temp_scanner_state
 *
 *  The following bits are defined for state flags and temp_scanner_state:
 *       NUM_LOCK
 *       LEFT
 *       RIGHT
 *       ALT
 *       CONTROL
 *       OVERLAY
 *       FN
 *       
 *
 * Returns: Updated temp_scanner_state
 * ------------------------------------------------------------------------- */
// 控制效果按键 
static void control_effect(BYTE state, BYTE event)
{
    // 判断按下或者弹起来的 ctrl 和 shift 保存键码
    if ((event == MAKE_EVENT)||(event == BREAK_EVENT))
    {
    	if (state & LShift)				// First generate scan code set 2.
		{
        	simple_code(0x12, event);
    	}
 		else if (state & LAlt)
		{
        	simple_code(0x11, event);
    	}
		else if (state & LCtrl) 
		{
        	simple_code(0x14, event);
    	}
    	else if (state & RShift) 
		{
        	simple_code(0x59, event);
    	}
		//else if (state & FN) 
		//{
        //	e0_prefix_code(0x63, event);
    	//}
    }
   

    // 下面函数在更新扫描状态
    if (event == MAKE_EVENT)		 	// Then update scanner state.
    {   // Key is pressed for the first time. This is a "make event".
		// Set scanner state for SHIFT, Alt, Ctrl, or Fn.
    	temp_scanner_state.byte |= state;
        if (state & FN) 
        {
            //这个函数是空的
            Hook_Fn_Key_Make();
            #if KB_FnStickKey
            FnStickKey = 1;
            #endif

			SET_MASK(Fn_S_V_Flag,_FN_BIT);   //Label:BLD_TIPIA_20161118_020
        }
    }
    else if (event == BREAK_EVENT)
    {   
        if (state & FN) 
        {
        	CLEAR_MASK(Fn_S_V_Flag,_FN_BIT);  //Label:BLD_TIPIA_20161118_020
            Hook_Fn_Key_Break();
        }
     	temp_scanner_state.byte &= ~state;
    }
    else
    { 
        ;   // Do nothing for repeat event. 
    }
}


/* ----------------------------------------------------------------------------
 * FUNCTION: control_effect_e0
 *
 * Generate scan code set 2 and update scanner status.
 * Only for Alt-Right and Ctrl-Right.
 *
 * Input: state flags
 *        event = Contact event (MAKE_EVENT, BREAK_EVENT, or REPEAT_EVENT)
 *        temp_scanner_state
 *
 * The following bits are defined for state_flags and temp_scanner_state:
 *       NUM_LOCK
 *       LEFT
 *       RIGHT
 *       ALT
 *       CONTROL
 *       OVERLAY
 *       FN
 *
 * Return: Updated temp_scanner_state
 * ------------------------------------------------------------------------- */
static void control_effect_e0(BYTE state, BYTE event)
{   // This is only called for Alt-Right and Ctrl-Right. 
    if ((event == MAKE_EVENT)||(event == BREAK_EVENT))
    {
    	// First setup to generate scan code set 2. 
    	if (state & RAlt) 
		{
			e0_prefix_code(0x11, event);
    	}
		if (state & RCtrl) 
		{
			e0_prefix_code(0x14, event);
    	}
	}

	if (event == MAKE_EVENT)
 	{   	// Key is pressed for the first time. This is a "make event". 
            // Set one flag in scanner state flags. 
     	temp_scanner_state.byte |= state;
  	}
	else if (event == BREAK_EVENT)
	{   	// Key has just been released. This is a "break event". 
            // Clear one flag in scanner state flags. 
      	temp_scanner_state.byte &= ~state;
  	}
 	else
  	{   
       	;   // Do nothing for repeat event. 
 	} 		
}




