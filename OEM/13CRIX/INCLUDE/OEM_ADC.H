#ifndef OEM_ADC_H
#define OEM_ADC_H  

//----------------------------------------------------------------------------
// It's for select ADC channel
//----------------------------------------------------------------------------
#define ADCChannel_0     0x80
#define ADCChannel_1     0x81
#define ADCChannel_2     0x82
#define ADCChannel_3     0x83
#define ADCChannel_4     0x90
#define ADCChannel_5     0x90
#define ADCChannel_6     0x90
#define ADCChannel_7     0x90

#define Used_Ch0         TRUE
#define Used_Ch1         TRUE
#define Used_Ch2         TRUE
#define Used_Ch3         TRUE
#define Used_Ch4         TRUE
#define Used_Ch5         TRUE
#define Used_Ch6         TRUE
//J80_019--	#define Used_Ch7         FALSE
#define Used_Ch7         TRUE	//J80_019++

extern void DisableADCModule(void);
extern void EnableADCModule(void);
extern void Init_ADC(void);
extern void ScanAll_ADCChannel(void);

extern void Get_Thermistor0_data(void);
extern void Get_Thermistor1_data(void);
extern void Get_Thermistor7_data(void);	//J80_019++
#endif