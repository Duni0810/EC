/*-----------------------------------------------------------------------------
 * TITLE: OEM_BATTERY.H
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/

#ifndef BATTERY_H
#define BATTERY_H

//-------------------------------------------------------------------------------
// Battery in ADC rang
//-------------------------------------------------------------------------------
#define BAT_IN_H			    827     // high
#define BAT_IN_L			    168     // low

//-------------------------------------------------------------------------------
// Define SMBus device address and channel
//-------------------------------------------------------------------------------
#define SmartBat_Addr	        0x16    // Smart battery address
#define Charger_Addr		    0x12    // Smart charger address
#define SmartBatChannel         SMbusCh1
#define SmartChargerChannel     SMbusCh1

//-------------------------------------------------------------------------------
// Define smart battery command
//-------------------------------------------------------------------------------
#define BATCmd_access           0x00  	// manufacture access
#define BATCmd_RCalarm          0x01 	// remaining capacity alarm
#define BATCmd_RTalarm          0x02   	// remaining time alarm
#define BATCmd_mode             0x03  	// battery mode
#define BATCmd_AtRate           0x04   	// AtRate
#define BATCmd_AtRateFull       0x05  	// AtRate time to full
#define BATCmd_AtRateEmpty      0x06  	// AtRate time to empty
#define BATCmd_AtRateOK         0x07   	// AtRate OK
#define BATCmd_temp             0x08   	// temperature
#define BATCmd_volt             0x09   	// voltage
#define BATCmd_current          0x0A   	// current
#define BATCmd_AVcurrent        0x0B   	// average current
#define BATCmd_MaxError         0x0C  	// maximum error
#define BATCmd_RSOC             0x0D  	// relative state of charge
#define BATCmd_ASOC             0x0E   	// absolute state of charge
#define BATCmd_RMcap            0x0F   	// remaining capacity
#define BATCmd_FCcap            0x10   	// full charge capacity
#define BATCmd_RunTimeEmpty     0x11 	// run time to empty
#define BATCmd_AVTimeEmpty      0x12  	// average time to empty
#define BATCmd_AVTimeFull       0x13   	// average time to full
#define BATCmd_CC               0x14  	// charging current
#define BATCmd_CV               0x15   	// charging voltage
#define BATCmd_BatStatus        0x16   	// battery status
#define BATCmd_CycleCount       0x17   	// cycle count
#define BATCmd_DCap             0x18   	// design capacity
#define BATCmd_DVolt            0x19   	// design voltage
#define BATCmd_Spec             0x1A 	// spec information
#define BATCmd_Date             0x1B   	// manufacture date
#define BATCmd_SerialNo         0x1C   	// serial number
#define BATCmd_Mname            0x20 	// manufacturer name
#define BATCmd_Dname            0x21  	// device name
#define BATCmd_Dchem            0x22  	// device chemistry
#define BATCmd_Mdata            0x23 	// manufacture data

#define BATCmd_SHA1W			0x27    //sending challenge from host EC to battery BMU
#define BATCmd_SHA1R			0x28    //receiving response from battery BMU to host EC

#define BATCmd_BatFW		0x37	// Battery firmware verion


#define BATCmd_MfgF4			0x3C    //OptionalMfgFunction4
#define BATCmd_MfgF2			0x3E    //OptionalMfgFunction2

//-------------------------------------------------------------------------------
// Charge and discharge control step define
//-------------------------------------------------------------------------------
#define BAT_Step_ID	            0x01    // Identify main battery
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define BATID_Step_GetDV        0x01    // First time to get battery design voltage and checking SMBus communication is OK
#define BATID_Step_Auth         0x02    // Battery Authentication if necessary
#define BATID_Step_MFName       0x03    // Get manufacturer name and checking battery is supported
#define BATID_Step_FirstData    0x04    // Polling first data

#define BATID_Step_InitOK       0x05    // Init_OK,prepare Discharge/Pre-Charge. (always at last step)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define BAT_Step_WC			    0x02    // Battery wake up charge
#define BAT_Step_PC		        0x03    // Battery pre-charge
#define BAT_Step_NC		        0x04    // Battery normal charge
#define BAT_Step_DC			    0x05    // Battery discharge

#define BAT_Step_FullyChg       0x10    // Battery fully charged
#define BAT_Step_FullyDisChg    0x11    // Battery fully discharged

#define BAT_Step_SetFail        0x20    // Battery will fail
#define BAT_Step_Fail		    0x21    // Battery abnormal

#define BAT_Step_ForceDC        0x30    // Battery force discharge
#define BAT_Step_ForceC         0x31    // Battery force charge
#define BAT_Step_AutoLN         0x32    // Battery auot learning

#define BAT_Step_ChargerFail    0x40    // Smart charger abnormal

#define BAT_Step_Out		    0x50    // Battery plug out

#if Lenovo_Support
#define BAT_Step_keep		0x60    // Battery keep
#endif 


//-------------------------------------------------------------------------------
// Define smart charger command
// ISL9237
//-------------------------------------------------------------------------------
#define C_ChargeCurrent 	    0x14    // Set charge current
#define C_ChargeVoltage		    0x15    // Set charge voltage  MaxSystemVoltage
#define C_Control0				0x39
#define C_Control1				0x3C
#define C_Control2				0x3D

#define C_Information			0x3A
#define C_MinSystemVoltage		0x3E    // 
#define C_InputCurrent		    0x3F    // Set input current  AdapterCurrentLimit1
#define C_ACProchot				0x47	// Set AC procthot 
#define C_DCProchot				0x48	// Set DC prochot
#define C_OTGVoltage			0x49
#define C_OTGCurrent			0x4A
#define C_MFID				    0xFE	// Get Manufacturer ID
#define C_DeviceID			    0xFF	// Get device ID

#define CHARGER_LEARNMODE		BIT6    // 1: Enalbe learn mode     0: disable learn mode (default)

#define ChargeDeviceID		    0x0008
#define ChargeMFID			    0x004D

//-------------------------------------------------------------------------------
// Define smart charger command
// bq24780
//-------------------------------------------------------------------------------
#define _CMD_ChargerOption0          	0x12
#define _CMD_ChargerCurrent             0x14
#define _CMD_ChargerVoltage				0x15
#define _CMD_ChargerOption1          	0x3B
#define _CMD_ChargerOption2          	0x38
#define _CMD_ChargerOption3          	0x37
#define _CMD_ProchotOption0          	0x3C
#define _CMD_ProchotOption1          	0x3D
#define _CMD_ProchotStatus				0x3A //Read
#define _CMD_DischargeCurrent        	0x39
#define _CMD_ChargerMinSysVoltage   	0x3E
#define _CMD_ChargerInputCurrent    	0x3F
#define _CMD_ChargerManufacturerID  	0xFE //Read
#define _CMD_ChargerDeviceID        	0xFF //Read

//-------------------------------------------------------------------------------
// Smart charger
//-------------------------------------------------------------------------------
#define SC_Current_Sense        1

#define BAT1_VOLT_WUCHG         13050
#define BAT1_CURR_WUCHG	        (128*SC_Current_Sense)

#define BAT1_VOLT_PRECHG        (4200*BAT1_S_Number)
#define BAT1_CURR_PRECHG        (256*SC_Current_Sense)

#define Bat1_PC2NC_Cap          256    

#define I_20W   			    (1024)
#define I_25W                   (1280)
#define I_30W				    (1536)
#define I_45W   			    (2048)
#define I_65W   			    (3072)
#define I_90W   			    (4096)
#define I_120W                  (6016) 
#define I_130W  			    (6144)                
#define I_150W  			    (7168)
#define Iput_Current_3250A		0x0D00
#define Iput_Current_3000A		0x0BB8
#define Iput_Current_2500A		0x09C4

//J80_015--	#define InputCurrent            Iput_Current_2500A	  //50W
#define InputCurrent            Iput_Current_3250A	//J80_015++
//---------------------------------------------------------------------------------
//AdapterProtect_Support
//---------------------------------------------------------------------------------
#define ACPowerWatts             120 //65      // Set adaptor 65w  
#define SYSTEM_POWER_Watts_Turbo 18      // set  system turbo power  5W
#define SYSTEM_POWER_MAX         (ACPowerWatts - SYSTEM_POWER_Watts_Turbo) // 
#define SYSTEM_POWER_Bat_ReChr   (SYSTEM_POWER_MAX - 5) //  the power  point of Battery recharge  temp_guoyong2 //
#define SYSTEM_POWER_ME_CTRL     50      // in this point , bios post p-status control to ME 
#define POWER_OVER_TIMES         3
#define POWER_RESUME_TIMES       3

#define SYSTEM_POWER_Protect      100 
#define SYSTEM_POWER_BATProtect   8000 
#define SYSTEM_POWER_Protect_TIMES    3


//#define POWER_OVER_FORCPU_TIMES  3
#define SYSTEM_POWER_BatChr_Step 100         // control current step: 100mA

#define SYSTEM_POWER_BATOnly_MAXC  4000      // 4A discharge current   temp_guoyong3 //

#define SYSTEM_POWER_BATOnly_CPUDown_R  3000 // // 3.5A discharge current   cpu  resume full frequence   temp_guoyong4//


//-------------------------------------------------------------------------------
// Main battery protect
//-------------------------------------------------------------------------------
#define BAT1_Low			    10      // RSOC 10%
#define BAT2_Low			    10      // RSOC 10%

#define BAT1_CriticalLow	    3       // RSOC 3%
#define BAT2_CriticalLow	    3       // RSOC 3%

#define BATOverVoltage		    (4425*BAT1_S_Number)

#define ChargerSettingTimeOut   3       // re-try counter of setting of smart charger
#define BatIDTimeOut            5       // battery 1 identify re-try counter
#define BatWCTimeOut            60      // timer base is 1min (1hr)
#define BatPCTimeOut            120     // timer base is 1min (2hr)
#define BatNCTimeOut            480     // timer base is 1min (8hr)

#define BatOVTimeOut            5       // Main battery over voltage     5sec
#define BatOTTimeOut            10      // Main battery over temperature
#define BatBadTimeOut           10      // Main battery is bad   10sec

#define KELVIN_CONSTANT	2732

#define CHARGE_INHIBIT_TEMP_LOW 0
#define CHARGE_INHIBIT_TEMP_LOW_KELVIN 	(CHARGE_INHIBIT_TEMP_LOW  * 10 + KELVIN_CONSTANT)
#define CHARGE_RESUME_TEMP_LOW_KELVIN (CHARGE_INHIBIT_TEMP_LOW_KELVIN+ 5 * 10)     //5 

#define CHARGE_INHIBIT_TEMP_HIGH  60 //4
#define CHARGE_INHIBIT_TEMP_HIGH_KELVIN (CHARGE_INHIBIT_TEMP_HIGH * 10 + KELVIN_CONSTANT)
#define CHARGE_RESUME_TEMP_HIGH_KELVIN (CHARGE_INHIBIT_TEMP_HIGH_KELVIN- 5 * 10)   //55


#define CHARGE_INHIBIT_TEMP_LOW_KELVIN_INITIAL  KELVIN_CONSTANT
#define CHARGE_RESUME_TEMP_LOW_KELVIN_INITIAL  (KELVIN_CONSTANT + 5*10)

#define CHARGE_INHIBIT_TEMP_HIGH_KELVIN_INITIAL (500 + KELVIN_CONSTANT)
#define CHARGE_RESUME_TEMP_HIGH_KELVIN_INITIAL  (450 + KELVIN_CONSTANT)

//-------------------------------------------------------------------------------
// Battery Fail cause
//-------------------------------------------------------------------------------
#define Bat_WakeUpChargeFail    0x01
#define Bat_PreChargeFail       0x02
#define Bat_NormalChargeFail    0x03
#define Bat_Weak                0x04
#define Bat_OverVoltage         0x05
#define Bat_OverTemperature     0x06
#define Bat_NoSupport           0x07
#define Bat_AuthFail            0x08

//-------------------------------------------------------------------------------
// Battery Stop charge cause
//-------------------------------------------------------------------------------
#if Sha1Support
extern void InitCreatRandomData(void); 	// 
extern void Sha1HashProcess(void);
extern BYTE Sha1WriteChallengeValueToBat1(void);
extern BYTE Sha1ReadResponseFromBat1(void);
extern BYTE ReadBat1BlockTest(void);
extern BYTE CompareAuthentication(void);
extern BYTE ProcessAuthentication(void);
extern BYTE Sha1Authentication(void);
#endif

//-------------------------------------------------------------------------------
// Functions prototype
//-------------------------------------------------------------------------------
extern void Battey1ControlCenter(void);
extern void SetBAT1CtrlStep(BYTE ctrlstep);
extern void BAT1IdentifyNextStep(void);
extern void SetBAT1IDTimeOutCNT(void);
extern void BAT1_SetFail(BYTE failcause);
extern void BAT1_ClrVariables(void);
extern void BAT1_PlugOutClrVariables(void);
extern void Adapter_PlugOutClrVariables(void);
extern void Hook_BAT1_ChargePinON(BYTE on_off);
extern void Battey1FrimUp(void);
extern BYTE DisableCharger(void);
extern BYTE SetSmartCharger(void);
extern BYTE PollingChargerIC(void);
extern void PollingBAT1Data(void);
extern void Lenovo_Battery_Health(void);
extern BYTE Battery_Auth_Lenovo(void);
extern void CheckBatShipMode(void);

typedef struct batINFO
{
	BYTE DeviceNO;
	BYTE DeviceName[12];
}sbatINFO;

typedef struct batFirstData
{
	BYTE Cmd;
	XBYTE *Var;
}sbatFirstData;

#endif
