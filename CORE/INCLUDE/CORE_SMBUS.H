/*-----------------------------------------------------------------------------
 * TITLE: CORE_SMBUS.H
 *---------------------------------------------------------------------------*/

#ifndef CORE_SMBUS_H
#define CORE_SMBUS_H

// SMBUS 通道
#define SMbusCh1	    0x00	// SMbus channel A index
#define SMbusCh2	    0x01	// SMbus channel B index
#define SMbusCh3	    0x02	// SMbus channel C index
#define SMbusCh4        0x03	// SMbus channel D index

// SMBUS 指令
#define	SMbusWQ		    0x80	// Write Quick command
#define	SMbusRQ		    0x00	// Read Quick command
#define	SMbusSBC	    0x84	// Send Byte command
#define	SMbusRBC	    0x04	// Receive Byte command
#define	SMbusWB		    0x88	// Write byte command
#define SMbusRB		    0x08	// Read byte command
#define	SMbusWW 	    0x8C	// Write word command
#define	SMbusRW		    0x0C	// Read  word command
#define	SMbusWBK	    0x94	// Write block command
#define	SMbusRBK	    0x14	// Read block command

// SMBUS 引脚状态
#define SMBus_Busy      0x01    // SMBus clock pin or data pin is low
#define SMBus_Free      0x00    // SMBus clock pin and data pin are both high

// SMBUS 引脚功能
#define SMBusPin_GPIO   0x00    // SMBus interface is GPIO function
#define SMBusPin_ALT    0x01    // SMBus interface is ALT function

// SMBUS 能否被使用的flag 
#define SMBus_CanUse    0x01    // SMBus interface can be used.
#define SMBus_CanNotUse 0x00    // SMBus interface can't be used.


// SMBUS 访问类型
#define SMBus_AccessType_Fucn   0x01   // OS  level
#define SMBus_AccessType_Table  0x00   // OEM level

// SMBUS 服务标志位
#define SMBus_NeedService   0x01
#define SMBus_ClearService  0x00

// SMMBUS 是否需要包错误检查（CRC8）
#define SMBus_NeedPEC   0x01
#define SMBus_NoPEC     0x00

extern void ResetSMBus(BYTE Channel);
extern BYTE bRWSMBus(BYTE Channel,BYTE Protocol,BYTE Addr,BYTE Comd,XBYTE *Var,BYTE PECSupport);
extern BYTE bRSMBusBlock(BYTE Channel,BYTE Protocol,BYTE Addr,BYTE Comd,XBYTE *Var);
extern BYTE bWSMBusBlock(BYTE Channel,BYTE Protocol,BYTE Addr,BYTE Comd,XBYTE *Var,BYTE ByteCont,BYTE PECsupport);
extern BYTE bSMBusSendByte(BYTE Channel,BYTE Addr,BYTE SData);
extern BYTE bSMBusReceiveByte(BYTE Channel,BYTE Addr,XBYTE *Var);
extern void CalcCRC8(BYTE sourcebyte);
extern BYTE CheckSMBusFree(BYTE channel);
extern BYTE CheckSMBusInterfaceIsALT(BYTE channel);
extern BYTE CheckSMBusInterfaceCanbeUse(BYTE channel, BYTE accesstype);
extern void Core_Init_SMBus(void);
extern BYTE I2C_ReadStream(BYTE I2C_Chn, BYTE I2C_Addr, XBYTE *Var, BYTE Count);
extern BYTE I2C_WriteStream(BYTE I2C_Chn, BYTE I2C_Addr, XBYTE *Var, BYTE Count);

typedef struct SMBusStruct
{
	XBYTE	*SMBusCTL	;  // HOCTL_A      0x1C01    	Host Control Register (HOCTL)	 
	XBYTE	*SMBusADR	;  // TRASLA_A     0x1C03    	Transmit Slave Address Register (TRASLA
	XBYTE	*SMBusCMD	;  // HOCMD_A      0x1C02    	Host Command Register (HOCMD)
	XBYTE	*SMBusSTA	;  // HOSTA_A      0x1C00    	Host Status Register (HOSTA
	XBYTE	*SMBusData0	;  // D0REG_A      0x1C04    	Data 0 Register (D0REG) 
	XBYTE	*SMBusData1	;  // D1REG_A      0x1C05    	Data 1 Register (D1REG)	
	XBYTE	*SMBusBData	;  // HOBDB_A      0x1C06    	Host Block Data Byte Register (HOBDB)	
	XBYTE	*SMBusIER	;  // IER1         0x1105    	            	
    XBYTE	*SMBusISR	;  // ISR1         0x1101    	        
	BYTE	IERFlag		;  // Int_SMBUS0   bit1      	        	
    XBYTE	*SMBusPEC	;  // PECERC_A     0x1C07    	Packet Error Check Register (PECERC)
} sSMBus;

typedef struct ResetSMBusS
{
	XBYTE	*SMBusSTA	; // HOSTA_A      0x1C00     Host Status Register (HOSTA
	XBYTE	*SMBusCTL2	; // HOCTL2_A     0x1C10     Host Control Register 2 (HOCTL2)
	XBYTE	*SMBusPinCTL; // SMBPCTL_A    0x1C0A     SMBus Pin Control Register (SMBPCTL)
	XBYTE	*SMBusPin0	; // GPCRB3       0x161B	    Port Control B3 Registers
	XBYTE	*SMBusPin1	; // GPCRB4		 0x161C	    Port Control B4 Registers
	XBYTE	*GPIOReg	; // GPDRB 		 0x1602	    Port B Data Register
	BYTE	GPIOData	; // (BIT3+BIT4)  应该是 GPB3,GPB4
    XBYTE   *recoverCunt; // SMBus1RecoverCunt
} sResetSMBusS;

extern sSMBus code asSMBus[];

#endif 
