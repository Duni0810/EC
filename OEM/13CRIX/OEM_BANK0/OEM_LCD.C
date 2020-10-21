
#include <CORE_INCLUDE.H>
#include <OEM_INCLUDE.H>

// 这个变量没有被使用到
const BYTE code BrightnessTBL_PWM[MAX_LEVEL+1]= {0x01,0x04,0x08,0x0C,0x12,0x19,0x20,0x25,0x2D,0x34,0x3F};

//-----------------------------------------------------------------------------
// Setup Brightness value
//-----------------------------------------------------------------------------
void Setup_Brightness(BYTE brg_level)
{
    // check setup value is under range
	if(brg_level > MAX_LEVEL)
	{
		brg_level = MAX_LEVEL;
	}

    //if(SystemIsS0)
    //{
    	//PWM_VADJ = BrightnessTBL_PWM[brg_level];
    //}
	//else
	//{
		//PWM_VADJ = 0x40;  // brightness is 0
	//}
}

//-----------------------------------------------------------------------------
// Initialize Brightness variable
// 初始化 LCD 背光灯
//-----------------------------------------------------------------------------
void Init_Brightness(void)
{
    BrgLevel = 0x04 ;	
	bBrightness = 0 ;

	// 检查系统类型，如果是 linux（3）
	// 设置背光亮度等级
	if(OS_Type==3)
	{
		LCD_BRIGHTNESS = 10;
	}
	else if(OS_Type > 8 && OS_Type < 12)
	{
		LCD_BRIGHTNESS = LCD_BRIGHTNESS_BK;
	}
	else
	{
		LCD_BRIGHTNESS = 10;
	}
}

// 关闭背光，几乎等于关闭显示器
void Off_Brightness(void)
{
    BrgLevel = 0x00 ;
}

//-----------------------------------------------------------------------------
// Check Brightness level change
// 检查亮度等级，如果是超过最亮的等级，则重新赋值为最大值
//-----------------------------------------------------------------------------
void Check_Brightness(void)
{
    if(bBrightness != BrgLevel)
    {    
        bBrightness = BrgLevel;
		LCD_BRIGHTNESS = BrgLevel;
        Setup_Brightness(bBrightness);
    }
}

