/*-----------------------------------------------------------------------------
 * TITLE: OEM_FAN.H
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/

#ifndef OEM_FAN_H
#define OEM_FAN_H

//-----------------------------------------------------------------------------
// Define Smbus device address
//-----------------------------------------------------------------------------
#define ThermalAddr	        0x98	// thermal sensor address
#define PCH_Address         0x96    // PCH address

//-----------------------------------------------------------------------------
// Define thermal sensor commands
//-----------------------------------------------------------------------------
#define LocalCmd		    0x00	// Read Local temprature
#define RemoteCmd	        0x01	// Read remote temprature
#define ReadStatsuCmd       0x02	// Read thermal status

#define CPURemoteOVTCmd	    0x19	// HW shutdown command 
#define CPULocalOVTCmd	    0x20	// HW shutdown command
#define CPURACmd            0x0D    // Stores the integer portion of the high limit for the External Diode
#define CPULACmd            0x0B    // Stores the 8-bit high limit for the Internal Diode
#define CPURALCmd           0x0E    // Stores the integer portion of the low limit for the External Diode
#define CPULALCmd           0x0C    // Stores the 8-bit low limit for the Internal Diode
#define ManifactorID        0xFE

#define Configuration       0x09
#define ThermHysteresis     0x21
#define ConsecutiveAlert    0x22
#define DiodeFactor         0x27
#define DiodeFactor2        0x28
#define FilterControl       0x40
#define AlertConfig         0xBF

//-----------------------------------------------------------------------------
// For SMSC thermal sensor setting
//-----------------------------------------------------------------------------
#define SMSC_ID             0x5D
#define SMSC_CPU_Local_OVT  0x78            
#define SMSC_CPU_Remote_OVT 0x78   
#define SMSC_CPURAValue     0x78    
#define SMSC_CPULAValue     0x5A   

#define GMT_ID              0x47
#define GMT_CPU_Local_OVT   0x78          
#define GMT_CPU_Remote_OVT  0x78  
#define GMT_CPURAValue      0x78
#define GMT_CPULAValue      0x5A
#define GMT_CPURALValue     0xC9
#define GMT_CPULALValue     0xC9

#define NS_ID               0x01
#define NS_CPU_Local_OVT    0x78            
#define NS_CPU_Remote_OVT   0x78   
#define NS_CPURAValue       0x78
#define NS_CPULAValue       0x5A
#define NS_CPURALValue      0xC9
#define NS_CPULALValue      0xC9

//-----------------------------------------------------------------------------
// PCH Thermal Command
//-----------------------------------------------------------------------------
#define WriteSTSPreferences     0x41
#define WriteCPUTempLimits      0x42
#define WriteMCHTempLimits      0x43
#define WritePCHTempLimits      0x44
#define WriteDIMMTempLimits     0x45
#define WriteMPCCPUPowerClamp   0x50
#define PCHBlockRead            0x40

//-----------------------------------------------------------------------------
// Fan control define
//-----------------------------------------------------------------------------
#define FanValue		    Fan_volt    // The register of voltage output. (DAC or PWM)
#define Fan2Value		    Fan2_volt
#define FanInitValue	    0x22        // 3000RPM
#define S5S0FanInitValue	0x22
#define S4S0FanInitValue	0x22
#define S3S0FanInitValue	0x22
#define	MaxOutValue			0x32        // 5000RPM

#define FullOnCounter	    20		    // time base 100ms
#define S5S0FullOnCounter	100		    // time base 100ms   10s
#define S4S0FullOnCounter	100		    // time base 100ms   10s
#define S3S0FullOnCounter	20		    // time base 100ms   2s

#define FanDOSValue	    0x22			//dos fan speed
#define FanHighValue	    0x32			//High fan speed

 

#define RPMACC			    100		    // +- 100 rpm ACC
#define RPM1Max			    7000	    // rpm max value for softwave filter

//-----------------------------------------------------------------------------
// Compute R.P.M to TACH value
//-----------------------------------------------------------------------------
//	Fan Speed(R.P.M) = 60 /((1/Fs) * Tach * P )
//	Fs = FreqEC(9.2MHz) / 128
//	P denotes the numbers of square pulses per revolution.
//	(60 * 9.2M/128  = 4312500
//-----------------------------------------------------------------------------
#define TACH1P		        2			// the numbers of square pulses per revolution.
#define TACH2P		        2			// the numbers of square pulses per revolution.

//-----------------------------------------------------------------------------
// Moniter temperature
//-----------------------------------------------------------------------------
#define CpuTooHot 		    86
#define CpuTooHotTimes 	    3

#define PCH_FanHigh_ON		90
#define PCH_FanHigh_OFF     80

#define PCHTooHot		    105
#define PCHTooHotTimes	    3

#define SysTooHot		    80
#define SysTooHotTimes	    5

#define ProchotOnTM	        87
#define ProchotOffTM		77
#define ShutdownTM			89

#define EMC1412_Addr		0x98    // temperature sensor
#define InternalTm			0x00    //
#define ExternalTm			0x01    //

//-----------------------------------------------------------------------------
// Function prototype
//-----------------------------------------------------------------------------
extern void GetThermalData(void);
extern void FanControl (void);
extern void MoniterTemprature(void);
extern void InitThermalChip(void);
extern void ReadThermalID(void);
extern void FanFullOn(void);
extern void FanFullOff(void);
extern void FanInitOn(void);
extern void S5S0FanInitOn(void);
extern void S4S0FanInitOn(void);
extern void S3S0FanInitOn(void);
extern void GetRPM1(void);
extern void GetRPM2(void);
extern void InitThermalTable1(void);
extern void InitThermalTable2(void);
extern void CheckFanRPM1(void);
extern void CheckFanRPM2(void);
extern void FanManager(void);
extern void ReadCPUTemp(void);
extern void ReadSysTemp(void);
extern void Clear_Fan_Debug(void);

//-----------------------------------------------------------------------------
// Structure define
//-----------------------------------------------------------------------------
typedef struct thermalstruct
{
	BYTE 	Level;
	BYTE 	CFanOn;
	BYTE 	CFanOff;
	BYTE	CRPM;
	BYTE	IsLinear;	//:Linear
	XBYTE	*FanOn;
	XBYTE	*FanOff;
	XBYTE	*RPM;
}thermal;

typedef	struct	TooHotStruct
{
	XBYTE	*Temprature;
	BYTE	Limit;
	XBYTE	*Counter;
	BYTE	Times;
}sTooHot;

typedef struct ThermalInitTablt
{
	BYTE	cmd	;	        // thermal init. command
	BYTE	value	;       // value of command	
	BYTE	RorW	;       // 0: read byte, 1: write byte
} sThermalInitTablt;

extern sThermalInitTablt code asThermalInitTablt[];
#endif

