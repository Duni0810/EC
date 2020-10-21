/*----------------------------------------------------------------------------
 * TITLE: CORE_SMBUS.C - the System Management SMBus Protocol drivers.
 *
 *---------------------------------------------------------------------------*/
 
#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

//----------------------------------------------------------------------------
// Read/write SMbus byte/word function	
//----------------------------------------------------------------------------
const sSMBus code asSMBus[]=
{

    // young modified  都是定义SMBUS相关寄存器的操作
    // 0    HOCTL_A      0x1C01     Host Control Register (HOCTL)
    // 1    TRASLA_A     0x1C03     Transmit Slave Address Register (TRASLA)
    // 2    HOCMD_A      0x1C02     Host Command Register (HOCMD)
    // 3    HOSTA_A      0x1C00     Host Status Register (HOSTA
    // 4    D0REG_A      0x1C04     Data 0 Register (D0REG) 
    // 5    D1REG_A      0x1C05     Data 1 Register (D1REG)
    // 6    HOBDB_A      0x1C06     Host Block Data Byte Register (HOBDB)
    // 7    IER1         0x1105                 
    // 8    ISR1         0x1101             
    // 9    Int_SMBUS0   bit1               
    // 10   PECERC_A     0x1C07     Packet Error Check Register (PECERC)
	{ &HOCTL_A, &TRASLA_A, &HOCMD_A, &HOSTA_A, &D0REG_A, &D1REG_A,	&HOBDB_A,	&IER1,	&ISR1,  Int_SMBUS0,   &PECERC_A},
	{ &HOCTL_B, &TRASLA_B, &HOCMD_B, &HOSTA_B, &D0REG_B, &D1REG_B,	&HOBDB_B,	&IER1,	&ISR1,  Int_SMBUS1,   &PECERC_B},
	{ &HOCTL_C, &TRASLA_C, &HOCMD_C, &HOSTA_C, &D0REG_C, &D1REG_C,	&HOBDB_C,	&IER2,	&ISR2,	Int_SMBUS2,   &PECERC_C},
	{ &HOCTL_D, &TRASLA_D, &HOCMD_D, &HOSTA_D, &D0REG_D, &D1REG_D,	&HOBDB_D,	&IER0,	&ISR0,	Int_SMBUS3,   &PECERC_D}	
};

const sResetSMBusS code asResetSMBusS[]=
{
    // 1    HOSTA_A      0x1C00     Host Status Register (HOSTA
    // 2    HOCTL2_A     0x1C10     Host Control Register 2 (HOCTL2)
    // 3    SMBPCTL_A    0x1C0A     SMBus Pin Control Register (SMBPCTL)
    // 4    GPCRB3       0x161B	    Port Control B3 Registers
    // 5    GPCRB4		 0x161C	    Port Control B4 Registers
    // 6    GPDRB 		 0x1602	    Port B Data Register
    // 7    (BIT3+BIT4)  应该是 GPB3,GPB4
    // 8    SMBus1RecoverCunt
	{ &HOSTA_A, &HOCTL2_A, &SMBPCTL_A, &GPCRB3, &GPCRB4, &GPDRB,	(BIT3+BIT4),   &SMBus1RecoverCunt },
	{ &HOSTA_B, &HOCTL2_B, &SMBPCTL_B, &GPCRC1, &GPCRC2, &GPDRC,	(BIT1+BIT2),   &SMBus2RecoverCunt },
	{ &HOSTA_C, &HOCTL2_C, &SMBPCTL_C, &GPCRF6, &GPCRF7, &GPDRF,	(BIT6+BIT7),   &SMBus3RecoverCunt },
	{ &HOSTA_D, &HOCTL2_D, &SMBPCTL_D, &GPCRH1, &GPCRH2, &GPDRH,	(BIT1+BIT2),   &SMBus4RecoverCunt }
}; 

//----------------------------------------------------------------------------
// 	延时操作
//----------------------------------------------------------------------------
void DelayInact(void) 
{
    BYTE index;
    BYTE counter;
   for(index=0;index<200;index++)
   {
        counter++;
   }
}

//----------------------------------------------------------------------------
// The function of reseting SMbus 	
//----------------------------------------------------------------------------
void ResetSMBus(BYTE Channel)
{
    #ifdef PECI_Support
    if(Channel==SMbusCh3)
    {
        return;
    }
    #endif

    // 设置此位后，终止当前主机事务。这个位，一旦设置，必须被软件清除，以允许SMBus主机控制器正常运行
    SET_MASK(*asSMBus[Channel].SMBusCTL,HOCTL_KILL);
    CLEAR_MASK(*asSMBus[Channel].SMBusCTL,HOCTL_KILL);

    // 写 清状态
	//RSTC3=0x0F;					            // Reset all SMBus module
	*asResetSMBusS[Channel].SMBusSTA = 0xFE;	// clear bits

    // 配置SMBUS 引脚为GPO 模式
    *asResetSMBusS[Channel].SMBusPin0 = OUTPUT;
	*asResetSMBusS[Channel].SMBusPin1 = OUTPUT;
	DelayInact();

    // 设置对应引脚为高电平
	*asResetSMBusS[Channel].GPIOReg |= asResetSMBusS[Channel].GPIOData;
	DelayInact();

    // 设置引脚为复用模式
    *asResetSMBusS[Channel].SMBusPin0 = ALT;
	*asResetSMBusS[Channel].SMBusPin1 = ALT;

    // 这个是做I2C 使能，这会儿还没使能SMBUS
	*asResetSMBusS[Channel].SMBusCTL2 = 0x02;

    // 设置clk 和 data 为高电平
	*asResetSMBusS[Channel].SMBusPinCTL = 0x03;
	DelayInact();

    // SMCLK0 / 1/2引脚不会被驱动为低电平。 其他SMBus逻辑控制该引脚 ???
	*asResetSMBusS[Channel].SMBusPinCTL = 0x07;

    // 使能SMBUS
	*asResetSMBusS[Channel].SMBusCTL2 = 0x01;

    #ifdef SMBusServiceCenterFunc

    // 清空状态位
    *asSSC[Channel].inusing=0x00;

    // 清除中断使能
    CLEAR_MASK(*asSMBus[Channel].SMBusIER, asSMBus[Channel].IERFlag);

    // 清中断状态
    *asSMBus[Channel].SMBusISR = asSMBus[Channel].IERFlag;

    // 清标志位
    *asSSC[Channel].serviceflag=0x00;
    //Init_SMBus_Regs();
    
    // 对中SMBUS用户自定义状态做初始化操作
    if(DB_SMBusAck1==ACK_Start)     // For SMBus utility
    {
        DB_SMBusAddr=0x00;
        if(Channel==0x00)
        {
	        DB_SMBusFlag1=0x00;		
        }
        else if(Channel==0x01)
        {
            DB_SMBusFlag2=0x00;
        }
        else if(Channel==0x02)
        {
            DB_SMBusFlag3=0x00;	
        }				
				
	    DB_SMBusACDID=0x00;	
        DB_SMBusAck1 = ACK_Error;   // For SMBus utility error ACK.
    }
    #endif
}

//----------------------------------------------------------------------------
// SMbus read byte/word and write byte/word function	
// SMBUS 读写 位，或者字操作
//----------------------------------------------------------------------------
BYTE bRWSMBus(BYTE Channel,BYTE Protocol,BYTE Addr,BYTE Comd,XBYTE *Var,BYTE PECSupport)
{	
	BYTE counter;
	BYTE error;
    BYTE status;
    BYTE resutl;

    // 检查SMBUS 接口能不能被使用
    if(CheckSMBusInterfaceCanbeUse(Channel, SMBus_AccessType_Fucn)==SMBus_CanNotUse)
    {
        resutl = FALSE;                             // SMBus interface can't be used.
    }
    else
    {
        // 做初始化操作
        error     = 0xEE;                             // Pre-set error
        resutl    = FALSE;                            // Pre-set result is fail
        SMBCRC8_A = 0x00;                             // Clear CRC variable

        // 写操作
	    if (Protocol&0x80)	                        // Write byte function
	    {	
            // 是否支持包错误检查
	        if(PECSupport)    
            {   
                CalcCRC8(Addr);                     // To calculate crc8
            }

            // 赋值地址
		    *asSMBus[Channel].SMBusADR = Addr;      // set address with writing bit
	        if(PECSupport)    
            {   
                CalcCRC8(Comd);                     // To calculate crc8
            }

            // 赋值命令
	        *asSMBus[Channel].SMBusCMD=Comd;	    // Set command
		    Protocol&=(~0x80);
            if(PECSupport)    
            {   
                CalcCRC8(*Var);                     // To calculate crc8
            }

            // 赋值，并检查 是读写字操作还是位操作
		    *asSMBus[Channel].SMBusData0 = *Var;    // set data1
		    if(Protocol==SMbusRW)		            // write word function
		    {
		        if(PECSupport)    
                {   
                    CalcCRC8(*(Var+0x01));          // To calculate crc8
                }
			    *asSMBus[Channel].SMBusData1 = *(Var+0x01);
		    }						                // set data2
	    }	
	    else 						                // Read function  读操作
	    {	
            // 设置地址与命令
		    *asSMBus[Channel].SMBusADR=Addr|0x01;	
            *asSMBus[Channel].SMBusCMD=Comd;	    // Set command
	    }							                // set address with reading bit

        // 清中断标志位
	    *asSMBus[Channel].SMBusSTA=0xFE;		    // clear bits

        // 做CRC8 检验
	    if(PECSupport)
        {
            *asSMBus[Channel].SMBusPEC=SMBCRC8_A;
	        *asSMBus[Channel].SMBusCTL=(Protocol|HOCTL_SRT|HOCTL_PEC_EN);   // Start transaction
        }
        else
        {
	        *asSMBus[Channel].SMBusCTL=(Protocol|HOCTL_SRT);	// Start transaction
        }

        // 初始化超时定时器操作
	    TR1 = 0;			                        // disable timer1
        ET1 = 0;                  	                // Disable timer1 interrupt
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        TH1 = Timer_26ms>>8;                        // Set timer1 counter 26ms
        TL1 = Timer_26ms;                           // Set timer1 counter 26ms
        TF1 = 0;                  	                // Clear overflow flag
        TR1 = 1;                 	                // Enable timer1

        // 检查超时判断
	    while (!TF1)
	    {
            status = *asSMBus[Channel].SMBusSTA;    // Read SMBus Host Status
            if(IS_MASK_SET(status, (HOSTA_FINTR+HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE )))
            {
                TR1=0;
			    TF1=0;
                ET1=1;                  	        // Enable timer1 interrupt
                break;
            }
	    }

        // 如果是超时溢出，则重启SMBUS 
	    if(TF1)                                     // 26ms time-out and no any status bit is set.
	    {
            TR1=0;
		    TF1=0;
            ET1=1;                                  // Enable timer1 interrupt
            ResetSMBus(Channel);
            error = 0xEE;
	    }
        else
        {
            // 检查中断标志，如果是是错误中断，则重启SMBUS
            if(IS_MASK_SET(status, (HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE)))
            {
                if(IS_MASK_SET(status, (HOSTA_BSER+HOSTA_FAIL)))
                {
                    ResetSMBus(Channel);
                }
                error = 0xEE;
            }
            else                                    // Only Finish Interrupt bit is set.
            {
                error = 0x00;
            }
        }

        // 检查错误号         	                    
	    if(error == 0xEE)                           // Fail
	    {
		    resutl = FALSE;
	    }
        else                                        // OK
        {
            // 如果是 读操作，则读取数据
	        if ((*asSMBus[Channel].SMBusADR & 0x01 != 0x00)) 
	        {
		        *Var = *asSMBus[Channel].SMBusData0;    // read data1

                // 判断是否是读取一个字操作，是的话还要读取一个字节
		        if(Protocol==SMbusRW)
		        {
			        *(Var+0x01) = *asSMBus[Channel].SMBusData1;
		        }							            // read data2
	        }
            resutl = TRUE; 
        }

	    *asSMBus[Channel].SMBusSTA=0xFE;	            // clear bits
    }

	return(resutl);
}

//----------------------------------------------------------------------------
// Read SMbus block function	
// SMBUS 块读操作
//----------------------------------------------------------------------------
BYTE bRSMBusBlock(BYTE Channel,BYTE Protocol,BYTE Addr,BYTE Comd,XBYTE *Var)
{
	BYTE ack;
    BYTE bcount;
    BYTE status;

    // 检查是否能使用SMBUS 
    if(CheckSMBusInterfaceCanbeUse(Channel, SMBus_AccessType_Fucn)==SMBus_CanNotUse)
    {
        ack = FALSE;                            // SMBus interface can't be used.
    }
    else
    {
        // 初始化参数，设置地址，指令等操作
	    ack = FALSE;					        // pr-set flag
	    bcount = 0x00;			                // clear byte counter
	
	    *asSMBus[Channel].SMBusADR=Addr|0x01;   // set address with reading bit	
	    *asSMBus[Channel].SMBusCMD=Comd;	    // Set command
        *asSMBus[Channel].SMBusSTA=0xFE;	    // clear bits
        *asSMBus[Channel].SMBusCTL=(Protocol|HOCTL_SRT);    // Start transaction

        // 设置超时定时器，设置初始值为26ms
	    TR1 = 0;			                    // disable timer1
        ET1 = 0;                  	            // Disable timer1 interrupt
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        TH1 = Timer_26ms>>8;                    // Set timer1 counter 26ms
        TL1 = Timer_26ms;                       // Set timer1 counter 26ms
        TF1 = 0;                  	            // Clear overflow flag
        TR1 = 1;                 	            // Enable timer1

        // 大循环做定时器是否溢出检查
        while(!TF1)                             // Not time out
        {
            // 读取SMBUS 中断标志位检查
            status = *asSMBus[Channel].SMBusSTA;    // Read SMBus Host Status

            // 检查中断类型
            if(IS_MASK_SET(status, (HOSTA_BDS+HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE )))
            {
                // 检查是否是传输过程中的错误中断
                if(IS_MASK_SET(status, (HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE)))
                {
                    if(IS_MASK_SET(status, (HOSTA_BSER+HOSTA_FAIL)))
                    {
                        ResetSMBus(Channel);
                    }
                    TR1 = 0;			        // disable timer1
 	                TF1 = 0;			        // clear overflow flag
	                ET1 = 1;			        // Enable timer1 interrupt
	                ack = FALSE;
                    break;
                }
                else                            // Only Byte Done Status bit is set.  传输完成中断标志位
                {
	                *Var = *asSMBus[Channel].SMBusBData;    // get first data  读取一个数据

                    // 如果count 为0 ，去读取数据，这个可能是第一次进入
	                if(bcount ==0x00)
	                {
		                bcount = *asSMBus[Channel].SMBusData0; // get the data of byte count， 获取数个数
		                Hook_GetSMBusReadBlockByteCount(bcount);
	                }	
            
	                bcount--;
	                Var++;

                    // 最后一位操作
	                if(bcount == 0x01)
	                {						    // set last byte flag
		                SET_MASK(*asSMBus[Channel].SMBusCTL,HOCTL_LABY);
	                }

                    // 清中断标志
	                *asSMBus[Channel].SMBusSTA=0xFE;    // clear bits

                    // 没有需要读取的数据，做清空定时器操作，并返回数据发送完成标志
	                if(bcount == 0x00)
	                {
                        TR1 = 0;			    // disable timer1
 	                    TF1 = 0;			    // clear overflow flag
	                    ET1 = 1;			    // Enable timer1 interrupt
                        ack = TRUE;
                        break;                  // SMBus read block done.
                    }
                }
            }
        }
        
        // 检查 是否是定时器溢出标志位
        if(TF1)                                 // time-out
        {
            TR1 = 0;			                // disable timer1
 	        TF1 = 0;			                // clear overflow flag
	        ET1 = 1;			                // Enable timer1 interrupt
		    ResetSMBus(Channel);
		    ack = FALSE;
        }

	    CLEAR_MASK(*asSMBus[Channel].SMBusCTL,HOCTL_LABY);    // clear last byte flag
	    *asSMBus[Channel].SMBusSTA=0xFE;        // clear bits
    }
    
	return(ack);	
}

//----------------------------------------------------------------------------
// Write SMbus block function	
// 块写SMBUS函数
//----------------------------------------------------------------------------
BYTE bWSMBusBlock(BYTE Channel,BYTE Protocol,BYTE Addr,BYTE Comd,XBYTE *Var,BYTE ByteCont,BYTE PECsupport)
{	
	BYTE ack;
    BYTE BCTemp;
    BYTE status;

    // 检查SMBUS 能否被使用
    if(CheckSMBusInterfaceCanbeUse(Channel, SMBus_AccessType_Fucn)==SMBus_CanNotUse)
    {
        ack = FALSE;                            // SMBus interface can't be used.
    }
    else
    {
	    ack = FALSE;					        // pre-set flag
	    BCTemp = 0x00;			                // clear byte count

        // 是否做 包错误检查
        if(PECsupport)
        {   
            SMBCRC8_A=0x00;
            CalcCRC8(Addr);
            CalcCRC8(Comd);
            CalcCRC8(ByteCont);
            CalcCRC8(*Var);
        }
    
        // 初始化  SMBUS 状态，比如地址，指令，设置写个数，设置待写数据的数据地址，清中断标志位
	    Protocol&=(~0x80);			
	    *asSMBus[Channel].SMBusADR=Addr;	    // set address with writing flag
        *asSMBus[Channel].SMBusCMD=Comd;	    // Set command
        *asSMBus[Channel].SMBusData0=ByteCont;  // set byte count
        BCTemp=ByteCont;	                    // sync byte count 
        *asSMBus[Channel].SMBusBData=*Var;	    // set first data
        *asSMBus[Channel].SMBusSTA=0xFE;	    // clear bits

        // 检查是否需要包错误检查，使能对应的相应寄存器
	    if(PECsupport)
        {   
            *asSMBus[Channel].SMBusCTL=(Protocol|HOCTL_SRT|HOCTL_PEC_EN);// Start transaction
        }
        else
        {
            *asSMBus[Channel].SMBusCTL=(Protocol|HOCTL_SRT);// Start transaction
        }

        // 初始化定时器，做超时检查
	    TR1 = 0;			                    // disable timer1
        ET1 = 0;                  	            // Disable timer1 interrupt
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        TH1 = Timer_26ms>>8;                    // Set timer1 counter 26ms
        TL1 = Timer_26ms;                       // Set timer1 counter 26ms
        TF1 = 0;                  	            // Clear overflow flag
        TR1 = 1;                 	            // Enable timer1

        while(!TF1)                             // Not time out
        {
            // 清中断状态标志
            status = *asSMBus[Channel].SMBusSTA;    // Read SMBus Host Status

            // 检查是否有中断产生，包括数据完成中断
            if(IS_MASK_SET(status, (HOSTA_BDS+HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE )))
            {
                // 数据错误中断产生，关闭定时器，并退出数据发送
                if(IS_MASK_SET(status, (HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE)))
                {
                    if(IS_MASK_SET(status, (HOSTA_BSER+HOSTA_FAIL)))
                    {
                        ResetSMBus(Channel);
                    }
                    TR1 = 0;			        // disable timer1
 	                TF1 = 0;			        // clear overflow flag
	                ET1 = 1;			        // Enable timer1 interrupt
	                ack = FALSE;
                    break;
                }
                else                            // Only Byte Done Status bit is set.  数据完成中断
                {
                    // 以下函数是数据发送完成中断，做数据地址偏移操作，并数据数自减
	                BCTemp--;
	                Var++;					    // point to next address of variable

                    // 如果 数据位不为0
	                if(BCTemp != 0x00)
	                {
                        // 如果支持包错误检查
                        if(PECsupport)
                        {
                            *asSMBus[Channel].SMBusBData=*Var;
                            CalcCRC8(*Var);
                            if(BCTemp==0x01)    // last byte of byte count，最后一位
                            {
                                *asSMBus[Channel].SMBusPEC = SMBCRC8_A;
                            }
                        }
                        else  
                        {
		                    *asSMBus[Channel].SMBusBData=*Var;
                        }
                        *asSMBus[Channel].SMBusSTA=0xFE;    // clear bits
	                }
                    else                        // write block transaction done,已经是最后一位，所有数据发送完成
                    {
	                    TR1 = 0;			    // disable timer1
 	                    TF1 = 0;			    // clear overflow flag
	                    ET1 = 1;			    // Enable timer1 interrupt
	                    ack = TRUE;
	                    break;
                    }
                }
            }
        }
        
        // 检查 是否是超时引起的
        if(TF1)                                 // time-out
        {
            TR1 = 0;			                // disable timer1
 	        TF1 = 0;			                // clear overflow flag
	        ET1 = 1;			                // Enable timer1 interrupt
		    ResetSMBus(Channel);
		    ack = FALSE;
        }
        
	    *asSMBus[Channel].SMBusSTA=0xFE;        // clear bits
    }
    
	return(ack);
} 

//----------------------------------------------------------------------------
// The function of SMbus send byte	
// SMBUS 发送字节函数
//----------------------------------------------------------------------------
BYTE bSMBusSendByte(BYTE Channel,BYTE Addr,BYTE SData)
{
	BYTE error;
    BYTE result;
    BYTE status;

    // 检查SMBUS接口能否被使用
    if(CheckSMBusInterfaceCanbeUse(Channel, SMBus_AccessType_Fucn)==SMBus_CanNotUse)
    {
        result = FALSE;                             // SMBus interface can't be used.
    }
    else
    {
        // 初始化参数
        error = 0xEE;                               // Pre-set error
        result = FALSE;                             // Pre-set result is fail
        
        // 设置 地址，数据（命令），清标志位，开启数据传输
	    *asSMBus[Channel].SMBusADR = Addr;	        // set address with writing bit
	    *asSMBus[Channel].SMBusCMD = SData;	        // Set command
	    *asSMBus[Channel].SMBusSTA = 0xFE;	        // clear bits
	    *asSMBus[Channel].SMBusCTL = (0x04|HOCTL_SRT);  // Start transaction

        // 开启定时器，做数据发送的超时处理操作
        TR1 = 0;                                    // Disable timer1
        ET1 = 0;                  	                // Disable timer1 interrupt
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        TH1 = Timer_26ms>>8;                        // Set timer1 counter 26ms
        TL1 = Timer_26ms;                           // Set timer1 counter 26ms
        TF1 = 0;                  	                // Clear overflow flag
        TR1 = 1;                 	                // Enable timer1

	    while (!TF1)
	    { 
            status = *asSMBus[Channel].SMBusSTA;    // Read SMBus Host Status

            // 产生中断标志位
            if(IS_MASK_SET(status, (HOSTA_FINTR+HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE )))
            {
                TR1=0;                              // Disable timer1
			    TF1=0;                              // Clear overflow flag
                ET1=1;                  	        // Enable timer1 interrupt
                break;
            }
	    }

        // 超时还没发送中断，必须得做SMBUS重启操作或者没有产生数据发送完成标志位
	    if(TF1)                                     // 26ms time-out and no any status bit is set.
	    {
            TR1=0;
		    TF1=0;
            ET1=1;                                  // Enable timer1 interrupt
            ResetSMBus(Channel);
            error = 0xEE;
	    }
        else    
        {
            // 检查是不是数据发送错误标志，是的话，重启SMBUS,否则清中断标志位，显示数据传输成功
            if(IS_MASK_SET(status, (HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE)))
            {
                if(IS_MASK_SET(status, (HOSTA_BSER+HOSTA_FAIL)))
                {
                    ResetSMBus(Channel);
                }
                error = 0xEE;
            }
            else                                    // Only Finish Interrupt bit is set.
            {
                error = 0x00;
            }
        }

        // 检查数据是否发送成功，如果数据发送成功，则返回TRUE，反正，返回false
	    if(error == 0xEE)
	    {
		    result = FALSE;
	    }
        else
        {
            result = TRUE;
        }
	    *asSMBus[Channel].SMBusSTA=0xFE;	// clear bits
    }
    
	return(result);
}

//----------------------------------------------------------------------------
// The function of SMbus receive byte	
// SMBUS 接受字节
//----------------------------------------------------------------------------
BYTE bSMBusReceiveByte(BYTE Channel,BYTE Addr,XBYTE *Var)
{
	BYTE error;
    BYTE result;
    BYTE status;

    // 检查 SMBUS 能不能被使用
    if(CheckSMBusInterfaceCanbeUse(Channel, SMBus_AccessType_Fucn)==SMBus_CanNotUse)
    {
        result = FALSE;                             // SMBus interface can't be used.
    }
    else
    {
        // 设置初始值
        error = 0xEE;                               // Pre-set error
        result = FALSE;                             // Pre-set result is fail

       // 设置地址 清状态
	    *asSMBus[Channel].SMBusADR = (Addr|0x01);   // set address with writing bit
	    *asSMBus[Channel].SMBusSTA = 0xFE;		    // clear bits

        // 开始传输  Process Call
	    *asSMBus[Channel].SMBusCTL = (0x04|HOCTL_SRT);  // Start transaction

        // 初始化定时器1 为26ms , 设置26ms超时
	    TR1 = 0;			                        // disable timer1
        ET1 = 0;                  	                // Disable timer1 interrupt
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        TH1 = Timer_26ms>>8;                        // Set timer1 counter 26ms
        TL1 = Timer_26ms;                           // Set timer1 counter 26ms
        TF1 = 0;                  	                // Clear overflow flag
        TR1 = 1;                 	                // Enable timer1


	    while (!TF1)
	    { 
            status = *asSMBus[Channel].SMBusSTA;    // Read SMBus Host Status
            if(IS_MASK_SET(status, (HOSTA_FINTR+HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE )))
            {
                TR1=0;
			    TF1=0;
                ET1=1;                  	        // Enable timer1 interrupt
                break;
            }
	    }
	
	    if(TF1)                                     // 26ms time-out and no any status bit is set.
	    {
            TR1=0;
		    TF1=0;
            ET1=1;                                  // Enable timer1 interrupt
            ResetSMBus(Channel);
            error = 0xEE;
	    }
        else
        {
            if(IS_MASK_SET(status, (HOSTA_DVER+HOSTA_BSER+HOSTA_FAIL+HOSTA_NACK+HOSTA_TMOE)))
            {
                if(IS_MASK_SET(status, (HOSTA_BSER+HOSTA_FAIL)))
                {
                    ResetSMBus(Channel);
                }
                error = 0xEE;
            }
            else                                    // Only Finish Interrupt bit is set.
            {
                error = 0x00;
            }
        }
	 
	    if(error == 0xEE)
	    {
		    result = FALSE; 
	    }	
        else
        {
	        *Var = *asSMBus[Channel].SMBusData0;    // read data
	        result = TRUE; 
        }
	    *asSMBus[Channel].SMBusSTA=0xFE;	    // clear bits
    }
    
	return(result);
}

//----------------------------------------------------------------------------
// 计算CRC检验参数，检测
//----------------------------------------------------------------------------
void CalcCRC8(BYTE sourcebyte)
{
    BYTE temp;
	SMBCRC8_A ^=sourcebyte;
    temp = SMBCRC8_A;

    if(IS_MASK_CLEAR(temp, BIT7))
    {
        temp = temp<<1;
        SMBCRC8_A ^= temp;
    }
    else
    {
        temp = temp<<1;
        SMBCRC8_A ^= 0x09;
        SMBCRC8_A ^= temp;
    }
    
    if(IS_MASK_CLEAR(temp, BIT7))
    {
        temp = temp<<1;
        SMBCRC8_A ^= temp;
    }
    else
    {
        temp = temp<<1;
        SMBCRC8_A ^= 0x07;
        SMBCRC8_A ^= temp;
    }
}

//----------------------------------------------------------------------------
// The function of Checking SMbus clock and data pins are both high	
// 检测时钟线与数据线是否为高电平，表示的是是否SMBUS 总线忙
//----------------------------------------------------------------------------
BYTE CheckSMBusFree(BYTE channel)
{
    BYTE pinstatus;

    pinstatus = *asResetSMBusS[channel].SMBusPinCTL;

    if((pinstatus&=0x03)==0x03) // Check BIT0 and BIT1
    {
        return(SMBus_Free);
    }
    else
    {
        return(SMBus_Busy);
    }
}

//----------------------------------------------------------------------------
// The function of Checking SMbus pins are all alt
// 检测 SMBUS 对应引脚的功能模式
//----------------------------------------------------------------------------
BYTE CheckSMBusInterfaceIsALT(BYTE channel)
{
    if(((*asResetSMBusS[channel].SMBusPin0 & 0xFB)!=0x00)||((*asResetSMBusS[channel].SMBusPin1 & 0xFB)!=0x00))
    {
        return(SMBusPin_GPIO);
    }
    else
    {
        return(SMBusPin_ALT);
    }
}

//----------------------------------------------------------------------------
// The function of Checking SMbus pins are all alt
// 该函数功能为检查SMBUS pin 引脚能否被使用，查看是否占用
// 返回 0x01  表示能被使用，返回0x02,表示不能被使用
//----------------------------------------------------------------------------
BYTE CheckSMBusInterfaceCanbeUse(BYTE channel, BYTE accesstype)
{
    BYTE checksmbus;

    checksmbus = SMBus_CanUse;                  // Pre-set smbus interface can be used
    
    #ifdef PECI_Support 
    if(channel==SMbusCh3)                       // If PECI function enable, return.
    {
        checksmbus = SMBus_CanNotUse;           // Interface can't be used
        return(checksmbus);                     // return result
    }
    #endif
    
    // 如果设置为GPIO模式 则不能使用
    if(CheckSMBusInterfaceIsALT(channel)==SMBusPin_GPIO)    // SMBus interface is GPIO function
    {
        checksmbus = SMBus_CanNotUse;           // Interface can't be used
    }
    else                                        // SMBus interface is ALT function
    {
        // 等待SMBUS 通道被释放
        if(accesstype==SMBus_AccessType_Fucn)
        {
            #ifdef SMBusServiceCenterFunc
            WatiSMBusCHxFree(channel);          // Check smbus is in used or not.
            #endif
        }


        // 如果通道忙状态
        if(CheckSMBusFree(channel)==SMBus_Busy)
        {
            checksmbus = SMBus_CanNotUse;       // Interface can't be used
            if( (*asResetSMBusS[channel].recoverCunt)++ >SMBus_BusyRecover)
            {
                ResetSMBus(channel);
            }
        }
        else
        {
            *asResetSMBusS[channel].recoverCunt=0x00;
            checksmbus = SMBus_CanUse;          // smbus interface can be used
        }
    }

    return(checksmbus);                         // return result
}

//----------------------------------------------------------------------------
// To init. SMBus setting
//----------------------------------------------------------------------------
void Core_Init_SMBus(void)
{
    // /如果总线时钟引脚或数据引脚的低电平比SMBus_BusyRecover时间大，则重置SMBus接口。
    SMBus_BusyRecover = 20;         // Reset SMBus interface if bus clock pin or data pin is low greater than SMBus_BusyRecover times.
}

/*-----------------------------------------------------------------------------
 * @subroutine - I2C_WriteStream
 * @function - I2C Write Stream data  I2C写数据流
 * @Upstream - By call
 * @input    - None
 * @return   - None
 * @note     - None
 *---------------------------------------------------------------------------*/
BYTE I2C_WriteStream(BYTE I2C_Chn, BYTE I2C_Addr, XBYTE *Var, BYTE Count)
{
    BYTE    i, j, len;

    len = 0;
    // 使能 SMBUS 和 I2C
    *asResetSMBusS[I2C_Chn].SMBusCTL2 = 0x03;

    // 清状态
    *asSMBus[I2C_Chn].SMBusSTA = 0xFE;

    // 设置 I2C 地址
    *asSMBus[I2C_Chn].SMBusADR = (I2C_Addr & 0xFE);

    // 设置第一个传输 数据
    *asSMBus[I2C_Chn].SMBusBData = *Var;

    // 不是能主机中断机制，使用Extend Command，做包错误检测
    *asSMBus[I2C_Chn].SMBusCTL = (_SMbusEXT | HOCTL_SRT);  // 0x9C

    while(1)
    {
        // 设置上限循环操作100 次操作
        i = 100;
        while(1)
        {
            WNCKR = 0x00;   /* Delay 15.26 us */

            // 读取SMBUS状态
            j = *asSMBus[I2C_Chn].SMBusSTA;
             

            // 判断多少次操作，是否已经到达极限
            if (i > 0)
            {
                i--;
            } else {
                // 清中断处理
                *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
                return FALSE;
            }

            // 下面分别判断SMBUS 是什么状态的

            // 当主机控制器收到一个字节(对于块读命令和i2c兼容的循环)或者
            // 它完成了一个字节的传输(对于块写命令和i2c兼容的循环)时，这个位将被设置为1。
            if ((j & HOSTA_BDS) != 0x00)
            {
                break;
            }

            // 如果设置了KILL并且成功终止了处理传输，则读取该位将返回1
            // 中断源是SMBus失去仲裁
            if ((j & (HOSTA_BSER + HOSTA_FAIL)) != 0x00)
            {
                ResetSMBus(I2C_Chn);
                return FALSE;
            }

            // 超时25ms， 没有返回ACK
            if ((j & (HOSTA_DVER + HOSTA_NACK + HOSTA_TMOE)) != 0x00)
            {
                *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
                return FALSE;
            }
        }

        // 设置位置偏移，如果大于计数值，不执行发送了
        len++;
        if (len == Count)
        {
            break;
        }
        Var++;

        // 设置 subus 数据
        *asSMBus[I2C_Chn].SMBusBData = *Var;

        // 清状态位
        *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
    }


    // 数据发送完成 关闭I2C
    //  清状态
    *asResetSMBusS[I2C_Chn].SMBusCTL2 = 0x01;
    *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
    return TRUE;
}
/*-----------------------------------------------------------------------------
 * @subroutine - I2C_ReadStream
 * @function - I2C Read Stream data  I2C读数据流
 * @Upstream - By call
 * @input    - None
 * @return   - None
 * @note     - None
 *---------------------------------------------------------------------------*/
BYTE I2C_ReadStream(BYTE I2C_Chn, BYTE I2C_Addr, XBYTE *Var, BYTE Count)
{
    BYTE    i, j;

     // 使能 SMBUS 和 I2C
    *asResetSMBusS[I2C_Chn].SMBusCTL2 = 0x03;

    // 清状态
    *asSMBus[I2C_Chn].SMBusSTA = 0xFE;

    // 设置I2C地址
    *asSMBus[I2C_Chn].SMBusADR = (I2C_Addr | 0x01);

     // 不是能主机中断机制，使用Extend Command，做包错误检测
    *asSMBus[I2C_Chn].SMBusCTL = (_SMbusEXT | HOCTL_SRT);

    // 如果只进行一次读操作，设置Last Byte (LABY)
    if (Count == 0x01)
    {
        SET_MASK(*asSMBus[I2C_Chn].SMBusCTL, HOCTL_LABY);
    }


    do
    {
        i = 100;
        while(1)
        {
            WNCKR = 0x00;   /* Delay 15.26 us */

            // 判断多少次操作，是否已经到达极限
            if (i > 0)
            {
                i--;
            } else {
                // 清状态处理
                *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
                return FALSE;
            }


            // 读取中断状态
            j = *asSMBus[I2C_Chn].SMBusSTA;

            // 下面分别判断SMBUS 是什么状态的

            // 当主机控制器收到一个字节(对于块读命令和i2c兼容的循环)或者
            // 它完成了一个字节的传输(对于块写命令和i2c兼容的循环)时，这个位将被设置为1。
            if ((j & HOSTA_BDS) != 0x00)
            {
                break;
            }

            // 如果设置了KILL并且成功终止了处理传输，则读取该位将返回1
            // 中断源是SMBus失去仲裁
            if ((j & (HOSTA_BSER + HOSTA_FAIL)) != 0x00)
            {
                ResetSMBus(I2C_Chn);
                return FALSE;
            }

            // 超时25ms， 没有返回ACK
            if ((j & (HOSTA_DVER + HOSTA_NACK + HOSTA_TMOE)) != 0x00)
            {
                *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
                return FALSE;
            }
        }

        // 读取数据，将数据放入buffer
        *Var = *asSMBus[I2C_Chn].SMBusBData;
        Count--;

        // 如果只有最后一个数据，设置last byte
        if (Count == 0x01)
        {
            SET_MASK(*asSMBus[I2C_Chn].SMBusCTL, HOCTL_LABY);
        }
        Var++;

        // 清状态处理
        *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
    } while (Count > 0);

    // 关闭I2C,清状态操作
    *asResetSMBusS[I2C_Chn].SMBusCTL2 = 0x01;
    *asSMBus[I2C_Chn].SMBusSTA = 0xFE;
    CLEAR_MASK(*asSMBus[I2C_Chn].SMBusCTL, HOCTL_LABY);
    return TRUE;
}

