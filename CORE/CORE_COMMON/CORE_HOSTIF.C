/*-----------------------------------------------------------------------------
 * TITLE: CORE_HOSTIF.C - Host Interface Handler
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
 * FUNCTION: Data_To_Host
 *
 * Clear error bits in the Host Interface status port and sends a command
 * response byte or a byte of keyboard data to the Host.  Generate Host IRQ1
 * if keyboard interrupts are enabled in controller command byte.
 *
 * Input:  data to send to Host.
 * ------------------------------------------------------------------------- */

// 这个是KBC部分 主机域部分
void Data_To_Host(BYTE data_byte)
{
    //SET_MASK(KBHISR,KEYL);	
    //CLEAR_MASK(KBHISR,AOBF);
    KBHISR &= 0x0F;         // 初始化端口状态，相当于清空状态标志位
	SET_MASK(KBHISR,KEYL);  // 设置键盘没有被锁定
    
    // bit4: PM Channel 1 Host Interface Interrupt Enable 
    // bit3: 输入buffer 满中断使能
    // bit2: 输出buffer 空中断使能
    // bit1: Output Buffer Full Mouse Interrupt Enable
    // bit0: Output Buffer Full Keyboard Interrupt Enable
    // 关于 KBHICR 寄存器，默认bit3 与bit6 设置为1，其余设置为0
    // 在这里只是保证配置低2位为0
    KBHICR &= 0xFC;       

    // 判断键盘中断，这个实际上是读取8042状态寄存器，判断他的中断标志
    if ( Ccb42_INTR_KEY )
    {
        // OBFKIE 为bit0 这里其实为改变输出buffer满中断的中断方式
		SET_MASK(KBHICR,OBFKIE);
    }

    // 将待发送的数据放入buffer, 并置位OBF
    KBHIKDOR = data_byte;

    // 等待两个时钟时间
	ShortDelayAfterWriteDOR();
}


//
void Data_To_Host_nWait(BYTE data_byte)
{
    // 数据发送函数
    Data_To_Host(data_byte);

    //下面函数设置26ms超时机制  
    TR1 = 0;                    // Disable timer1
    ET1 = 0;                  	// Disable timer1 interrupt
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    TH1 = Timer_26ms>>8;        // Set timer1 counter 26ms
    TL1 = Timer_26ms;           // Set timer1 counter 26ms
    TF1 = 0;                  	// Clear overflow flag
    TR1 = 1;                 	// Enable timer1

	while (!TF1)
	{ 
        // 当8032将数据写入KBHIKDOR和KBHIMDOR时，该位置1。  相反，当主机读取KBDOR时，该位将被清除
        // 所以这里是判断主机是否读取了存入的数据
	    if(IS_MASK_CLEAR(KBHISR, OBF))
        {
            break;
        }  
        //当主机处理器将数据写入KBDIR或KBCMDR时，该位置1。 相反，当8032固件读取KBHIDIR时，该位将被清除。 
        if(IS_MASK_SET(KBHISR, IBF))
        {
            break;
        }  
	}
    
    TR1 = 0;			// disable timer1
 	TF1 = 0;			// clear overflow flag
	ET1 = 1;			// Enable timer1 interrupt
}

//-----------------------------------------------------------------------------
// KBC 数据挂起
//-----------------------------------------------------------------------------
void KBC_DataPending(BYTE nPending)
{
    if( KBPendingRXCount > 3 )  return;

    // 4个字节的缓冲区  KBPendingRXCount&0x03 防止越界
    KBDataPending[(KBPendingRXCount&0x03)] = nPending;
    KBPendingRXCount++;
    SetServiceSendFlag();  // 2ms定时器重装载
}

//-----------------------------------------------------------------------------
// 获取挂起数据，返回挂起的数据
//-----------------------------------------------------------------------------
BYTE GetKB_PendingData(void)
{
    BYTE buffer_data;
    buffer_data=KBDataPending[(KBPendingTXCount&0x03)];
    KBPendingTXCount++;

    // 如果超出buffer 则清空flag标志位
    if( KBPendingTXCount >= KBPendingRXCount )
    {
        KBPendingTXCount = 0;
        KBPendingRXCount = 0;

        // 如果环形缓冲区数据空，则 禁止发送KBC 在中断函数中
		if (scan.kbf_head == scan.kbf_tail)
    	{
    		Timer_B.fbit.SEND_ENABLE = 0;
		}
    }
    return buffer_data;
}

//-----------------------------------------------------------------------------
// 发送数据给主机域
//-----------------------------------------------------------------------------
void KBC_DataToHost(BYTE nKBData)
{
    // 当有数据被写入时，将数据放入缓冲区 KBDIR or KBCMDR（主机接口） / KBHIKDOR and KBHIMDOR （EC接口）
    if(IS_MASK_SET(KBHISR,OBF)||IS_MASK_SET(KBHISR,IBF))
	//if(IS_MASK_SET(KBHISR,OBF))
	{
		KBC_DataPending(nKBData);
	}
	else
	{
		Data_To_Host(nKBData );	// Send data to host.
	}
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Aux_Data_To_Host - Send auxiliary device (mouse) data to the Host.
 *
 * Clear error bits in the Host Interface status port and sends a byte of
 * aux device (mouse) data to the Host.  Generates Host IRQ12 if aux device
 * (mouse) interrupts are enabled in controller command byte.
 *
 * Input: data to send to Host.
 * ------------------------------------------------------------------------- */
// 发送AUX数据给主机
void Aux_Data_To_Host(BYTE data_byte)
{
    KBHISR &= 0x0F;
	SET_MASK(KBHISR,AOBF);

	//if (Ccb42_SYS_FLAG)		// Put system flag bit in Status Reg.
  	//	SET_MASK(KBHISR,SYSF);
  	//else
	//	CLEAR_MASK(KBHISR,SYSF);

	KBHICR &= 0xFC;

    // 与键盘类似操作，这里是判断鼠标中断
    if ( Ccb42_INTR_AUX )
    {
       	SET_MASK(KBHICR,OBFMIE);
    }

    // 存放数据给寄存器
 	KBHIMDOR = data_byte;
	ShortDelayAfterWriteDOR();
}

//----------------------------------------------------------------------------
// 开启数据发送服务， Timer_B.fbit.SEND_ENABLE 这个参数在定时器中断1
//----------------------------------------------------------------------------
void SetServiceSendFlag(void)
{
    Load_Timer_B();  // 清除定时器1标志位，并使能定时器1，重装载 2ms
 	Timer_B.fbit.SEND_ENABLE = 1;
}

/* ----------------------------------------------------------------------------
 * FUNCTION: service_send
 *
 * Send data from the scanner keyboard or from multibyte command responses to
 * the Host.
 *
 * Send scan codes from scanner keyboard to the Host.  Also handle multiple
 * byte transmissions for standard commands and extended commands that return
 * more than one byte to the Host.
 * 
 * 将扫描码从键盘发送到主机，还可以处理向主机返回一个以上的标准指令和扩张命令的多字节传输
 *
 * When sending multiple bytes, the 1st byte is sent immediately, but the
 * remaining bytes are sent by generating another send request via the
 * function "handle_unlock" which will call "Start_Scan_Transmission".
 * If more bytes are to be sent, "Start_Scan_Transmission" will start Timer A,
 * and the Timer A interrupt handler will generate the send request when the
 * response timer has expired!
 *     当发送多个字节时，第一个字节立即发送，但是剩余的字节通过“ handle_unlock”函数生成另一
 * 个发送请求来发送，该函数将调用“ Start_Scan_Transmission”。 如果要发送更多字节，则“ 
 * Start_Scan_Transmission”将启动定时器A，并且当响应定时器到期时，定时器A中断处理程序将生
 * 成发送请求！
 * 
 * ------------------------------------------------------------------------- */
void service_send(void)
{
    BYTE temp_flag, send;
    BYTE data_word;
    send = FALSE;

 	//Load_Timer_B();
 	//Timer_B.fbit.SEND_ENABLE = 1;
 	SetServiceSendFlag();  // 是能定时器1 并且使能定时器1中的发送数据使能
	
    // 判断buffer 是否被CPU读取
    if( IS_MASK_SET(KBHISR,OBF) || IS_MASK_SET(KBHISR,IBF) )
    //if(IS_MASK_SET(KBHISR,OBF))
    {
        return;
    }

    // 当有挂起数据发送时，执行发送指令
    if ( KBPendingRXCount > 0 )
    {
        Data_To_Host(GetKB_PendingData());
        return;
    }

    // 禁能键盘或者当主机处理器将数据写入KBDIR或KBCMDR时
	if((Ccb42_DISAB_KEY == 1)|| IS_MASK_SET(KBHISR,IBF))
	{
		return;
	}
  
    // 从环形缓冲区中获取键盘键值
    data_word = Get_Buffer();

    // buffer 为空就禁止发送中断， 否则开启发送使能
    if (data_word == 0xFF) 
    {   
	    Timer_B.fbit.SEND_ENABLE = 0;
    }
    else
    {   
        send = TRUE;  // send 为临时变量
    }

    //  如果bufer 不为空, 表示有数据发送
	if (send)							// Send it 
    {
   		temp_flag = Gen_Info_BREAK_SCAN;
        Gen_Info_BREAK_SCAN = 0;       // 清空flag
        
        //  如果Ccb42_XLATE_PC 为1,表示需要转化键码2为键码1
        //  如果 send_to_pc 函数的返回值为1 表示要发送断码标识？？
        if (send_to_pc(data_word, temp_flag)) 
		{
            Gen_Info_BREAK_SCAN = 1;    // Break prefix code. 
        }
    } 
}

/* ----------------------------------------------------------------------------
 * FUNCTION: get_response
 *
 * Gets data required for scanner keyboard responses to keyboard commands sent
 * from Host. 获取扫描仪键盘对主机发送的键盘命令的响应所需的数据。
 *
 * Input: Kbd_Response has response code.
 *        This is called when Kbd_Response_CMD_RESPONSE == 0.
 *
 * Return: data to send.
 * ------------------------------------------------------------------------- */
/* This table has commands that the keyboard may send to the Host in response
   to transmissions, etc. */
const BYTE code response_table[] =  // 16字节数组
{
    0x00,   /* 0 Undefined. */
    0xFA,   /* 1 Manufacurer ID. */
    0xAB,   /* 2 Byte 1 of keyboard ID. */
    0xAA,   /* 3 BAT completion. */
    0xFC,   /* 4 BAT failure. */
    0xEE,   /* 5 Echo. */
    0xFA,   /* 6 Acknowledge. */
    0xFE,   /* 7 Resend. */
    0xFA,   /* 8 Return scan code 2. */
    0x83,   /* 9 Byte 2 of U.S. keyboard ID. */
    0xFA,   /* A reset ack. */
    0xEE,   /* B Echo command. */
    0xAA,   /* C First response. */
    0xFA,   /* D Return scan code 2. */
    0x02,   /* E. */
    0x84    /* F Byte 2 of Japanese keyboard ID. */
};


extern BYTE get_response(void)
{
    BYTE result, code_word;
    code_word = 0;

    switch (Kbd_Response & maskKBD_RESPONSE_CODE) // Kbd_Response & 0x03f
    {
        case 2:
            // 美式键盘或者日式键盘
            if (Get_Kbd_Type() == 0)  // 这里返回的永远是美式键盘
			{
                code_word = 9;   // U.S. keyboard.   <<<<<<<<< 永远是这个
            }
            else 
			{
                code_word = 0xF; // Japanese keyboard.
            }
            break;
    }

    // 从列表中获取返回值
    result = response_table[Kbd_Response & maskKBD_RESPONSE_CODE];

    // Kbd_Response & ~maskKBD_RESPONSE_CODE 相当于保留了最高位
    // 或上键盘的
    Kbd_Response = (Kbd_Response & ~maskKBD_RESPONSE_CODE) | code_word;

    return(result);
}


/* ----------------------------------------------------------------------------
 * FUNCTION: get_multibyte
 *
 * Get multiple bytes in response to a command that requires multiple bytes to
 * be returned to Host.  These commands include the "standard" "AC" command
 * and the extended command "85".  Only 1 byte at a time is returned from this
 * function, it keeps track of the last byte it sent by maintaining a pointer
 * to the "data packet" that contains the multiple bytes it is supposed to
 * retrieve.  The next time it's called it will return the next byte in the
 * packet until all bytes are sent.
 *
 * Input: Kbd_Response has multibyte response code.
 *        This is called when Kbd_Response_CMD_RESPONSE == 1.
 *
 * Return: data to send.
 * ------------------------------------------------------------------------- */
#if 0
static BYTE get_multibyte(void)
{
    //WORD data_word;
    BYTE data_word;

    switch (Kbd_Response & maskKBD_RESPONSE_CODE)
    {
        case (respCMD_AC & maskKBD_RESPONSE_CODE):
        /* Process command "AC".  Bit[7:1] of Tmp_Byte[0] is the address offset
           of data "packet" (initially 0). */
            //data_word = Version[Tmp_Byte[0]>>1];
            Tmp_Byte[0]++;
            if (data_word == 0xAA) /* Keep going if not end of table. */
            {   /* Otherwise, stop by clearing. */
                Kbd_Response = Kbd_Response & ~maskKBD_RESPONSE_CODE;
                Kbd_Response_CMD_RESPONSE = 0;
            }
            break;

        case (respCMD_CFG & maskKBD_RESPONSE_CODE):
        /* Process Read Configuration Table command.  Config_Table_Offset has
           address offset of the first byte to send.  Config_Data_Length has
           the number of bytes to send.  Tmp_Load holds the number of bytes
           sent and is also used as the index. */
           // Dino 20070517
			//data_word = Read_Config_Table(Config_Table_Offset + (WORD) Tmp_Load);
            Tmp_Load++; /* Bump number of bytes sent. */
            if (Tmp_Load == Config_Data_Length) /* Keep going if not finished. */
            {   /* Otherwise, stop by clearing. */
                Kbd_Response = Kbd_Response & ~maskKBD_RESPONSE_CODE;
                Kbd_Response_CMD_RESPONSE = 0;
            }
            break;

        case (respARRAY & maskKBD_RESPONSE_CODE):
        /* Send data from an array.
           Tmp_Pntr has address of byte to send.
           Tmp_Load has number of bytes to send. */
            data_word = *Tmp_Pntr++;
            Tmp_Load--;
            if (Tmp_Load == 0)   /* Keep going if not end of array. */
            {   /* Otherwise, stop by clearing. */
                Kbd_Response = Kbd_Response & ~maskKBD_RESPONSE_CODE;
                Kbd_Response_CMD_RESPONSE = 0;
            }
            break;

        default:  /* Look for an OEM response code if not CORE. */
           //data_word = Gen_Hookc_Get_Multibyte();
           data_word = 0x00;
           break;
    } /* switch (Kbd_Response & maskKBD_RESPONSE_CODE) */

    return(data_word);
}
#endif

/* ----------------------------------------------------------------------------
 * FUNCTION: send_ext_to_pc, send_to_pc
 *
 * send_ext_to_pc is just a 'pre-entry' to the original send_to_pc
 * routine.  It's sole purpose is to check auxiliary keyboard entries
 * for external hotkey functions.  This allows it to set flags for
 * CTRL and ALT states, then if BOTH are set, hotkeys can be checked.
 *
 * Send data to Host.  If password is enabled, the data
 * will not be sent until the password is entered correctly.
 *
 * Input: data - Data to send to PC.
 *        break_prefix_flag - TRUE if last scan code was a break prefix.
 *
 * Returns: TRUE if translation mode is enabled and
 *          scan code was a break prefix.
 * ----------------------------------------------------------------------------*/

/**
 *功能：send_ext_to_pc，send_to_pc
 * send_ext_to_pc只是原始send_to_pc例程的“预输入”。 唯一的目的是检查辅助键盘的外部热键功能。
 * 这允许它设置CTRL和ALT状态的标志，然后如果两个都设置，则可以检查热键。
 *
 *将数据发送到主机。 如果启用了密码，则只有正确输入密码后才能发送数据。
 *
 *输入：数据-要发送到PC的数据。
 *
 *break_prefix_flag-如果上次扫描代码是中断前缀，则为TRUE。
 *
 *返回：如果启用了翻译模式并且扫描代码是一个断码前缀，则为TRUE。
 */

static BYTE send_to_pc(BYTE data_word, BYTE break_prefix_flag)
{
    return(common_send_to_pc(data_word, break_prefix_flag));
}

static BYTE common_send_to_pc(BYTE data_word, BYTE break_prefix_flag)
{
   	BYTE	send_it = FALSE;

    // 判断需不需要将扫描码2 转化为扫描码1
	if (Ccb42_XLATE_PC == 0)			 // Send data as is.   按原样发送数据
   	{
      	send_it = TRUE;
      	break_prefix_flag = FALSE;       // 不需要断码前缀
   	}
   	else								// Translation mode is enabled. 
   	{ 
        // 数据转化,将键码2转化为扫描码1
    	data_word = translate_to_pc(data_word, break_prefix_flag);
      	if (data_word == 0xFF)
      	{
         	break_prefix_flag = TRUE;	// Don't send break code prefix. 
      	}
	  	else if(data_word == 0x00)
		{
         	break_prefix_flag = TRUE;;	// Don't send break code prefix. 
      	}
      	else
      	{
         	break_prefix_flag = FALSE;  
		 	send_it = TRUE;
      	}
   	}

    // 将数据放入  KBHIKDOR 中，这个接口是EC阈接口
	if (send_it) {Data_To_Host(data_word);}

   	return(break_prefix_flag);
}

/* ----------------------------------------------------------------------------
 * FUNCTION: translate_to_pc
 *
 * Translate scan code from "set 2 scan code" to "set 1 scan code".
 *
 * Input: data - scan code received from aux keyboard or local keyboard.
 *        break_prefix_flag - TRUE if last scan code was a break prefix.
 *
 * Return: a value with all bits set if data is break prefix (0xF0);
 *         otherwise, returns translated key.
 * ------------------------------------------------------------------------- */
/*----------------------------------------------------------------------------
  *功能：translate_to_pc
  *
  *将扫描代码从“设置2扫描代码”转换为“设置1扫描代码”。
  *
  *输入：数据-从辅助键盘或本地键盘接收的扫描代码。
  * break_prefix_flag-如果最后一个扫描代码是一个中断前缀，则为TRUE。
  *
  *返回：如果数据是中断前缀（0xF0），则将所有位都设置为1的值；
  *否则，返回翻译后的密钥。
  *-------------------------------------------------------------------------*/
static BYTE translate_to_pc(BYTE data_word, BYTE break_prefix_flag)
{
    /* Scan code set 2 to scan code set 1 translation table.  First byte is a
       dummy entry because scan code "0" is not translated. */
    //扫描代码集2扫描代码集1转换表。 第一个字节是一个虚拟条目，因为未翻译扫描代码“ 0”。

    // 通码映射表
    static const BYTE code scan2_table[] =
    {  // 16*8
        0x00, 0x43, 0x41, 0x3F, 0x3D, 0x3B, 0x3C, 0x58,
        0x64, 0x44, 0x42, 0x40, 0x3E, 0x0F, 0x29, 0x59,
        0x65, 0x38, 0x2A, 0x70, 0x1D, 0x10, 0x02, 0x5A,
        0x66, 0x71, 0x2C, 0x1F, 0x1E, 0x11, 0x03, 0x5B,
        0x67, 0x2E, 0x2D, 0x20, 0x12, 0x05, 0x04, 0x5C,
        0x68, 0x39, 0x2F, 0x21, 0x14, 0x13, 0x06, 0x5D,
        0x69, 0x31, 0x30, 0x23, 0x22, 0x15, 0x07, 0x5E,
        0x6A, 0x72, 0x32, 0x24, 0x16, 0x08, 0x09, 0x5F,
        0x6B, 0x33, 0x25, 0x17, 0x18, 0x0B, 0x0A, 0x60,
        0x6C, 0x34, 0x35, 0x26, 0x27, 0x19, 0x0C, 0x61,
        0x6D, 0x73, 0x28, 0x74, 0x1A, 0x0D, 0x62, 0x6E,
        0x3A, 0x36, 0x1C, 0x1B, 0x75, 0x2B, 0x63, 0x76,
        0x55, 0x56, 0x77, 0x78, 0x79, 0x7A, 0x0E, 0x7B,
        0x7C, 0x4F, 0x7D, 0x4B, 0x47, 0x7E, 0x7F, 0x6F,
        0x52, 0x53, 0x50, 0x4C, 0x4D, 0x48, 0x01, 0x45,
        0x57, 0x4E, 0x51, 0x4A, 0x37, 0x49, 0x46, 0x54
    };

    BYTE check_break_bit = FALSE;

    // 遇到断代码前缀
    if (data_word == 0xF0)
    {   /* Signify that break code prefix was encountered. */
		data_word = 0xFF;
    }
    else if (data_word == 0x00) // 按键检测错误/超限
    {
        data_word = 0x00;      		/* Key detection error/overrun. */
    }
    else if ((data_word & 0x80) == 0)  // 
    {   /* Translate codes 01 thru 7F. */ // 将代码01转换为7F。
        /* The variable "data" has scan code (set 2) to translate.
           Set "data" to the translated (to set 1) scan code. */
        data_word = scan2_table[data_word];
        check_break_bit = TRUE;
    }
    else if (data_word == 0x83)  	/* ID code for 101/102 keys. */ 
    {
        data_word = 0x41;        	/* Translate ID code. */
        check_break_bit = TRUE;
    }
    else if (data_word == 0x84)     /* ID code for 84 keys. */
    {
        data_word = 0x54;         	/* Translate ID code. */
        check_break_bit = TRUE;
    }

    // check_break_bit 为转化成功标志 
    // break_prefix_flag 位断码前缀码标志
    // 实际上这个就是键码1 中的断码标记  好像键码1中的84键只有1个断码字符
    if (check_break_bit && break_prefix_flag)
    {   /* Last code received by this routine was the break prefix.  This must
           be a break code.  Set high bit to indicate that this is a break code. */
        data_word |= 0x80;
    }

    return(data_word);
}






// 下面这个在代码中没有被使用到
//----------------------------------------------------------------------------
// Kernel Init super IO function
//----------------------------------------------------------------------------
const BYTE code Core_initsio_table[]=
{
				// Configure and Enable Logical Device 06h(KBD)
	0x07 ,0x06,	// Select Logical Device 06h(KBD)
  	0x70 ,0x01,	// Set IRQ=01h for Logical Device 06h(KBD)
  	0x30 ,0x01,	// Enable Logical Device 06h(Mouse)
				// Configure and Enable Logical Device 05h(Mouse)
 	0x07 ,0x05,	// Select Logical Device 05h(Mouse)
  	0x70 ,0x0C,	// Set IRQ=0Ch for Logical Device 05h(Mouse)
	0x30 ,0x01,	// Enable Logical Device 05h(Mouse)
				// Configure and Enable Logical Device 11h(PM1)
   	0x07 ,0x11,	// Select Logical Device 11h(PM1)
  	0x70 ,0x00,	// Clear IRQ=0 for  Logical Device 11h(PM1)
 	0x30 ,0x01,	// Enable Logical Device 11h(PM1)
				// Configure and Enable Logical Device 0Fh(Shared Memory)
 	0x07 ,0x0F,	// Logical Device 0Fh(Shared Memory)
	0x30 ,0x01,	// Enable Logical Device 0Fh(Shared Memory)
};

void Core_InitSio(void)
{
    BYTE code * data_pntr;
    BYTE cnt;

  	SET_MASK(LSIOHA,LKCFG);
  	SET_MASK(IBMAE,CFGAE);
  	SET_MASK(IBCTL,CSAE);

    cnt=0;
    data_pntr=Core_initsio_table;
    while(cnt < (sizeof(Core_initsio_table)/2) )
    {
        IHIOA=0;              // Set indirect Host I/O Address
        IHD=*data_pntr;
        while( IS_MASK_SET(IBCTL,CWIB));
        data_pntr ++;

        IHIOA=1;              // Set indirect Host I/O Address
        IHD=*data_pntr;
        while( IS_MASK_SET(IBCTL,CWIB));
        data_pntr ++;
        cnt ++;
    }

 	CLEAR_MASK(LSIOHA,LKCFG);
  	CLEAR_MASK(IBMAE,CFGAE);
  	CLEAR_MASK(IBCTL,CSAE);
}