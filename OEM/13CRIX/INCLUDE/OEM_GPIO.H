/*-----------------------------------------------------------------------------
 * TITLE: OEM_GPIO.H
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/
 
#ifndef OEM_GPIO_H
#define OEM_GPIO_H

//-----------------------------------------------------------------
// 
//-----------------------------------------------------------------
#define HighActive  	1
#define LowActive     	0

//-----------------------------------------------------------------
// GPIO Registers define
//-----------------------------------------------------------------
/*  General Control Register (GCR)  */
// BIT 7     -  GPIOB5 HIGH Follow LPCRST# HIGH-to-LOW
// BIT 6     -  WUI7 Enabled
// BIT 5     -  WUI6 Enabled
// BIT [2:1] -  [01] LPCRST# in GPIOB7
//              [10] LPCRST# in GPIOD2
//              [11] LPCRST# is dsiabled 
#define GCR_Init		0x06            // 
#define GCR2_Init		0x00			//

//-----------------------------------------------------------------
// GPIO Port A Registers define  AAAAAAAAAAAAAAAAAAAAAAAAAAA
//-----------------------------------------------------------------
#define PortA_Init		0x46            // PORT Data

#define PortA0_Ctrl		OUTPUT			//  EN_VDDQ
#define PortA1_Ctrl		OUTPUT			//  Power_LED    	
#define PortA2_Ctrl		OUTPUT			//  BLT_OFF_EC
#define PortA3_Ctrl		ALT 			//  BEEP_EC
#define PortA4_Ctrl		ALT          	//  SYS_FAN_PWM2
#define PortA5_Ctrl		INPUT			//  BOARD_ID1
#define PortA6_Ctrl		OUTPUT			//  CAPSLOCK_LED_L
#define PortA7_Ctrl		ALT          	//  SYS_FAN_PWM
//-----------------------------------------------------------------
#define VDD_EN_ON()				SET_MASK(GPDRA,BIT(0))
#define VDD_EN_OFF()			CLEAR_MASK(GPDRA,BIT(0))

#define PWR_LED_ON()			CLEAR_MASK(GPDRA,BIT(1))
#define PWR_LED_OFF()			SET_MASK(GPDRA,BIT(1))

#define LCD_BLT_OFF()			CLEAR_MASK(GPDRA,BIT(2)) //Low to turn off dispaly
#define LCD_BLT_ON()			SET_MASK(GPDRA,BIT(2)) 

//TF_008A++>>
#define IS_BOARD_ID1_HI()    	IS_MASK_SET(GPDRA,BIT(5))
#define IS_BOARD_ID1_LO()    	IS_MASK_CLEAR(GPDRA,BIT(5))
//TF_008A++<<

#define CAPLED_ON()				CLEAR_MASK(GPDRA,BIT(6))
#define CAPLED_OFF()			SET_MASK(GPDRA,BIT(6))
#define IS_CAPLED_ON()			IS_MASK_CLEAR(GPDRA,BIT(6))

//-----------------------------------------------------------------
// GPIO Port B Registers define  BBBBBBBBBBBBBBBBBBBBBBBBBB
//-----------------------------------------------------------------
#define PortB_Init		0x82            // PORT Data

#define PortB0_Ctrl		OUTPUT			// TPS2546_CTL1    S3 S5 low, S0 when ZX200_S0_PG high, then pull high
#define PortB1_Ctrl		OUTPUT			// FUSB3_PWREN
#define PortB2_Ctrl		OUTPUT			// BL_ENABLE_EC
#define PortB3_Ctrl		ALT			    // CLK_SMB0_BAT
#define PortB4_Ctrl		ALT			    // DAT_SMB0_BAT
#define PortB5_Ctrl		OUTPUT			// EC_GA20
#define PortB6_Ctrl		INPUT			// VDDQ_PG
#define PortB7_Ctrl		OUTPUT			// LCDVDD_ECEN
//-----------------------------------------------------------------------------
#define TPS2546_CTL1_HI()  		SET_MASK(GPDRB,BIT(0))
#define TPS2546_CTL1_LO()  		CLEAR_MASK(GPDRB,BIT(0))

#define FUSB3_PWREN_HI()		SET_MASK(GPDRB,BIT(1))
#define FUSB3_PWREN_LO()		CLEAR_MASK(GPDRB,BIT(1))

#define BL_ENABLE_EC_HI()		SET_MASK(GPDRB,BIT(2))
#define BL_ENABLE_EC_LO()		CLEAR_MASK(GPDRB,BIT(2))

#define GATEA20_ON()			SET_MASK(GPDRB,BIT(5))
#define GATEA20_OFF()			CLEAR_MASK(GPDRB,BIT(5))

#define Read_VDDQ_PG()			IS_MASK_SET(GPDRB,BIT(6))

//-----------------------------------------------------------------
// GPIO Port C Registers define   CCCCCCCCCCCCCCCCCCCC
//-----------------------------------------------------------------
#define PortC_Init		0x00            // PORT Data

//TF_010--	#define PortC0_Ctrl		INPUT			// EC_PWR_CTR1
#define PortC0_Ctrl		OUTPUT			// EC_PWR_CTR1	//TF_010++
#define PortC1_Ctrl		ALT				// I2C_CLK   		CLK_THER_CPU
#define PortC2_Ctrl		ALT				// I2C_DATA  		CLK_THER_CPU
#define PortC3_Ctrl		INPUT 			// I2C_DP_INT_TYPEC
#define PortC4_Ctrl		INPUT	        // ECPROCHOT_L
#define PortC5_Ctrl		OUTPUT	        // CAM_PWR_EN
#define PortC6_Ctrl		OUTPUT  		// BAT_LED2_OR
#define PortC7_Ctrl		OUTPUT			// WIFI_ENABLE
//-----------------------------------------------------------------------------
//TF_010++>>
#define EC_PWR_CTR1_ON()    		SET_MASK(GPDRC,BIT(0))
#define EC_PWR_CTR1_OFF()   		CLEAR_MASK(GPDRC,BIT(0))
//TF_010++<<

#define IS_USBPD1_INT_N_HI()    IS_MASK_SET(GPDRC,BIT(3))
#define IS_USBPD1_INT_N_LO()    IS_MASK_CLEAR(GPDRC,BIT(3))

#define IS_ECPROCHOT()     		IS_MASK_CLEAR(GPDRC,BIT(4))

#define CAM_PWR_EN_ON()    		SET_MASK(GPDRC,BIT(5))
#define CAM_PWR_EN_OFF()   		CLEAR_MASK(GPDRC,BIT(5))

#define BAT_LED2_ON()    		SET_MASK(GPDRC,BIT(6))
#define BAT_LED2_OFF()   		CLEAR_MASK(GPDRC,BIT(6))

#define WIFI_ENABLE_ON()		SET_MASK(GPDRC,BIT(7))
#define WIFI_ENABLE_OFF()		CLEAR_MASK(GPDRC,BIT(7))

//-----------------------------------------------------------------
// GPIO Port D Registers define  DDDDDDDDDDDDDDDDDDD
//-----------------------------------------------------------------
#define PortD_Init		0x01            // PORT Data

#define PortD0_Ctrl		OUTPUT			// EC_DRAM_CTRL
#define PortD1_Ctrl		OUTPUT			// CPU_POR        
#define PortD2_Ctrl		ALT			    // BUF_PLT_RST#
#define PortD3_Ctrl		ALT				// EC_SCI	
#define PortD4_Ctrl		OUTPUT			// EC_SMI
#define PortD5_Ctrl		INPUT			// PEXWAKE_L         Detect low active
#define PortD6_Ctrl		ALT 			// SYS_FAN_TACH
#define PortD7_Ctrl		ALT      	    // SYS_FAN_TACH2
//-----------------------------------------------------------------------------
#define DRAM_CTRL_HI()		  	SET_MASK(GPDRD,BIT(0))
#define DRAM_CTRL_LO()		 	CLEAR_MASK(GPDRD,BIT(0))

#define CPU_RST_HI()		  	SET_MASK(GPDRD,BIT(1))
#define CPU_RST_LO()		 	CLEAR_MASK(GPDRD,BIT(1))

#define SMI_ON()              	SET_MASK(GPDRD, BIT(3))
#define SMI_OFF()             	CLEAR_MASK(GPDRD, BIT(3))

#define SCI_ON()              	SET_MASK(GPDRD, BIT(4))
#define SCI_OFF()             	CLEAR_MASK(GPDRD, BIT(4))

#define IS_PEXWAKE_HI()    		IS_MASK_SET(GPDRD,BIT(5))
#define IS_PEXWAKE_LO()    		IS_MASK_CLEAR(GPDRD,BIT(5))

//-----------------------------------------------------------------
// GPIO Port E Registers define  EEEEEEEEEEEEEEEEEEEEEEEEE
//-----------------------------------------------------------------
#define PortE_Init		0x00            // PORT Data

#define PortE0_Ctrl		INPUT			// VCORE_PG
#define PortE1_Ctrl		OUTPUT			// EN_1.8VS0
#define PortE2_Ctrl		INPUT			// 1.8VS0_PG
#define PortE3_Ctrl		OUTPUT			// PWR_ON 
#define PortE4_Ctrl		INPUT			// PWRSW_R
#define PortE5_Ctrl		OUTPUT			// EC_PCIRST3_N
#define PortE6_Ctrl		INPUT			// ALLSYS_PG
#define PortE7_Ctrl		INPUT			// LID_INT_N
//-----------------------------------------------------------------------------
#define IS_VCORE_OK()    	    IS_MASK_SET(GPDRE,BIT(0))

#define V1P8_EN_HI()            SET_MASK(GPDRE,BIT(1))
#define V1P8_EN_LO()            CLEAR_MASK(GPDRE,BIT(1))

#define IS_1P8_OK()		        IS_MASK_SET(GPDRE,BIT(2))

#define PWR_ON_HI()  		    SET_MASK(GPDRE,BIT(3))
#define PWR_ON_LO()  		    CLEAR_MASK(GPDRE,BIT(3))

#define Read_NBSW()				IS_MASK_CLEAR(GPDRE,BIT(4))

#define PCIRST3_HI()		    SET_MASK(GPDRE,BIT(5))
#define PCIRST3_LO()		    CLEAR_MASK(GPDRE,BIT(5))

#define Read_ALWGD()			IS_MASK_SET(GPDRE,BIT(6))

#define Read_LID()				(IS_MASK_CLEAR(GPDRE,BIT(7))  && (!lidflag))

//-----------------------------------------------------------------
// GPIO Port F Registers define  FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
//-----------------------------------------------------------------
#define PortF_Init		0x00            // PORT Data

#define PortF0_Ctrl		ALT 		    // CLK_TP
#define PortF1_Ctrl		ALT 		    // DAT_TP
#define PortF2_Ctrl		OUTPUT			// RUSB3_PWREN1
#define PortF3_Ctrl		OUTPUT          // EN_5V3.3VS0
#define PortF4_Ctrl		OUTPUT          // EC_PCIRST1_N
#define PortF5_Ctrl		OUTPUT          // EC_PCIRST2_N
#define PortF6_Ctrl		ALT	       	    // GFX_SMBCLK
#define PortF7_Ctrl		ALT	            // GFX_SMBDATA 
//-----------------------------------------------------------------------------
#define FUSB3_PWREN1_HI()		SET_MASK(GPDRF,BIT(2))
#define FUSB3_PWREN1_LO()		CLEAR_MASK(GPDRF,BIT(2))

#define ALWON_ON()				SET_MASK(GPDRF,BIT(3))  
#define ALWON_OFF()				CLEAR_MASK(GPDRF,BIT(3))
#define IS_ALWON_HI()			IS_MASK_SET(GPDRF, BIT(3))

#define PCIRST1_LO()	    	CLEAR_MASK(GPDRF,BIT(4))
#define PCIRST1_HI()	    	SET_MASK(GPDRF,BIT(4))

#define PCIRST2_LO()	    	CLEAR_MASK(GPDRF,BIT(5))
#define PCIRST2_HI()	    	SET_MASK(GPDRF,BIT(5))

//-----------------------------------------------------------------
// GPIO Port G Registers define  GGGGGGGGGGGGGGGGGGG
//-----------------------------------------------------------------
#define PortG_Init		0x00            // PORT Data

#define PortG0_Ctrl		OUTPUT			// GPIO5_AC_DETECT
#define PortG1_Ctrl		INPUT       	// GPU_PX_EN
#define PortG2_Ctrl		OUTPUT          // FIN_PWR_EN
#define PortG3_Ctrl		INPUT           // NC
#define PortG4_Ctrl		INPUT           // NC
#define PortG5_Ctrl		INPUT           // NC
#define PortG6_Ctrl		OUTPUT			// BT_ON
#define PortG7_Ctrl		INPUT           // NC
//-----------------------------------------------------------------------------
#define FIN_PWR_EN_ON()    		SET_MASK(GPDRG,BIT(2))
#define FIN_PWR_EN_OFF()   		CLEAR_MASK(GPDRG,BIT(2))

#define BT_EN_ON()				SET_MASK(GPDRG,BIT(6))
#define BT_EN_OFF()				CLEAR_MASK(GPDRG,BIT(6))

//-----------------------------------------------------------------
// GPIO Port H Registers define  HHHHHHHHHHHHHHHHHHHHHH
//-----------------------------------------------------------------
#define PortH_Init		0x00            // PORT Data

#define PortH0_Ctrl		OUTPUT          // VCORE_EN
#define PortH1_Ctrl		OUTPUT          // PX_EN2
#define PortH2_Ctrl		INPUT			// BOARD_ID2
#define PortH3_Ctrl		OUTPUT		    // EN_VTT
#define PortH4_Ctrl		OUTPUT		    // PX_EN
#define PortH5_Ctrl		INPUT		    // STB_PWRGD
#define PortH6_Ctrl		OUTPUT		    // S5_PWR_ON
//-----------------------------------------------------------------------------
#define VCORE_EN_HI()           SET_MASK(GPDRH,BIT(0))
#define VCORE_EN_LO()           CLEAR_MASK(GPDRH,BIT(0))

#define PX_EN2_HI()           	SET_MASK(GPDRH,BIT(1))
#define PX_EN2_LO()           	CLEAR_MASK(GPDRH,BIT(1))

#define EC_VTT_EN_LO()			CLEAR_MASK(GPDRH,BIT(3))
#define EC_VTT_EN_HI()			SET_MASK(GPDRH,BIT(3))

#define PX_EN_HI()           	SET_MASK(GPDRH,BIT(4))
#define PX_EN_LO()           	CLEAR_MASK(GPDRH,BIT(4))

#define Read_STB_PWRGD()		IS_MASK_SET(GPDRH,BIT(5))

#define PWR_S5_EN_HI()			SET_MASK(GPDRH,BIT(6))
#define PWR_S5_EN_LO()			CLEAR_MASK(GPDRH,BIT(6))

//-----------------------------------------------------------------
// GPIO Port I Registers define   IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
//-----------------------------------------------------------------
#define PortI_Init		0x04            // PORT Data

#define PortI0_Ctrl		ALT             // EC_THRM_ALERT1
#define PortI1_Ctrl		ALT				// ADP_I
#define PortI2_Ctrl		OUTPUT			// DISUSB1
#define PortI3_Ctrl		INPUT      		// CHG_AC_PRES
#define PortI4_Ctrl		INPUT          	// BATT_PRS_L
#define PortI5_Ctrl		ALT		    	// BAT_I
#define PortI6_Ctrl		ALT		        // EC_THRM_ALERT2
#define PortI7_Ctrl		ALT         	// EC_THRM_ALERT3
//-----------------------------------------------------------------------------
#define Read_ACIN()			  	IS_MASK_SET(GPDRI,BIT(3))

#define Read_BatIN()		  	IS_MASK_CLEAR(GPDRI,BIT(4))
#define IS_BATTERY1_IN()      	IS_MASK_CLEAR(GPDRI, BIT(4))
#define IS_BATTERY1_OUT()     	IS_MASK_SET(GPDRI, BIT(4))

//-----------------------------------------------------------------
// GPIO Port J Registers define  JJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJJ
//-----------------------------------------------------------------
#define PortJ_Init		0x04            // PORT Data

#define PortJ0_Ctrl		OUTPUT			// M2PWR_ENT
#define PortJ1_Ctrl		OUTPUT			// HW_POP_MUTE_EC
#define PortJ2_Ctrl		OUTPUT			// DISUSB2
//TF_002--	#define PortJ3_Ctrl		INPUT			// CPU_RST_OK
#define PortJ3_Ctrl		OUTPUT			// LAN_PWR_SOFT_START //TF_002++
#define PortJ4_Ctrl		OUTPUT			// BAT_LED1_WH
#define PortJ5_Ctrl		OUTPUT			// KBLED_EN
//-----------------------------------------------------------------------------
#define M2PWR_ENT_ON()    		SET_MASK(GPDRJ,BIT(0))
#define M2PWR_ENT_OFF()   		CLEAR_MASK(GPDRJ,BIT(0))

#define MUTE_ON()    			SET_MASK(GPDRJ,BIT(1))
#define MUTE_OFF()   			CLEAR_MASK(GPDRJ,BIT(1))

//TF_002++>>
#define LAN_OFF()    			SET_MASK(GPDRJ,BIT(3))
#define LAN_ON()   		      	CLEAR_MASK(GPDRJ,BIT(3))
//TF_002++<<

#define BAT_LED1_ON()    		SET_MASK(GPDRJ,BIT(4))
#define BAT_LED1_OFF()   		CLEAR_MASK(GPDRJ,BIT(4))

#define KBLED_ON()				SET_MASK(GPDRJ,BIT(5))
#define KBLED_OFF()				CLEAR_MASK(GPDRJ,BIT(5))

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#define Fan_volt		DCR4 	// Fan1 PWM
#define Fan2_volt		DCR7	// Fan2 PWM
#define PC_BEEP_PWM	    DCR3    // beep

#define Read_SYS_I()	    1//Fix_ADCAvg1             // System current
#define Read_VAdapter()     //ADCAvg0                 // Adapter voltage
#define Read_CPU_VCC()      //ADCAvg0                    
#define Read_CPU_ICC()      //ADCAvg1
#define Read_GFX_VR_VCC()   //ADCAvg3
#define Read_GFX_VR_ICC()   //ADCAvg4



extern void Init_GPIO(void);
extern void TurnOnOffWlan(void);
extern void TurnOnOffBT(void);
extern void TurnOnOffCamera(void);
extern void Hook_SCION(void);
extern void Hook_SCIOFF(void);
extern void Hook_SMION(void);
extern void Hook_SMIOFF(void);
extern void Hook_A20ON(void);
extern void Hook_A20OFF(void);
extern void Hook_KBRSTON(void);
extern void Hook_KBRSTOFF(void);
extern void Hook_NUMLED_ON(void);
extern void Hook_NUMLED_OFF(void);
extern void Hook_CAPLED_ON(void);
extern void Hook_CAPLED_OFF(void);
extern void BackLight_ON(BYTE blk);
extern void BackLight_OFF(BYTE blk);

typedef struct InitGPIOReg
{
    XBYTE 	*reg;	
	BYTE	value;
} sInitGPIOReg;

#endif
