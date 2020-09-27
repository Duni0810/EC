/*-----------------------------------------------------------------------------
 * TITLE: CORE_SCN.C - code to handle local keyboard scanning.
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

// 延时一个时钟周期
void CapDelay(void)
{
    WNCKR = 0x00;               // Delay 15.26 us. page 411, 使用65.536KHz 频率
    //WNCKR = 0x00;               // Delay 15.26 us	
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   scan_keys
 *
 * Scan key matrix once.
 *
 * Return: value != 0, scan activity detected, scan again later. 检测到扫描活动，稍后再扫描。
 *         value  = 0, no scan activity detected. 未检测到扫描活动。
 * 
 * // 若返回 非0 表示有按键动作(make, break or repeat)
 * ------------------------------------------------------------------------- */
BYTE scan_keys(void)
{
	BYTE result;
	result = FALSE;
	//TF_006++>>
    /* key buffer half-full or not */
    // 这个buffer实际上就是一个环形缓冲区
    // 如果buffer 超过一半的话 返回0x01  并且超过一半的buffer 就不进行扫描操作
    if(check_keyscan_buffer_half_full() != 0x00)
    {
        return(0x01);  // 
    }

    //TF_006++<<
    // 先清空按键状态
    scan.saf_make  = 0;  // 通码
    scan.saf_break = 0;  // 断码
    scan.saf_keys  = 0;  // 按键有效

    // 感觉像做检测按键按下的状态类型  表示是什么按键按下和通码断码的状态
    // 键盘上有动作时，typematic.byte 不为0 ,实际上 这个值在 change_valid()函数中被改变
    // change_valid 这个函数在debounce_key 中
 	if (typematic.byte)
	{
        // 如果在这个函数中判断的是repeat code，则将repeat 数据重新发送即可
        // 否则什么都不做
     	check_tm(typematic);  	// Check typematic.
 	}

    // 表示有新的按键按下
	if (new_keyh.byte)   		// Exist Debounce key?  Debounce：防反跳
 	{   						// Yes, bounding. 
     	scan.saf_make = 1;   	// Set debounce make bit.  通码标志
      	debounce_key();         // 这个函数里面有 change_valid 他会改变数值 typematic
      	result = TRUE;
 	}
    
    // result = false 进入函数
    // 如果是第一次按下按键也会进这里
    if (!result)				// Otherwise, scan all. 
    {
        // 十六列的扫描
        for (ITempB03=0;ITempB03<16;ITempB03++)
        {
            // 将相应的位置1，其实就是设置矩阵扫描的电平信号(KSO)；传入的数据为每列的编码号
        	Write_Strobe(ITempB03);
			CapDelay();         // 这里是延时一个时间周期

            // 这个是读取感应线状态，默认KSI位高电平，如果有按键按下，则相应的感应线为低电平
         	ITempB02 = KSI; 	// Read in KI 0 - 7 sense line data.  default is High
			//-----------------------------------
			//Label:BLD_TIPIA_20160827_005

            // 扫描到最后一列，并且也不明白为什么要 “| 0x02”
			if(0x0F==ITempB03)  // 当扫描到第16行(从第0行开始)我猜测这个是预留给外部拓展键盘的代码
			{
				/*
				if(Read_Delete())
				{
					ITempB02 = ITempB02 & 0xFD;
				}
				else
				{
				*/
					ITempB02 = ITempB02 | 0x02;
				//}
			}
			//-----------------------------------
            
            // 这里 ITempB02 表示的是 KSI sense line 的状态
            // 并且默认KSI为高电平，如果有按键按下，则相应的感应线为低电平
            // ~ITempB02 表示 有按键按下的感应线变为高电平
			ITempB02 = (~ITempB02) ^ bscan_matrix[ITempB03];
            
            // 检测到有新的按键按下
            if (ITempB02 != 0)
           	{
                // 传入的是 感应线状态，和当前扫描的列数
            	check_scan(ITempB02, ITempB03); 
          	}

            // 判断上一个按键是否仍然有效
            if (bscan_matrix[ITempB03]) 
          	{  							// Here, if current still active. 
                scan.saf_keys = 1;   	// Set keys active bits. Check all key release. 
            }   

            // 清空矩阵键盘状态
			KSOL=0xFF;	
			KSOH1=0xFF;
        } // 扫描16行结束

        // If ghost exist, make key clear.
        if (new_keyl.field.ghost == 1)
        {  
            new_keyl.field.ghost = 0;	// Ghost flag is set. Clear it.
            ITempB01 = new_keyl.byte & 7;
            if (ITempB01 == 0)
            {  
                ITempB01 = Byte_Mask((BYTE) (new_keyh.field.input));
                ITempB01 = ITempB01 & diode_key; 
            }

            if (ITempB01 == 0)
            {   
                new_keyh.byte = 0;	// Clear new key. 
                new_keyl.byte = 0;
            }
        } 
    }

	return((BYTE) (scan.saf_make + scan.saf_break + scan.saf_keys));
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Write_Strobe
 *
 * Writes the scan matrix KSO[15:0] lines based on Scan Line bit number.
 *
 * KSO[15:0] will be all floats or open except for the bit corresponding to
 * the Scan Line number.  This bit will be low to be detected by the sense
 * line input port later.
 *
 * Config.Msk_Strobe_H and Config.Msk_Strobe_L specify which of the 16 bits on
 * the port are used for the scanner.  (1 = Scanner, 0 = Other OEM function).
 * This allows port pins which are not used for the scanner output lines to be
 * used for something else.
 *
 * Input: Scan line number.
 * 
 * 
 * 功能：Write_Strobe
 * 根据“扫描线”位元数写入扫描矩阵KSO [15：0]行。
 * 
 * KSO [15：0]将全部浮动或断开，除了与扫描行号相对应的位。 该位将为低，以便稍后由感测线输入端口检测到。
 * 
 * Config.Msk_Strobe_H和Config.Msk_Strobe_L指定端口的16位中的哪一个用于扫描程序。 （1 =扫描仪，0 =其
 * 他OEM功能）。 这样可以将不用于扫描仪输出线的端口引脚用于其他用途。
 * 输入：扫描行号。
 * 
 * 
 * 其实相当于低电平扫描有效行

 * ------------------------------------------------------------------------- */
void Write_Strobe(BYTE scan_line_num) //default is High, Low effective
{
    // 这个有循环16字节，其中分为高8位和低8位
    // 如果是低8位，则先将高8位设置为高电平，其余的循环扫描到低8位的某一位，则某个位置设置为0
    // 例如循环到 第三行，则 设置为 1111 1111 1111 0111
	if (scan_line_num<8)
   	{   

     	KSOL=~(0x01<<scan_line_num);
      	KSOH1=0xFF;
  	} else {

    // 当扫描到高8位时，即设置低8位为1，其余的循环扫描到高8位的某一位，则某个位置设置为0
    // 下面代码中 -0x08是因为寄存器分为高8位和低8位，所以设置设置高8位必须减去偏移量，然后写入寄存我
    // 例如循环到 第12行，则 设置为 11110 1111 1111 1111
     	KSOL=0xFF;
   		KSOH1=~(0x01<<(scan_line_num-0x08));
   	} 

    // 判断有误外部键盘
	if(ExtendMatrix)
	{
		KSOH2 = 0xFF;
		Hook_SetGPIOScanPinH();
	}
}

/* ----------------------------------------------------------------------------
 * FUNCTION: check_scan
 *
 * Input:
 *     changes = Any bit set indicates there is a difference in state from
 *         last KSI to this KSI.
 *     scan_address = Address of strobe line that was low when changes was
 *         computed.
 *
 * NOTE:  For the shift and bit test of "changes" to work, "changes" MUST be
 * unsigned.  (Right shifting an unsigned quantity fills vacated bits with
 * zeros.  Right shifting a signed quantity will fill vacated bits with sign
 * bits on some machines and zero bits on others.)
 * ------------------------------------------------------------------------- */
// 这里传入 change 为 sense line 状态， scan_address 为 扫描的列地址
static void check_scan(BYTE changes, BYTE scan_address)
{
    BYTE flag;
    BYTE change_make_key;
    BYTE bit_num; 
     
	flag = FALSE;					// Simple Matrix. 


	if (new_keyl.field.ghost == 1)
 	{
    	flag = TRUE;
	}
	else
	{   /* Find 0 -> 1 's. */
        // 其实这里是为了找到改变之前的make key 的键码
        // 例如我之前sense line 状态为 0x01, 现在的状态为0x04
        // 所以在之前进行 ITempB02 = (~ITempB02) ^ bscan_matrix[ITempB03] 操作是找到所以改变的按键
        // 这里的与操作是为了反找到改变的按键，就是0x40
        // 实际上这里的 changes 为存在变化的混合的sense 状态， 按上面的例子，这里的 changes 为0x05(0x01 ^ 0x04)
		change_make_key = changes & ~bscan_matrix[scan_address];

        // 改变的键值为 空
     	if (change_make_key == 0) 
		{
           	flag = FALSE;
    	}
   		else if (find_paths(change_make_key, scan_address))   // 判断鬼键是否存在
		{
         	flag = TRUE;    /* TRUE if ghost keys. */
       	}
	}

    // 这个函数就是判断鬼键存在，并设置鬼键的flag
	if (flag)
 	{   /* Ghost exists. */
  		new_keyl.field.ghost = 1;  /* Set ghost flag. */
    	/* Only look at 1 -> 0 transition & diode key always no ghost. */
        // 消除鬼键，这个办法就是去除引发鬼键的按键
     	changes &= bscan_matrix[scan_address] | diode_key;
	}

    bit_num = 0;
    while (changes != 0)
    {  
        // 当前sense line 有按键按下  
        // change 为改变的状态混合关系
        if (changes & 0x01) 	/* Look at changes 1 bit at a time. */
		{
            cscfnd(bit_num, scan_address);
        }
        bit_num++;
        changes = changes >> 1; /* Shift bit out (and a zero bit in) to check next bit. */
    }
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   cscfnd
 *
 * Find changed bit.  This subroutine is called for each bit in this KSI that
 * is different from last KSI.
 * ------------------------------------------------------------------------- */
// 查找更改的位。 对于此KSI中与上一个KSI不同的每个位
static void cscfnd(BYTE bit_num, BYTE scan_address)
{
    // Byte_Mask(bit_num) = (1 << (bit_num))
    // 实际上，在这个函数的前一个函数中已经有一个前提，就是在混合状态已经有检测到按键按下，这里
    // 实际上就是判断上一个按键的状态如何，如果上一个按键是按下的，这里必然是弹起
    // 所以只针对 通码与断码有关
    if (bscan_matrix[scan_address] & Byte_Mask(bit_num))
    {  
        if (scan.saf_break == 0) 
		{
            scan.saf_break = 1;
            setup_debounce(bit_num, scan_address, BREAK_EVENT);
        }
    }
    else
    {   
        if (scan.saf_make == 0) 
		{
            scan.saf_make = 1;
            setup_debounce(bit_num, scan_address, MAKE_EVENT);
        }
    } 
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   setup_debounce
 *
 * Input:
 *     scan_address = number of bit of KO.
 *     bit_num = number of bit that changed from last KI to this KI for KO.
 *     event = contact event (MAKE_EVENT or BREAK_EVENT).
 * ------------------------------------------------------------------------- */
static void setup_debounce(BYTE bit_num, BYTE scan_address, BYTE event)
{
    new_keyh.field.output = scan_address;  // 保存 output 地址位
    new_keyh.field.input = bit_num;        // 保存input地址位     通过这两个地址能确定唯一按键
    new_keyh.field.debounce_status = 1;
    new_keyl.field.state = 0;  /* Debounce has not counted out. */
    new_keyl.field.same = 0;   /* Key activity indication. */

    if (event == MAKE_EVENT)
    {   /* For MAKE key (key pressed). */
        new_keyl.field.trans = 0;
        new_keyl.field.count = Ext_Cb2.field.Break_Count; /* Shouldn't this be Make_Count? */
    }
    else
    {   /* For BREAK key (key released). */
        new_keyl.field.trans = 1;
        new_keyl.field.count = Ext_Cb2.field.Make_Count; /* Shouldn't this be Break_Count? */
    }
}

/* ----------------------------------------------------------------------------
 * FUNCTION: find_paths
 *
 * Input:  DOWN_STATE  scan_address
 *         change_make_key
 *
 * Return: FALSE if no paths found, Otherwise returns TRUE.
 * ------------------------------------------------------------------------- */
// change_make_key 是 新的按键位置， scan_address 这个是扫描的地址(0-15)
// 我觉得这个像是在判断鬼键，返回时true表示为鬼键，false 为不是鬼键
static FLAG find_paths(BYTE change_make_key, BYTE scan_address)
{
    FLAG paths_found, done;
    BYTE paths, temp, bits;
    BYTE first_address;

    /* For the shift and bit test to work, "bits" MUST be unsigned.  (Right
       shifting an unsigned quantity fills vacated bits with zeros.  Right
       shifting a signed quantity will fill vacated bits with sign bits on some
       machines and zero bits on others.) */
    // 初始化 局部变量
    done = FALSE;
    paths_found = FALSE;
    first_address = scan_address;  //  保存 线状态

    // 因为diode_key 一直为0，所以 对他取反就全为1
    // 下面这个语句还是 change_make_key 本身
    change_make_key &= ~(diode_key); /* Ignore diode key. */

    /* change_make_key = bKO_BITS. */
    // 空 ，表示下面函数直接不执行，返回为 paths_found = FALSE
    if (change_make_key == 0) 
	{
        done = TRUE; /* paths_found == FALSE */
    }

    // 可能有数据
    if (!done)
    {
        // paths 表示的是混合地址
        paths = bscan_matrix[scan_address] | change_make_key;
        paths &= ~(diode_key);  /* Ignore diode key. */
        /* paths = bKO_PATHS. */
        if (paths == 0) 
		{
            done = TRUE;  /* paths_found == FALSE */
        }
    }

    // 表示 paths 不为空，当然 一般不会是空的，因为只要diode_key 不为FF 就不会是空
    while (!done)
    {
        // 扫描非当前列数据，因为判断的是鬼键，所以只要保证在一行上有两个或两个以上按键按下，
        // 如果第三列与这些按键的行列有关，则表示
        scan_address++;

        // 到这里都只是为了循环一遍扫描线，扫描一圈之后跳出循环
        if (scan_address >= MAX_SCAN_LINES) 	
        //if(scan_address >= STANDARD_SCAN_LINES)
		{
            scan_address = 0;   /* Wrap around */
        }

        if (scan_address == first_address) 
		{
            done = TRUE;        /* No scan lines left.  paths_found == FALSE */
        }
        ///////////////////////////////////////////


        // 如果没找到位置的话
        if (!done)
        {   /* Check Path */
            temp  = bscan_matrix[scan_address]; /* Any paths? */
            temp &= ~(diode_key);               /* Ignore diode key */

            
            if (temp != 0) 
			{    /* Paths found */
                // 感觉像是获取前一个值，这里的paths 是 混合值，例如前一个值为0x01 当前为0x04, 
                // 这里的paths 就是0x5 ，这里计算出的temp是其他列与扫描列的比较
                temp &= paths;  /* Do paths line up? */
            }

            // 其他列与扫描列的比较有交集表示处于同一行上，则进入判断
            if (temp != 0)
            {   /* Paths line up */
                // 由于temp表示的是正在循环的扫描列与混合地址状态的不同
                // 如果 正在循环的扫描列的sense line 状态与 按下按键的sense状态一致
                // 则表示的是有 在在同一行完全相同按键，这个不是鬼键,否则就是存在鬼键
                if (change_make_key != temp)  /* Only 1 bit set? */
                {   /* No, ghost exists. */
                    // 如果进来就说明这个大循环也结束了
                    paths_found = TRUE;
                    done = TRUE;
                }

                if (!done)
                {   /* The following checks if more than one bit is set to 1.
                       Because "bits" is unsigned, each shift moves the bit
                       under test out and a 0 bit in.  When the first 1 bit is
                       found, if "bits" == 0, this indicates that there is only
                       one bit set. */
                    bits = paths;  /* Only 1 bit set? */
                    temp = FALSE;

                    // 这里是按位找到鬼键的位置
                    // 这个do-while 表示的是表示在这列中找到一个按键了
                    // 并且在后面的if判断中，看下还有没有按键按下，如果有的话表示两个按键按下在同一列
                    // 这个时候就说明是鬼键
                    do
                    {
                        if (bits & 0x01) 
						{
                            temp = TRUE;
                        }

                        /* Shift bit out (and a zero bit in) to check next bit. */
                        bits = bits >> 1;
                    } while (temp == FALSE);

                    if (bits != 0)
                    {   /* No, more than 1 bit set. */
                        paths_found = TRUE;
                        done = TRUE;
                    }
                } /* if (!done) */
            } /* if (temp != 0) */
        } /* if (!done) */
    } /* while (!done) */

   return (paths_found);
}

/* ----------------------------------------------------------------------------
 * FUNCTION: debounce_key
 * ------------------------------------------------------------------------- */
static void debounce_key(void)
{							
    scan.scan_line = new_keyh.field.output;	
    Write_Strobe(scan.scan_line); 
	CapDelay();	
    ITempB01 = Read_Scan_Lines();
	//-----------------------------------
	//Label:BLD_TIPIA_20160827_005
	if(0x0F==scan.scan_line)
	{
		/*
		if(Read_Delete())
		{
			ITempB01 = ITempB01 & 0xFD;
		}
		else
		{
		*/
			ITempB01 = ITempB01 | 0x02;
		//}
	}
	//----------------------------------- 
    ITempB01 = ~ITempB01;

    ITempB02 = Byte_Mask((BYTE) (new_keyh.field.input));	
    ITempB02 = ITempB02 & ITempB01;

    if (ITempB02 != 0)
    {  
        if (new_keyl.field.trans == 0) 
		{
            new_keyl.field.same = 1;    // last key detected as a MAKE,  same = 1.
        }
        else 
		{
            new_keyl.field.same = 0;    // last key detected as a BREAK, same = 0.
        }
    }
    else
    { 
        if (new_keyl.field.trans == 0) 
		{
            new_keyl.field.same = 0;    // last key detected as a MAKE,  same = 0. 
        }
        else 
		{
            new_keyl.field.same = 1;    // last key detected as a BREAK, same = 1. 
        }
    } 
#if 0		// Dino 0201 change keyboard debounce time to 5ms
    if (new_keyl.field.state == 0)
    {  
        if (new_keyl.field.count != 0)
        {   
            new_keyl.field.count--;
        }
        else
        {
            if (new_keyl.field.same == 0)
            {   
                new_keyh.byte = 0;	// Debounce failed, so claer all data
                new_keyl.byte = 0;
            }
            else
            {   
                new_keyl.field.state = 1;
            } 
        }
    }
    else
#endif
    { 
        if (new_keyl.field.same == 0)
        {   
            new_keyh.byte = 0;	// Debounce failed. 
            new_keyl.byte = 0;
        }
        else
        { 
      		change_valid(); 	// Debounce OK 
        } 
    }
}

/* ----------------------------------------------------------------
 * FUNCTION: Read_Scan_Lines - Read KSI[7:0] of the scan matrix
 *
 * Return: Data from sense lines that are enabled.
 *         The lines that are disabled return 1s.
 *
 * C prototype: BYTE Read_Scan_Lines();
 * ---------------------------------------------------------------- */
// 读取矩阵键盘感应线函数
BYTE Read_Scan_Lines(void)
{
  	return (KSI);                  
}

/* ---------------------------------------------------------------
 * FUNCTION: change_valid
 * --------------------------------------------------------------- */
static void change_valid(void)
{
    ITempB04 = Byte_Mask((BYTE) (new_keyh.field.input)); 
    ITempB03 = bscan_matrix[new_keyh.field.output]; 
    ITempB03 = ITempB03 ^ ITempB04;
    bscan_matrix[new_keyh.field.output] = ITempB03;   // bscan_matrix[]  write only one

	ITempB05 = BREAK_EVENT;           	// Ready for BREAK indication. 
    if ((ITempB04 & ITempB03) != 0)  // It's not same read agin
    {   						
        ITempB05 = MAKE_EVENT;
        typematic.byte = new_keyh.byte;	// Set New Typematic Key.
        scan.TMscale = TM_SCALE;
        bTMcount = bTMdelay;

		etkeytypematic.byte = 0x00;
    }
	
    Send_Scan2(new_keyh, ITempB05);
    new_keyh.byte = 0;
    new_keyl.byte = 0; 
}

/* ----------------------------------------------------------------------------
 * FUNCTION: check_tm - check typematic function.
 *
 * Input: Typematic key
 * ------------------------------------------------------------------------- */
static void check_tm(union KEY key)
{
    // 好像采用的行扫描的方式
    // Byte_Mask((BYTE) (key.field.input));   这句好像是在表明某一行存在按键按下
 	ITempB02 = FALSE;
    ITempB01 = Byte_Mask((BYTE) (key.field.input));  // 3字节 

    // 判断有没有按键按下，其实是按键是否有效
    // bscan_matrix[key.field.output] 极有可能的是保留上一个列状态
    // ITempB01 保持的是当前的列状态，如果进行&操作表示的是状态标志
    ITempB01 = ITempB01 & bscan_matrix[key.field.output]; // bscan_matrix 为19字节的数组


    // 如此的话下面的函数就不进入了，没有按键按下，并清除当前按键类型
    if (ITempB01 == 0)		 	// Release Typematic key?  
    {   
        typematic.byte = 0;	    // Clear Typematic. 
        ITempB02 = TRUE;        // #define TRUE 1
    }

    // 如果按键是make code 的话 ,就是说明有新的按键。下面这个数值会重新赋值，否则不会赋值
    // 如果执行上一个if 判断 ，就不会执行下面操作
    // 说明有按键按下，并且有效
    if (!ITempB02)
    {  
        scan.TMscale--;		    // Count down Prescale.
        if (scan.TMscale != 0)  // TM_SCALE 默认 =  1
		{
            ITempB02 = TRUE;
        }
    }

    // 如果执行上一个if 判断 ，就不会执行下面操作，说明有按键按下
    if (!ITempB02)
    {
        scan.TMscale = TM_SCALE;// Reload prescale counter. 
        bTMcount--;             // Count down TMcount. 
        if (bTMcount != 0) 
		{
            ITempB02 = TRUE;
        }
    }

    // 如果执行上一个if 判断 ，就不会执行下面操作，说明有按键按下
    // 说明按键与上一个按键是同一个，所以是repeat 模式
    if (!ITempB02)
    {
        bTMcount = bTMrepeat;   // Reload TMcount.  bTMrepeat 为20
        Send_Scan2(key, REPEAT_EVENT);
    }
}

/* ----------------------------------------------------------------
 * FUNCTION: Scan_Init - Initialize internal keyboard (scanner)
 * ---------------------------------------------------------------- */
// 就是将所有的矩阵输入线设置为低电平
void Scan_Init(void)				// Lower all KSO lines for scan matrix
{
 	KSOL  = 0x00;  
  	KSOH1 = 0x00;
	
	if(ExtendMatrix)				// The function of extend keys
	{
		KSOH2=0x00;
		
		Hook_SetGPIOScanPinCtrl();
		Hook_SetGPIOScanPinL();
	}
}

/* ----------------------------------------------------------------
 * FUNCTION: Enable_Any_Key_Irq
 *
 * Setup for any key from scanner to generate an interrupt.
 *
 * Lower the "strobe" lines so that when any key is pressed at least one input
 * line will go low.  Any bits (out of the possible 16) that are not used for
 * the internal keyboard (scanner) are left alone.  This is done using
 * Config.Msk_Strobe_H and Config.Msk_Strobe_L.
 *
 * Then, if the internal keyboard (scanner) is enabled, allow the interrupt to
 * occur when a key is pressed on the scanner.
 * -------------------------------------------------------------- */
void Enable_Any_Key_Irq(void)	// Lower all KSO lines for scan matrix
{
	KSOL=0x00;             
  	KSOH1=0x00;
	
	if (ExtendMatrix)
	{
		KSOH2=0x00;
		Hook_SetGPIOScanPinL();
	}	
	SET_MASK(IER1,Int_KB);      // enable KSI interrupt
	/*
	//Label:BLD_TIPIA_20160827_005
	//Label:BLD_TIPIA_20160921_009 (Disable GPIOH0 interrupt)
	IPOLR6 = IPOLR6|Int_WKO60;   // level low triggered
	ISR6 = Int_WKO60;                  // clear interrupt
	SET_MASK(IER6,Int_WKO60);    // Enable GPIOH0 interrupt
*/
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   Start_Scan_Transmission
 *
 * Lock out other devices so scanner can transmit data to Host, and start
 * Timer B with event "SEND_ENABLE".  When the "RESPONSE_TIME" has expired,
 * the Timer B interrupt handler will post a "SEND" service request to be
 * dispatched by the main service loop.  At that time the data will be sent to
 * the Host via the "SEND" service handler.
 * ------------------------------------------------------------------------- */
void Start_Scan_Transmission(void)
{
    //Load_Timer_B();
    //Timer_B.fbit.SEND_ENABLE = 1;
    SetServiceSendFlag();
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   Check_Scan_Transmission
 *
 * See if the scanner keyboard data transmission (to Host) should be allowed.
 *
 * Return: Transmission status indication.  TRUE if ok to transmit.
 * ------------------------------------------------------------------------- */
FLAG Check_Scan_Transmission(void)
{
    FLAG allow_transmission = FALSE;

    // 传输标准位没被锁定并且没被禁止
    if ((Int_Var.Scan_Lock == 0) && (Flag.SCAN_INH == 0)) 
    {   	/* Scanner transmission is locked and inhibited. */ 
    
        if (scan.kbf_head != scan.kbf_tail) // buffer 不为空
		{
            allow_transmission = TRUE;  /* Allow transmission */
        }
    }    

    return (allow_transmission);
}

/* ----------------------------------------------------------------
 * FUNCTION: Get_Kbd_Type
 *
 * Returns the type of the internal keyboard.
 *
 * Returns: 0 = U.S. keyboard, 1 = Japanese keyboard.
 * -------------------------------------------------------------- */
FLAG Get_Kbd_Type(void)
{						// Check keyboard type here
						// Returns: 0 = U.S. keyboard, 1 = Japanese keyboard.
	if(1)				// check type here
	{
		return(0);  	// U.S. keyboard. 
	}
	else
	{
		return(1);		// Japanese keyboard
	}
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Lock_Scan
 *
 * Lock out the scanner via "Scan_Lock" and clear pending scanner "SEND"
 * events.
 * ------------------------------------------------------------------------- */
void Lock_Scan(void)
{
    Int_Var.Scan_Lock = 1;
    //Timer_B.fbit.SEND_ENABLE = 0;
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Init_Kbd - Scanner Keyboard Initialization.
 * ------------------------------------------------------------------------- */
void Unlock_Scan(void) 
{
	Int_Var.Scan_Lock = 0;
}

void CheckKSO1617Support(void)
{
	ExtendScanPin = 0x00;

    // 对于 KSO16CtrlReg/KSO17CtrlReg(1623/1625) 寄存器默认为GPI（general purpose input???）
    // 在手册page 262 与 page 265 有说明. 默认情况下,这两个寄存器对应的GPIO 为输入不带上下拉
    // 所以默认情况下这两个寄存器默认为 0x10000000。
    // 故下面这个运行情况： ExtendScanPin = 0   ExtendMatrix = 0
	if((KSO16CtrlReg&0xFB)==0x00)
	{
		ExtendScanPin++;
	}

	if((KSO17CtrlReg&0xFB)==0x00)
	{
		ExtendScanPin++;
	}
	
	if(ExtendScanPin!=0x00)
	{
		ExtendMatrix = 1;
	} 
	else
	{
		ExtendMatrix = 0;
	}
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Init_Kbd - Scanner Keyboard Initialization.
 * ------------------------------------------------------------------------- */
void Init_Kbd(void)
{
	CheckKSO1617Support();

	Scan_Init();  

	Hook_Setup_Scanner_Pntr();
    Setup_Diode_Key();          // ???

    new_keyh.byte = 0;
    new_keyl.byte = 0;

 	ClearExtendKeys();	// for extend keys

    Clear_Fn_Keys();
    Clear_Key();        /* Clear key matrix/buffer */
    Clear_Typematic();

	Enable_Any_Key_Irq();

    FN_Key_Break = 0x00;        // Clear FN_Key_Break
    FN_Key_Break_HS = 0x00;
    table_entry_bk = 0x00;
    
    #if KB_FnStickKey
    FnStickKey = 0;
    #endif
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   Setup_Diode_Key
 * ------------------------------------------------------------------------- */
void Setup_Diode_Key(void)
{
 	if(1)
	{   
        //if (Ext_Cb3_DISABLE_NKEY) { /* Simple matrix (NO diodes) */
        diode_key = 0x00;
    }
    else 
	{                      /* N-key (diodes in keyboard) */
        diode_key = 0xFF;
    }
}


/* ----------------------------------------------------------------------------
 * FUNCTION: Clear_Key - Clear local keyboard buffer and related variables.
 * ------------------------------------------------------------------------- */
void Clear_Key(void)
{
    BYTE i;
    //WORD msk_strobe;

    for(i = 0; i < MAX_SCAN_LINES; i++)	// Clear scan matrix.
	{
        bscan_matrix[i] = 0;
    }

    for(i = 0; i < KBF_SIZE; i++)		// Clear key buffer.
	{
        bKEY_BUFF[i] = 0;
    }

    scan.kbf_head = 0;
    scan.kbf_tail = 0;
    typematic.byte = 0;
    scan.saf_make = 0;
    scan.saf_break = 0;
    scan.saf_keys = 0;
	scan.scan_line = 0;
}


/* ----------------------------------------------------------------------------
 * FUNCTION: Clear_Typematic - Set default typematic delay and clear type-matic action.
 * ------------------------------------------------------------------------- */
void Clear_Typematic(void)
{
    typematic.byte = 0;
    bTMdelay = TM_DELAY;
    bTMrepeat = TM_REPEAT;
}

/* ----------------------------------------------------------------------------
 * FUNCTION: Set_Typematic - Set the typematic rate.
 *
 * Input: typematic rate
 *        Bit[4:0] Typematic repeat interval
 *        Bit[6:5] Delay time
 * ------------------------------------------------------------------------- */
//const BYTE code repeat_tbl[] =
//{
//     3,  4,  4,  5,  5,  6,  6,  7,  7,  8,  9, 10, 11, 12, 13, 14,
//    15, 16, 18, 20, 22, 24, 25, 27, 30, 33, 37, 41, 44, 48, 52, 55
//};
//const BYTE code delay_tbl[] = {27, 55, 83, 111};

const BYTE code repeat_tbl[] =
{
     3,  4,  4,  5,  5,  5,  6,  6,  7,  7,  8, 9, 10, 11, 11, 12,
    13, 15, 16, 18, 20, 21, 23, 25, 27, 30, 33, 37, 40, 43, 47, 50
};
const BYTE code delay_tbl[] = {25, 50, 75, 100};

void Set_Typematic(WORD type_rate)
{
    Save_Typematic = type_rate;   /* Save for suspend/resume. */

    /* Bit 4 - 0 typematic repeat interval index. */
    bTMrepeat = repeat_tbl[type_rate & 0x1F];

    /* Bit 6 - 5 delay time index. */
    bTMdelay = delay_tbl[(type_rate >> 5) & 0x03];
}


/* ----------------------------------------------------------------------------
 * FUNCTION: Handle_Scan - Service the local keyboard in response to a key press.
 * ------------------------------------------------------------------------- */
void service_scan(void)
{
 	BYTE scan_activity;

    if (Timer_A.fbit.TMR_SCAN == 0)
    {
        Timer_A.fbit.TMR_SCAN = 1;	// Activate scan timer. 使能扫描状态
    }
	
    // 判断有没有外部按键
	if(ExtendMatrix)//---------------- extend keys, matrix is 18*8
	{
		scan_activity = scan_keys()|bScanExtendKeys();
	}
	else //-----------------------------matrix is 16*8
	{
		scan_activity = scan_keys();// Scan keyboard matrix one time.
	}

	// scan_activity = 1  有效才能进去 未检测到扫描活动， 这是清空操作
    // 这里面会做清空按键服务操作
    if(!scan_activity)			   // No scan activity
    {   
        Timer_A.fbit.TMR_SCAN = 0; // Disable the scan timer. 
        F_Service_KEY = 0;         // Clear any pending service request.  
        Enable_Any_Key_Irq();      // Enable Any Key interrupt.
        FN_Key_Break = 0x00;       // Clear FN_Key_Break
        FN_Key_Break_HS = 0x00;
        table_entry_bk = 0x00;
  	}

    // 如果有定时器2 按键中断没有被使能
    if (Timer_B.fbit.SEND_ENABLE == 0) 	// 1 = Device send request (Request already set)
    {
   		if (Check_Scan_Transmission())	// Data available 数据有效性检查
		{   
     		Start_Scan_Transmission();	// Start new transmission
     	}
    }
}


/*****************************************************************************/
/********** SCANNER KEY BUFFER ROUTINES **************************************/
/*****************************************************************************/

/* ----------------------------------------------------------------------------
 * FUNCTION:   Get_Buffer
 *
 * Get data byte from keyboard buffer (if not empty) and update "scan.kbf_head"
 * with new index into keyboard buffer.
 *
 * Return: Data from buffer (WORD of all 1's if buffer is empty).
 * ------------------------------------------------------------------------- */
//WORD Get_Buffer(void)
BYTE Get_Buffer(void)
{
    //WORD buffer_data;
    BYTE buffer_data;

    if (scan.kbf_head != scan.kbf_tail)
    {
        //buffer_data = (WORD) bKEY_BUFF[scan.kbf_head++];
		buffer_data = bKEY_BUFF[scan.kbf_head++];
        if (scan.kbf_head >= KBF_SIZE) 
		{
            scan.kbf_head = 0;
        }
    }
    else
    {   /* Buffer is empty. */
        buffer_data = 0xFF;
    }
    return (buffer_data);
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   Buffer_Mark - Mark local keyboard buffer tail.
 * ------------------------------------------------------------------------- */
void Buffer_Mark(void)
{
    scan.kbf_mark = scan.kbf_tail;   /* Copy scan.kbf_tail to scan.kbf_mark. */
}

/* ----------------------------------------------------------------------------
 * FUNCTION:   Buffer_Key
 *
 * Input:  Row/Column (0iii,oooo) to put in buffer.
 * Return: TRUE operation successful, FALSE unsuccessful.
 * ------------------------------------------------------------------------- */
FLAG Buffer_Key(BYTE row_column)
{
    FLAG ready = TRUE;  /* Ready for successful operation */

    bKEY_BUFF[scan.kbf_tail] = row_column;  /* Store Data to Buffer Tail */
    scan.kbf_tail++;                        /* Increment Buffer Tail (pointer) */

    if (scan.kbf_tail >= KBF_SIZE) 
	{
        scan.kbf_tail = 0;  /* Wrap pointer if too large. */
    }

    /* Check Overflow */
    if (scan.kbf_tail == scan.kbf_head)
    {   /* Overflow */
        scan.kbf_tail = scan.kbf_mark;  /* Restore buffer marker. */
        bKEY_BUFF[scan.kbf_tail] = 0;   /* Set OverFlow Mark. */
        ready = FALSE;                  /* Overflow Indication !!! */
    }

    return (ready);
}


/* ----------------------------------------------------------------------------
 * FUNCTION:   Buffer_String
 *
 * Places a string in the local keyboard buffer (without a terminating NULL).
 * Call Buffer_Key to place data in key buffer.
 *
 * Input:  Pointer to null terminated string.
 * Return: 0x80 if buffer overflow error, else 0.
 * ------------------------------------------------------------------------- */
BYTE Buffer_String(const BYTE *pntr)
{
    register BYTE error = 0;

    while ((*pntr != 0) && (error == 0))
    {
        if (Buffer_Key(*pntr++) == 0) {
            error = 0x80;   /* Indicate Overflow */
        }
    }

    return (error);
}

//*****************************************************************
// The functions of extend keys
//*****************************************************************
//-----------------------------------------------------------------
// The function of Scaning extend keys
//-----------------------------------------------------------------
BYTE bScanExtendKeys(void)
{
	ITempB06 = FALSE;
	//TF_006++>>
    /* buffer half-full */
    if(check_keyscan_buffer_half_full()!=0x00)
    {
        return(0x01);
    }
    //TF_006++<<
	extendscan.saf_break = 0;
	extendscan.saf_make= 0;
	extendscan.saf_keys= 0;

 	if (etkeytypematic.byte) 
	{
     	CheckEtKeystm(etkeytypematic);	// Check typematic. 
 	}

	if (new_extendkey.byte)   		
 	{   								// Yes, bounding.
     	extendscan.saf_make = 1;  
      	DebounceExtendkey();
      	ITempB06 = TRUE;
 	}

	if(ITempB06 == FALSE)
	{
        for (ITempB01=0;ITempB01<ExtendScanPin;ITempB01++)
        {
			SetExtendScanLines(ITempB01);
			ITempB02 = KSI;
			ITempB02 = (~ITempB02)^bscan_matrix[ITempB01+STANDARD_SCAN_LINES];
            if (ITempB02 != 0) 
           	{
				CheckExtendKeys(ITempB02,(ITempB01+STANDARD_SCAN_LINES));
          	}

            if (bscan_matrix[ITempB01+STANDARD_SCAN_LINES]) 
          	{  								// Here, if current still active.
                extendscan.saf_keys = 1;   	// Set keys active bits. Check all key release.
            }   
        }
	}

	if (extendstatus.field.ghost == 1)
 	{  
     	extendstatus.field.ghost = 0;	// Ghost flag is set.
     	ITempB01 = extendstatus.byte & 7;
   		if (ITempB01 == 0)
     	{  
        	ITempB01 = Byte_Mask((BYTE)(new_extendkey.field.input));
        	ITempB01 = ITempB01 & diode_key; 
  		}
    	if (ITempB01 == 0)
      	{   
        	new_extendkey.byte = 0;		// Clear new key. 
        	extendstatus.byte = 0;
      	}
	} 
	
	return((BYTE) (extendscan.saf_make + extendscan.saf_break + extendscan.saf_keys));
}

//-----------------------------------------------------------------
// The function of Setting extend scan lines
//-----------------------------------------------------------------
void SetExtendScanLines(BYTE scan_line)
{
	if(scan_line<0x02)
	{
		KSOL = 0xFF;
		KSOH1 = 0xFF;
		KSOH2 = ~(0x01<<scan_line);
		Hook_SetGPIOScanPinH();
	}
	else 
	{
		KSOL = 0xFF;
		KSOH1 = 0xFF;
		KSOH2 = 0xFF;
		Hook_SetGPIOScanPinL();
	}
}

//-----------------------------------------------------------------
// The function of Setting extend scan lines
//-----------------------------------------------------------------
void CheckExtendKeys(KSI_bit_num, scan_address)
{
    BYTE flag;
    BYTE change_make_key;
       
	flag = FALSE;				
	if (extendstatus.field.ghost == 1)
 	{
    	flag = TRUE;
   	}
	else
  	{   							// Find 0 -> 1 's.
 		change_make_key = KSI_bit_num & ~bscan_matrix[scan_address];
     	if (change_make_key == 0) 
		{
        	flag = FALSE;
     	}
     	else if (find_paths(change_make_key, scan_address)) 
		{
          	flag = TRUE;    			// TRUE if ghost keys.
      	}
	}

	if (flag)
 	{   								// Ghost exists. 
     	extendstatus.field.ghost = 1;	// Set ghost flag.
      	KSI_bit_num &= bscan_matrix[scan_address] | diode_key;
	}

	ITempB03 = 0x00;
    while (KSI_bit_num != 0)
    {   
        if (KSI_bit_num & 0x01) 	
		{
    		if (bscan_matrix[scan_address] & Byte_Mask(ITempB03))
   		 	{  
        		if (extendscan.saf_break == 0) 
				{
            		extendscan.saf_break = 1;
					ExtendKeyDebounce(ITempB03, scan_address, BREAK_EVENT);
        		}
    		}
    		else
    		{
        		if (extendscan.saf_make == 0) 
				{
            		extendscan.saf_make = 1;
					ExtendKeyDebounce(ITempB03, scan_address, MAKE_EVENT);
        		}
    		} 
        }
        ITempB03++;
        KSI_bit_num = KSI_bit_num >> 1; /* Shift bit out (and a zero bit in) to check next bit. */
    }
}

//-----------------------------------------------------------------
// The function of extend keys debounce
//-----------------------------------------------------------------
void ExtendKeyDebounce(BYTE KSI_bit_num, BYTE scan_address, BYTE event)
{
    new_extendkey.field.output = scan_address-STANDARD_SCAN_LINES;
    new_extendkey.field.input = KSI_bit_num;
    new_extendkey.field.debounce_status = 1;
    extendstatus.field.state = 0;  // Debounce has not counted out. */
    extendstatus.field.same = 0;   // Key activity indication. */

    if (event == MAKE_EVENT)
    { 
        extendstatus.field.trans = 0;
        extendstatus.field.count = 1;//Ext_Cb2.field.Break_Count; 
    }
    else
    {  
        extendstatus.field.trans = 1;
        extendstatus.field.count = 1;//Ext_Cb2.field.Make_Count; 
    }
}


void DebounceExtendkey(void)
{
	SetExtendScanLines(new_extendkey.field.output);
	ITempB04 = Read_Scan_Lines();
	ITempB04 = ~ITempB04;
 	ITempB05 = Byte_Mask((BYTE)(new_extendkey.field.input)); 
	ITempB05 = ITempB05 & ITempB04;

	if (ITempB05 != 0)
    {  
        if (extendstatus.field.trans == 0) 	// key make
		{
            extendstatus.field.same = 1;   	// last key detected as a MAKE, same = 1.
        }
        else 
		{
            extendstatus.field.same = 0;    // last key detected as a BREAK, same = 0.
        }
    }
    else
    { 
        if (extendstatus.field.trans == 0) 
		{
            extendstatus.field.same = 0;    // last key detected as a MAKE, same = 0. 
        }
        else 
		{
            extendstatus.field.same = 1;    // last key detected as a BREAK, same = 1. 
        }
    } 

    if (extendstatus.field.state == 0)	// 1 means debounce counted out.
    {  
        if (extendstatus.field.count != 0)
        {  
            extendstatus.field.count--;
        }
        else
        {
            if (extendstatus.field.same == 0)
            {   
                new_extendkey.byte = 0;	// Debounce failed, so claer all data
                extendstatus.byte = 0;
            }
            else
            {  
                extendstatus.field.state = 1;
            } 
        } 
    }
    else
    { 
        if (extendstatus.field.same == 0)
        {   
            new_extendkey.byte = 0;	/* Debounce failed. */
            extendstatus.byte = 0;
        }
        else
        {   
          	CheckExtendKeyValid();
        } 
    }
}


//-----------------------------------------------------------------
// The function of checking extend key valid
//-----------------------------------------------------------------
void CheckExtendKeyValid(void)
{
    ITempB02 = Byte_Mask((BYTE) (new_extendkey.field.input)); 
    ITempB01 = bscan_matrix[(new_extendkey.field.output + STANDARD_SCAN_LINES)];
    ITempB01 = ITempB01 ^ ITempB02;                        
    bscan_matrix[(new_extendkey.field.output + STANDARD_SCAN_LINES)] = ITempB01; 

	ITempB03 = BREAK_EVENT;                       
    if ((ITempB02 & ITempB01) != 0)
    {   						
        ITempB03 = MAKE_EVENT;  							
        etkeytypematic.byte = new_extendkey.byte;
        extendscan.TMscale = TM_SCALE;
        bTMcount = bTMdelay;
		
		typematic.byte = 0x00;
    }
	
    Send_EtScan2(new_extendkey, ITempB03);
    new_extendkey.byte = 0;
    extendstatus.byte = 0;
}

//-----------------------------------------------------------------
// The function of
//-----------------------------------------------------------------
void Send_EtScan2(union KEY key, BYTE event)
{	
	if(IS_MASK_SET(UtilityFlag,KBDebug))
	{
		if(KeyScanACK==KU_ACK_Start)
		{
			if(event==MAKE_EVENT)
			{
				KeyScanKSI = key.field.input;
				KeyScanKSO = key.field.output;
				KeyScanACK = KU_ACK_FullKeyboard;
			}
		}
	}
	else
	{
    	ITempB02 = (key.field.input * 3) + key.field.output; 
		ITempB01 = *(Extendkey_Table_Pntr + ITempB02);

		Et_Hook_keyboard(key.field.input, key.field.output);
    	Send_Key(ITempB01, event);
	}
}

//-----------------------------------------------------------------
// The function of clearing extend keys
//-----------------------------------------------------------------
void ClearExtendKeys(void)
{
	new_extendkey.byte = 0;		
	extendstatus.byte = 0;
	etkeytypematic.byte = 0;
}

//-----------------------------------------------------------------
// The function of checking extend keys Typematic
//-----------------------------------------------------------------
void CheckEtKeystm(union KEY key)
{
	ITempB02 = FALSE;
    ITempB01 = Byte_Mask((BYTE) (key.field.input)); 
    ITempB01 = ITempB01 & bscan_matrix[key.field.output+STANDARD_SCAN_LINES];

    if (ITempB01 == 0) 
    {
        etkeytypematic.byte = 0;
        ITempB02 = TRUE;
    }

    if (!ITempB02)
    {
        extendscan.TMscale--;
        if (extendscan.TMscale != 0) 
		{
            ITempB02 = TRUE;
        }
    }

    if (!ITempB02)
    {
        extendscan.TMscale = TM_SCALE;
        bTMcount--;            
        if (bTMcount != 0) 
		{
            ITempB02 = TRUE;
        }
    }

    if (!ITempB02)
    {
        bTMcount = bTMrepeat;   
        Send_EtScan2(key,REPEAT_EVENT);  
    }
}
//TF_006++>>
//*******************************************************
/**
 * ****************************************************************************
 *
 * key buffer half full or not.
 *
 * @return
 * 0, buffer no half-full
 * 1, buffer half-full
 *
 * @parameter
 *
 * ****************************************************************************
 */
// 如果键盘buffer中的数据超过一半，则返回1， 否则返回0
BYTE check_keyscan_buffer_half_full(void)
{
    BYTE lb_pending_size;
    BYTE lb_result;

    /* pre-set buffer no half-full */
    lb_result = 0x00;
        
    if(scan.kbf_head > scan.kbf_tail)
    {
        lb_pending_size = (scan.kbf_tail + KBF_SIZE) - scan.kbf_head;
    }
    else
    {
        lb_pending_size = scan.kbf_tail - scan.kbf_head;
    }

    if(lb_pending_size >= (KBF_SIZE/2))
    {
        /* buffer half-full */
        lb_result = 0x01;
    }

    return(lb_result);
}
//TF_006++<<