#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>


void uart_Initial_Host(void)
{
	BADRSEL = 0x01;
  //  RSTDMMC = 0x00;
	GCR1 = 0x05;
	GPCRB0 = ALT;
	GPCRB1 = ALT;
}


void uart_Initial(void)
{
#if UART_Debug

	SCON=0x40;         // Mode 1
	SCON |= BIT4;      // Receive data enable
    T2CON &= 0xF0;     // EXEN2=0; TR2=0; C/T2#=0; CP/RL2#=0;
    T2CON |= 0x30;     // RCLK = 1; TCLK=1;     receive and send use time2
    T2MOD=0x00;
    RCAP2H=R2HV;
    RCAP2L=R2LV;
    TR2=1;

	UART_RX_Buffer_In = 0;
	
	SET_MASK(UART_Buffer_Status,_buffer_Empty);   // Set buffer empty flag
	SET_MASK(UART_Buffer_Status,_Send_Ready);     // Set send ready flag
	
	SET_MASK(IE, BIT4); // Enable UART intrrupt

#endif	
}

//------------------------------------------------------
//    Send out an char by 8032 UART from buffer
// 1. Just call in the event of 1 ms
// 2. It wil send a byte by 8032 UART when the macro UART_Debug is TRUE
//------------------------------------------------------
void UART_Send_Byte(void)
{
#if UART_Debug
	unsigned char ch;

	if(IS_MASK_SET(UART_Buffer_Status,_buffer_Empty)) // The buffer is empty
	{
		return;
	}

	CLEAR_MASK(UART_Buffer_Status,_Send_Ready);  // Set busy for send data, wait for ready when send next data
	
	ch = (*(char xdata *)((&Uart_Base_Add)+UART_TX_Buffer_Out)); // Get a char from buffer
	
	UART_TX_Buffer_Out++;

	if(UART_TX_Buffer_Out == DebugUartRange)
	{
		UART_TX_Buffer_Out = 0;
	}

	CLEAR_MASK(UART_Buffer_Status,_buffer_Full);  // Clear buffer full flag

	if(UART_TX_Buffer_Out == UART_TX_Buffer_In) 
	{
		SET_MASK(UART_Buffer_Status,_buffer_Empty); // Set bufer empty flag
	}
	
	SBUF = ch; // SBUF is the Register for UART TX
	           // After the success to send a char, TI will set to 1 automatically
#endif
}


//------------------------------------------------------
//    Print out an BYTE to UART buffer
//------------------------------------------------------
void UART_Print_Byte(unsigned char ch)
{
#if UART_Debug

	if(IS_MASK_SET(UART_Buffer_Status,_buffer_Full))   // The UART buffer is full
	{
		UART_TX_Buffer_In=0x06;
		UART_TX_Buffer_Out=0x00;
		((*(XBYTE *)((&Uart_Base_Add)+0))='\n');
		((*(XBYTE *)((&Uart_Base_Add)+1))='l');
		((*(XBYTE *)((&Uart_Base_Add)+2))='o');
		((*(XBYTE *)((&Uart_Base_Add)+3))='s');
		((*(XBYTE *)((&Uart_Base_Add)+4))='t');
		((*(XBYTE *)((&Uart_Base_Add)+5))='\n');
		
		CLEAR_MASK(UART_Buffer_Status,_buffer_Empty);  // Clear buffer empty flag
	}
	else
	{
		((*(char xdata *)((&Uart_Base_Add)+UART_TX_Buffer_In))=ch); // Put the char to buffer
		
		UART_TX_Buffer_In++;
		
		if(UART_TX_Buffer_In == DebugUartRange)
		{
			UART_TX_Buffer_In = 0;
		}
		
		CLEAR_MASK(UART_Buffer_Status,_buffer_Empty);  // Clear buffer empty flag
		
		if(UART_TX_Buffer_In == UART_TX_Buffer_Out)
		{
			SET_MASK(UART_Buffer_Status,_buffer_Full); // Set bufer full flag
		}
	}
	
#endif
}

//------------------------------------------------------
//    Print HEX to UART buffer
// 1. The HEX data rang is 0--FF
//------------------------------------------------------
void UART_Print_HEX(unsigned char HEX_Dat)
{
#if UART_Debug

	unsigned char ch;
	ch = HEX_Dat >> 4;
	if(ch <= 9)
	{
		ch = ch + '0';
	}
    else
    {
		ch = ch + 'A' - 10;
    }
    UART_Print_Byte(ch);

	ch = HEX_Dat & 0x0F;
	if(ch <= 9)
	{
		ch = ch + '0';
	}
    else
    {
		ch = ch + 'A' - 10;
    }
    UART_Print_Byte(ch);
	
#endif
}

//------------------------------------------------------
//    Print integer to UART buffer
//------------------------------------------------------
void UART_Print_Int(unsigned int aInt)
{
#if UART_Debug

	unsigned char temp;
	
	temp = aInt / 10000;
	if (temp != 0)
	{
		 UART_Print_Byte(temp + '0');
	}

	temp = aInt % 10000 / 1000;
	if (temp != 0)
	{
		UART_Print_Byte(temp + '0');
	}

	temp = aInt % 1000 / 100;
	if (temp != 0)
	{
		UART_Print_Byte(temp + '0');
	}

	temp = aInt % 100 / 10;
	if (temp != 0)
	{
		UART_Print_Byte(temp + '0');
	}
	
   	temp = aInt % 10;
	UART_Print_Byte(temp + '0');
	
#endif
}


//------------------------------------------------------
//    Print  an string to UART buffer
//  1. If the string has '\n', don't care, the receive software will parse
//------------------------------------------------------
void UART_Print_Str(unsigned char *str)
{
#if UART_Debug

	unsigned char i;
	unsigned char j;
	unsigned char ch;

	for (i = 0; i < Str_MAX_Len; i++)
	{
		ch = *(str + i);

		if (ch == '\0')  // String end
		{
			break;
		}
		else
		{
			UART_Print_Byte(ch);
		}
	}

#endif
}




void Cmd_Hello(void)
{
	UART_Print_Str("This is CF03-T say hello\n");
}

void Cmd_Battery(void)
{
	UART_Print_Str("This is CF03-T say Battery\n");
}

void Cmd_Charger(void)
{
	UART_Print_Str("This is CF03-T say Charger\n");
}

const UART_CMD code UART_Cmd_Table[] = {
		{"Hello",   Cmd_Hello    },
		{"Battery", Cmd_Battery  },
		{"Charger", Cmd_Charger  },
		{0,0},
};
/*
//------------------------------------------------------
//    Processing of 8032 UART receiving commands
//  1. The command must be end of '\r' & '\n', 0x0D  0x0A
//  2. This function must be call in S0/S3/S4/S5
//------------------------------------------------------
void UART_RX_Command(void)
{
	unsigned char i;
	unsigned char j;
	unsigned char k;
	unsigned char Rx_index;

	Rx_index = UART_RX_Buffer_In;
	if(Rx_index)
	{
		for(i=0; i<Rx_index; i++)
		{
			if(0x0D == UART_RX_Buffer[i])  // 0x0D is the char '\r'
			{
				break;
			}
		}

		if(i<Rx_index)  // Receives the command
		{
			for(j=0; 0!=(UART_Cmd_Table[j].CmdFun); j++)  // Search command
			{
				for(k=0; (0x0D!=UART_RX_Buffer[k])&&(0!=(UART_Cmd_Table[j].CmdName[k])); k++)  // Rx command end of '\r'
				{
					if(UART_Cmd_Table[j].CmdName[k] == UART_RX_Buffer[k]) // compare command
					{
						continue;
					}
					else
					{
						break;
					}
				}

				if(0x0D==UART_RX_Buffer[k])  // There is match command
				{
					(UART_Cmd_Table[j].CmdFun)();
					UART_RX_Buffer_In = 0;
					break;
				}
			}

			if(0==(UART_Cmd_Table[j].CmdFun))  // There is no command match
			{
				UART_Print_Str("Command : ");
				for(i=0; i<Rx_index; i++)
				{
					UART_Print_Byte(UART_RX_Buffer[i]);
				}
				UART_Print_Str("\nThis command is not support!\n");
				UART_RX_Buffer_In = 0;
			}
		}

		if(i==(UART_RX_MAX_LEN-1))  // Rx Buffer overflow
		{
			UART_Print_Str("The Rx buffer is overflow and it's self-reset !!!\n");
			UART_RX_Buffer_In = 0;
		}
	}
}
*/


