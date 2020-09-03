/*-----------------------------------------------------------------------------
 * TITLE: CORE_MAIN.C - Main Program for KBC firmware.
 *
 * Main processing loop (interrupt driven) for the keyboard controller/scanner.
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

/* ----------------------------------------------------------------------------
 *  FUNCTION: main - Main service loop.
 *
 *  Wait in idle state until an IRQ causes an exit from idle.  If the IRQ
 *  handler posted a service request (via bSERVICE) then dispatch control to
 *  the appropriate service handler.  Otherwise, go back to idle state.  After
 *  all service requests have been handled, return to idle state.
 * ------------------------------------------------------------------------- */
// 关于电源选项中，S0,S1,S2,S3,S4,S5 的含义如下：
//     S0: 实际上就是我们平常使用的工作状态，所有设备都开启；
//     S1: 待机状态，这时除了通过CPU时钟控制器将CPU关闭之外，其他的部件仍然正常工作
//     S2: power standby 这时，CPU处于停止运行状态，总线时钟也被关闭，但其余设备仍
// 然运行，与S1几乎一样
//     S3: Suspend to RAM 就是把系统运行中的所有资料保存不动，进入假关机状态，此时
// 除了内存需要电源供电之外，其他设备全部掉电
//     S4: Suspend to Disk 即使把windows 内存中的资料完全保存在HDD中，重新开机时，
// 直接从HDD完整的读到RAM,使用这种模式，HDD一定要留出一个完整的连续空间。
//     S5: shutdown
void main(void)
{
	DisableAllInterrupt(); 
	SP = 0xC0;					// Setting stack pointer


    // 如果 WinFlashMark == 0x33 执行下面的 
	if(Hook_ECRetunrMainFuncKeepCondition()==0x33)  // Exit from follow mode or EC scatch ROM
	{
		CLEAR_MASK(FBCFG,SSMC); // disable scatch ROM ; FBCFG: flash bus config  为了兼容8510

        // 与手册中的一致 page209 这个是为了清空 cache
		_nop_();
	    MPRECF = 0x01;          // MPRECF 手动预取计数  清除内部动态缓存
	    _nop_();
	    MPRECF = 0x01;
	    _nop_(); 
	    MPRECF = 0x01;
	    _nop_();
	    MPRECF = 0x01;
	    _nop_();
	    _nop_();


        WinFlashMark = 0x00;
        ResetBANKDATA();          // init bank mechanism to code bank 0
        Hook_ECExitFollowMode();  // 初始化 SMBUS cache 等操作(cache 没做什么事情)

        Init_Timers();            // 初始化定时器0与1，并使能定时器0

		EnableModuleInterrupt();  // 使能 部分功能模块的中断 keboard 与 PMC
	}
	else
	{
		Core_Initialization();    // 片上初始化，初始化片内寄存器，和用户自定义参数状态

		Oem_Initialization();     // 产商初始化
        InitEnableInterrupt();    // 使能外部中断1 ，清除所有中断标志位，并开启总中断
		
        #if UART_Debug
		uart_Initial();
        UART_Print_Str("\n\n------------------------------------");
        UART_Print_Str("\n  EC Init OK !!!");
        UART_Print_Str("\n\n------------------------------------");
		#else
		//uart_Initial_Host();
		#endif
	}


	if((0x55==BRAM_FLASH_ID0)&&(0xaa==BRAM_FLASH_ID1)&&(0x55==BRAM_FLASH_ID2)&&(0xaa==BRAM_FLASH_ID3))
	{
		//PulseSBPowerButton();
	}

	BRAM_FLASH_ID0=0;
	BRAM_FLASH_ID1=0;
	BRAM_FLASH_ID2=0;
	BRAM_FLASH_ID3=0;	

	while(1)
   	{
        // 肯定会运行这个if OEM_SkipMainServiceFunc 空
        if(OEM_SkipMainServiceFunc()==Normal_MainService)
        {
    		main_service();  // core main
            
    		EnableModuleInterrupt();
    		_nop_();
    		_nop_();
    		_nop_();
    		_nop_();

			#if TouchPad_only
            if(PS2CheckPendingISR()==0x00)
            {
                ScanAUXDeviceStep();
            }
			#endif

            // 状态判断 如果都没有什么服务要操作的话，直接进入空闲状态
            #ifdef SMBusServiceCenterFunc
    		if((Service==0x00)&&(Service1==0x00)&&(CheckSMBusNeedService()==SMBus_ClearService))
            #else
            if((Service==0x00)&&(Service1==0x00))
            #endif
    		{
         		PCON=1;      		// enter idle mode  power control
    		}
        }
  	} 
}

/* ----------------------------------------------------------------------------
 * FUNCTION: main_service - Check for new/more service requests.
 *
 * Check for a request flag.  The check is done according to priority.  If a
 * request flag is set, handle the request and loop back to get the flags
 * again.  Do not follow through to check the next flag.  The flags are to be
 * checked in order.
 * ------------------------------------------------------------------------- */
void main_service(void) 
{
    #ifdef SMBusServiceCenterFunc
    while((Service!=0x00)||(Service1!=0x00)||(CheckSMBusNeedService()==SMBus_NeedService))
    #else
    while((Service!=0x00)||(Service1!=0x00))
    #endif
    {
        //-----------------------------------
        // Host command/data service    6064 
        // 通过KBC接口处理来自系统的命令/数据
        // 表示有6064 服务数据需要发送
        //-----------------------------------
        if(F_Service_PCI)
        {
            F_Service_PCI=0;
            service_pci1();
            continue;
        }

        //-----------------------------------
        // Service unlock 不运行 参数 F_Service_UNLOCK  没被初始化
        //-----------------------------------
        if(F_Service_UNLOCK)
        {
            F_Service_UNLOCK=0;
            service_unlock();
            continue;
        }

        //-----------------------------------
        // Send byte from KBC       6064
        //-----------------------------------
        if(F_Service_SEND)
        {
            F_Service_SEND=0;
            service_send();
            continue;
        }

        //-----------------------------------
        // Send PS2 interface data	
        //-----------------------------------
        if(F_Service_Send_PS2)
        {
            F_Service_Send_PS2=0;
            service_PS2_data();
            continue;
        }

        //-----------------------------------
        // process PS2 interface data
        // 这个PS2 好像是 鼠标的服务函数
        //-----------------------------------
        if(F_Service_PS2)
        {
            F_Service_PS2=0;
            service_ps2();
            continue;
        }

        //-----------------------------------
        // process SMBus interface data
        // SMBus 服务函数
        //-----------------------------------
        #ifdef SMBusServiceCenterFunc
		
		if (IS_MASK_CLEAR(SYS_MISC1, BAT_FW_UP))
	 	{
        	ServiceSMBus();
		}	
		
        #endif

        //-----------------------------------
        // Secondary Host command/data service
        // PM1 相关
        //-----------------------------------
        if(F_Service_PCI2)
        {
            F_Service_PCI2=0;
            service_pci2();
            continue;
        }


        //-----------------------------------
        // 1 millisecond elapsed  事件轮训状态查询并使能
        // 这个函数比较重要
        //-----------------------------------
        if(F_Service_MS_1)
        {
            F_Service_MS_1=0;
            service_1mS();
            continue;
        }


        //-----------------------------------
        // Keyboard scanner service 5ms 事件 
        // 执行键盘扫描操作 ,将扫描到的键值进行处理获取键码 存入buffer
        //-----------------------------------
        if(F_Service_KEY)
        {
        	F_Service_KEY=0;  // 这个是属于5ms 事件中定义的
			service_scan();
			continue;
        }

        //-----------------------------------
        // 空函数  只是保留在这里
        //-----------------------------------
        Hook_main_service_H();
        
        //-----------------------------------
        // Low level event  5ms 事件
        //-----------------------------------
        if(F_Service_Low_LV)
        {
            F_Service_Low_LV=0;
            service_Low_LVEvent();
            continue;
        } 

        //-----------------------------------
        // Third Host command/data service PMC PM2相关
        // 输入buffer 满中断
        //-----------------------------------
        if(F_Service_PCI3)
        {
            F_Service_PCI3=0;
            service_pci3();
            continue;
        }

        //-----------------------------------
        // CIR IRQ  空函数 没使用
        //-----------------------------------
        if(F_Service_CIR)
        {
            F_Service_CIR=0;
            service_cir();
            continue;
        }

        //-----------------------------------
        // fourth command/data service  PM3 相关
        //-----------------------------------
        if(F_Service_PCI4)
        {
            F_Service_PCI4=0;
            service_pci4();
            continue;
        }

        // 下面的行数都没有使用
        //------------------------------------
        // service_OEM_1
        //------------------------------------
        if(F_Service_OEM_1)
        {
            F_Service_OEM_1=0;
            service_OEM_1();
            continue;
        }

        //------------------------------------
        // service_OEM_2
        //------------------------------------
        if(F_Service_OEM_2)
        {
            F_Service_OEM_2=0;
            service_OEM_2();
            continue;
        }

        //------------------------------------
        // service_OEM_3
        //------------------------------------
        if(F_Service_OEM_3)
        {
            F_Service_OEM_3=0;
            service_OEM_3();
            continue;
        }

        //------------------------------------
        // service_OEM_4
        //------------------------------------
        if(F_Service_OEM_4)
        {
            F_Service_OEM_4=0;
            service_OEM_4();
            continue;
        }

        //-----------------------------------
        //
        //-----------------------------------
        Hook_main_service_L();
    }
}

static void service_Low_LVEvent(void)
{
	if((KBPendingTXCount != KBPendingRXCount )||(scan.kbf_head != scan.kbf_tail))
	{
		SetServiceSendFlag();
	}

    if(IS_MASK_SET(KBHISR,P_IBF))
    {
  	    F_Service_PCI = 1;
    }
    
    if(IS_MASK_SET(PM1STS,P_IBF))
    {
  	    F_Service_PCI2 = 1;
    }
}

//----------------------------------------------------------------------------
// FUNCTION: service_unlock
// Unlock aux devices and re-enable Host interface IRQ if it is ok to do so.
//----------------------------------------------------------------------------
static void service_unlock(void)
{
	Unlock_Scan();
}

//------------------------------------------------------------
// Polling events
//------------------------------------------------------------
void service_1mS(void)
{
	Timer1msEvent();
	Timer1msCnt++;
    if(Timer1msCnt>=10)
    {
        Timer1msCnt = 0x00;
    }

    if(Hook_Only_Timer1msEvent()==Only_Timer1msEvent)
    {   
        return;
    }

    if((Timer1msCnt%5)==0x00)
    {
	    Timer5msEvent();
	    Timer5msCnt++;
	    if ( Timer5msCnt & 1 )  	// 10ms events
	    {
            Timer10msEventA();
	    }
	    else
	    {
		    Timer10msEventB();
     	    switch( Timer5msCnt )   // Share Loading Branch Control
    	    {
       		    case 2: Timer50msEventA();
                    break;
          	    case 4: Timer50msEventB();
             	    break;
        	    case 6: Timer50msEventC();
              	    break;
          	    case 8: Timer100msCntB++;
         		    if ( Timer100msCntB & 1 )
             	    {
                  	    Timer100msEventA();
              	    }
             	    else
             	    {
                   	    Timer100msEventB();
              	    }
               	    break;

           	    default:        
				    Timer5msCnt=0;
              	    break;
     	    }

    	    if ( Timer5msCnt == 0x00 )  // default 事件
    	    {       			// 50msec
          	    Timer100msCnt ++;
          	    if ( Timer100msCnt & 1 )
         	    {
             	    Timer100msEventC();
          	    }
         	    else
     		    {       		// 100msec
          		    switch( Timer100msCnt )
              	    {
                	    case 2:	Timer500msEventA();
                 		    break;
                 	    case 4:	Timer500msEventB();
                      	    break;
                 	    case 6:	Timer500msEventC();
                     	    break;
                 	    case 8:	Timer1SecEventA();
                     	    break;
					    case 10: 	Timer1SecEventB();
                     	    break;	
                 	    case 12:	Timer500msEventA();
                      	    break;
                	    case 14:	Timer500msEventB();
                      	    break;
               		    case 16: 	Timer500msEventC();
                      	    break;
                 	    case 18: 	Timer1SecEventC();
                     	    break;
                  	    default:        // 1 Sec
                      	    Timer100msCnt = 0;
                  		    Timer1SecCnt ++;
                    	    if ( Timer1SecCnt == 60 )
                      	    {
                         	    Timer1MinEvent();
                         	    Timer1SecCnt=0;
                     	    }
                    	    break;
          		    }
              	}
       		}
   		}
	} 
}

//------------------------------------------------------------
// 1ms events
//------------------------------------------------------------
void Timer1msEvent(void)
{
    ReSendPS2PendingData();
    Hook_Timer1msEvent(Timer1msCnt);
}

//------------------------------------------------------------
// 5ms events
//------------------------------------------------------------
void Timer5msEvent(void)
{
    F_Service_Low_LV = 1;
	if (Timer_A.fbit.TMR_SCAN) 
	{
  		F_Service_KEY = 1;		// Request scanner service. 
  	}
    Hook_Timer5msEvent();
}

//------------------------------------------------------------
// 10ms events
//------------------------------------------------------------
void Timer10msEventA(void)
{
    SetStartScanAUXFlag();
	Hook_Timer10msEventA();	
}

//------------------------------------------------------------
// 10ms events
//------------------------------------------------------------
void Timer10msEventB(void)
{
	Hook_Timer10msEventB();	 
}

//------------------------------------------------------------
// 50ms events A
//------------------------------------------------------------
void Timer50msEventA(void)
{
	Hook_Timer50msEventA();
}

//------------------------------------------------------------
// 50ms events B
//------------------------------------------------------------
void Timer50msEventB(void)
{
	Hook_Timer50msEventB();
}

//------------------------------------------------------------
// 50ms events C
//------------------------------------------------------------
void Timer50msEventC(void)
{
	Hook_Timer50msEventC();
}

//------------------------------------------------------------
// 100ms events A
//------------------------------------------------------------
void Timer100msEventA(void)
{
	Hook_Timer100msEventA();
}

//------------------------------------------------------------
// 100ms events B
//------------------------------------------------------------
void Timer100msEventB(void)
{
	Hook_Timer100msEventB();
}

//------------------------------------------------------------
// 100ms events C
//------------------------------------------------------------
void Timer100msEventC(void)
{
	Hook_Timer100msEventC();
}

//------------------------------------------------------------
// 500ms events A
//------------------------------------------------------------
void Timer500msEventA(void)
{
	Hook_Timer500msEventA();
}

//------------------------------------------------------------
// 500ms events B
//------------------------------------------------------------
void Timer500msEventB(void)
{
	Hook_Timer500msEventB();
}

//------------------------------------------------------------
// 500ms events C
//------------------------------------------------------------
void Timer500msEventC(void)
{
	Hook_Timer500msEventC();
}

//------------------------------------------------------------
// 1sec events A
//------------------------------------------------------------
void Timer1SecEventA(void)
{	
	Hook_Timer1SecEventA();
}

//------------------------------------------------------------
// 1sec events B
//------------------------------------------------------------
void Timer1SecEventB(void)
{
	Hook_Timer1SecEventB();
}

//------------------------------------------------------------
// 1sec events C
//------------------------------------------------------------
void Timer1SecEventC(void)
{
	Hook_Timer1SecEventC();
}

//------------------------------------------------------------
// 1min events
//------------------------------------------------------------
void Timer1MinEvent(void)
{
    Hook_Timer1MinEvent();
}

