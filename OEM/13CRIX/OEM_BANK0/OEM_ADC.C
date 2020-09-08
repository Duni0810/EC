/*-----------------------------------------------------------------------------
 * TITLE: OEM_ADC.C
 *
 * Author : Dino
 *
 * Note : These functions are reference only.
 *        Please follow your project software specification to do some modification.
 *---------------------------------------------------------------------------*/
 
#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

#define ADC_RAM_DEBUG 0
//prefix code ADC_RAM_DEBUG(0xB0);
#if ADC_RAM_DEBUG
#define ADC_RamDebug(x)           RamDebug(x)
#else
#define ADC_RamDebug(x)           {}
#endif

//----------------------------------------------------------------------------
// The function of disabling ADC channel
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Disable ADC module
//----------------------------------------------------------------------------
void DisableADCModule(void)
{
    VCH0CTL=0x1F;
	//VCH1CTL=0x1F;
	//VCH2CTL=0x1F;
	//VCH3CTL=0x1F;

    //VCH4CTL = 0x80;     // Voltage Channel 4 Disable
    //VCH5CTL = 0x80;   // Voltage Channel 5 Disable
    //VCH6CTL = 0x80;   // Voltage Channel 6 Disable
    VCH7CTL = 0x80;   // Voltage Channel 7 Disable
    
	CLEAR_MASK(ADCCFG, ADCEN);
}

//----------------------------------------------------------------------------
// Enable ADC module
//----------------------------------------------------------------------------
void EnableADCModule(void)
{
    VCH0CTL = ADCChannel_0;   // Clear data vaild flag, and select Channel 0
    //VCH1CTL = ADCChannel_1;   // Clear data vaild flag, and select Channel 1
    //VCH2CTL = ADCChannel_2;   // Clear data vaild flag, and select Channel 2
    //VCH3CTL = ADCChannel_3;   // Clear data vaild flag, and select Channel 3

    //VCH4CTL = ADCChannel_4;     // Voltage Channel 4 Enable
    //VCH5CTL = ADCChannel_5;   // Voltage Channel 5 Enable
    //VCH6CTL = ADCChannel_6;   // Voltage Channel 6 Enable
    VCH7CTL = ADCChannel_7;   // Voltage Channel 7 Enable
    
	SET_MASK(ADCCFG, ADCEN);
}


// 扫描所有的ADC通道，这里主要用到ADC通道1与ADC通道7
void ScanAll_ADCChannel(void)
{
#if Used_Ch0
    if(IS_MASK_SET(VCH0CTL, DATVAL))
    {
        SET_MASK(VCH0CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg0 = (XWORD)((VCH0DATM<<8)+VCH0DATL);
    }
#endif

#if Used_Ch1
    if(IS_MASK_SET(VCH1CTL, DATVAL))
    {
        SET_MASK(VCH1CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg1 = (XWORD)((VCH1DATM<<8)+VCH1DATL);
    }
#endif

#if Used_Ch2
    if(IS_MASK_SET(VCH2CTL, DATVAL))
    {
        SET_MASK(VCH2CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg2 = (XWORD)((VCH2DATM<<8)+VCH2DATL);
    }
#endif

#if Used_Ch3
    if(IS_MASK_SET(VCH3CTL, DATVAL))
    {
        SET_MASK(VCH3CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg3 = (VCH3DATM*0xFF)+VCH3DATL;
    }
#endif

#if Used_Ch4
    if(IS_MASK_SET(VCH4CTL, DATVAL))
    {
        SET_MASK(VCH4CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg4 = (VCH4DATM*0xFF)+VCH4DATL;
    }
#endif	

#if Used_Ch5
    if(IS_MASK_SET(VCH5CTL, DATVAL))
    {
        SET_MASK(VCH5CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg5 = (VCH5DATM*0xFF)+VCH5DATL;
    }
#endif

#if Used_Ch6
    if(IS_MASK_SET(VCH6CTL, DATVAL))
    {
        SET_MASK(VCH6CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg6 = (VCH6DATM*0xFF)+VCH6DATL;
    }
#endif

#if Used_Ch7
    if(IS_MASK_SET(VCH7CTL, DATVAL))
    {
        SET_MASK(VCH7CTL,DATVAL);        // write 1, to clear data vaild flag
        ADCAvg7 = (VCH7DATM*0xFF)+VCH7DATL;
    }
#endif
}


//----------------------------------------------------------------------------
// Init ADC module
//----------------------------------------------------------------------------
void Init_ADC(void)
{
    // 初始化ADC功能
    // 1. 将  AINITB @ ADCSTS 执行一次开关，手册有要求这样操作，并且这个位设置为0，如果设置为1会导致大量的能耗
    // 2. 使能 数据溢出事件和周期结束事件，并选择ADC时钟源 DOVE,EOCE,SDIVSRC @ ADCSTS
    // 3. 设置ADC转化时间，为52.2ms,并禁能ADC模块  {ADCCTS1, ADCCTS0} = 11B
    // 4. 使能ADC通道0以及ADC通道7   VCHnCTL(1,7)
    // 5. 使能ADC 功能   ADCCFG bit0

    // Initial ADC registers 手册要求这样  初始化不应该启动EC因为会提高能耗
    SET_MASK(ADCSTS, AINITB);    // Analog Accuracy Initialization start
    CLEAR_MASK(ADCSTS, AINITB);  // Analog Accuracy Initialization stop

    SET_MASK(ADCSTS, DOVE);      // Data Overflow Event
    SET_MASK(ADCSTS, EOCE);      // End-of-Cycle Event
    CLEAR_MASK(ADCSTS, SDIVSRC); // Clock Source Select, EC clock(9.2MHz) 选择EC时钟源
    
    // ADC Conversion Time set
    //
    // {ADCCTS1, ADCCTS0}     =11b              =01b                =00b/10b
    // Conversion Time (us)   200*(SCLKDIV+1)   103.2*(SCLKDIV+1)   30.8*(SCLKDIV+1)
    //
    // According to the following Settings, the Conversion time is :
    // 200*(0xFA+1) = 50200us = 50.2ms
    SET_MASK(ADCCFG, DFILEN);    // ADC Conversion Time Select 0, ADCCTS0
    SET_MASK(ADCSTS, DFILEN);    // ADC Conversion Time Select 1, ADCCTS1
    ADCCTL = 0xFA;               // Default is 0x15

    CLEAR_MASK(ADCCFG, BIT0);    // ADC Module disable


    VCH0CTL = ADCChannel_0;   // Clear data vaild flag, and select Channel 0
    //VCH1CTL = ADCChannel_1;   // Clear data vaild flag, and select Channel 1
    //VCH2CTL = ADCChannel_2;   // Clear data vaild flag, and select Channel 2
    //VCH3CTL = ADCChannel_3;   // Clear data vaild flag, and select Channel 3

    //VCH4CTL = ADCChannel_4;     // Voltage Channel 4 Enable
    //VCH5CTL = ADCChannel_5;   // Voltage Channel 5 Enable
    //VCH6CTL = ADCChannel_6;   // Voltage Channel 6 Enable
    VCH7CTL = ADCChannel_7;   // Voltage Channel 7 Enable  
    
    SET_MASK(ADCCFG, BIT0);      // ADC Module Enable
}



#if 1

// NTC 热敏电阻曲线
const unsigned char code NTC110K[]=
{
//0   1        2        3        4        5        6        7       8        9        A       B        C        D       E      F
110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 110, 107, 105, 102, 99, 97,
95,  93,  92,  90,  89,  87,  85,  84,  83,  82,  80,  79,  78,  77,  76, 75,
74,  73,  72,  71,  70,  70,  69,  68,  67,  66,  66,  65,  64,  64,  63, 62,
62,  61,  60,  60,  59,  58,  58,  57,  57,  56,  56,  55,  55,  54,  53, 53,
52,  52,  51,  51,  51,  50,  50,  49,  49,  48,  48,  47,  47,  47,  46, 46,
45,  45,  44,  44,  44,  43,  43,  43,  42,  42,  41,  41,  41,  40,  40, 39,
39,  39,  39,  38,  38,  37,  37,  37,  37,  36,  36,  36,  35,  35,  35, 34,
34,  33,  33,  33,  33,  32,  32,  32,  31,  31,  30,  30,  30,  30,  29, 29,
28,  28,  28,  28,  27,  27,  27,  27,  26,  26,  26,  25,  25,  25,  24, 24,
24,  23,  23,  23,  22,  22,  22,  22,  21,  21,  21,  21,  20,  20,  20, 19,
19,  19,  18,  18,  18,  18,  17,  17,  17,  16,  16,  16,  15,  15,  15, 15,
14,  14,  14,  13,  13,  13,  12,  12,  12,  12,  11,  11,  11,  10,  10, 10,
9,   9,   9,   8,   8,   8,   7,   7,   7,   6,   6,   6,   5,   5,   5,  4,
4,   4,   3,   3,   3,   2,   2,   2,   1,   1,   1,   0,   0,   0,   0,  0,
};



// 以下都是为了通过热敏电阻的变化值，获取温度参数
void Get_Thermistor0_data(void)
{
	BYTE index;

	index = ADCAvg0>>2;
	if(index>220)
	{
		index=220;
	}

	ADC0_NTC_T = NTC110K[index];

	if(ADC0_NTC_T>13) // Dennis required to correction error  这个是错误码，相当于偏移量了吧
	{
		ADC0_NTC_T = ADC0_NTC_T-13;
	}
	else
	{
		ADC0_NTC_T=0;
	}
}

// 这个通道好像没有被使能
void Get_Thermistor1_data(void)
{
	BYTE index;

	index = ADCAvg1>>2;
	if(index>220)
	{
		index=220;
	}

	ADC1_NTC_T = NTC110K[index];
}

//J80_019++>>
void Get_Thermistor7_data(void)
{
	BYTE index;

	index = ADCAvg7>>2;
	if(index>220)
	{
		index=220;
	}
	
	ADC7_NTC_T = NTC110K[index];

	if(ADC7_NTC_T>13) // Dennis required to correction error
	{
		ADC7_NTC_T = ADC7_NTC_T-13;
	}
	else
	{
		ADC7_NTC_T=0;
	}
}
//J80_019++<<
#endif