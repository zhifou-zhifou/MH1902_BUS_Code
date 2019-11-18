#include <stdio.h>
#include <stdlib.h>
#include <string.h>//C语言标准库

#include "debug.h"//调试函数 头文件

#include "gpio.h"
#include "spi.h"
#include "uart.h"
#include "timer.h"
#include "delay.h"//基本外设头文件

#include "flash.h" //内部FLASH头文件

#include "24cxx.h"//E2PROM头文件

#include "mh523.h"
#include "iso14443_4.h"
#include "iso14443a.h"
#include "iso14443b.h"//射频卡相关头文件

#include "7816.h"
#include "mhscpu_sci.h"//
#include "iso7816_3.h"//接触式卡头文件
#include "emv_errno.h"
#include "emv_hard.h"

#include "Packet.h"
#include "define.h" //上位机通讯协议相关
#include "jrreader.h"

#include "psam.h" //PSAM 头文件 
#include "psamAPI.h"

#include "Download.h"//Ymodem头文件
#include "Iap_Flash.h"

#include "Digital.h"//数码管
#include "led.h"//LED
#include "key.h"//按键


Packet receivePacket;//上位机接收数据结构体
Packet sendPacket;
MH523TypeDef MH523;//初始化一个卡片缓存区
//APDUPacket APDU;
extern Errorflag ProtocolFlag;

ISO7816_ADPU_Commands  APDU_Send;
ISO7816_ADPU_Responce  APDU_Rec;

unsigned char timess[6];

void NVIC_Configuration(void);

unsigned char write[]= {"flash write test111111111111111111111111111111111111111111111111!"};
unsigned char read[sizeof(write)];

void SystemReset(void);

int main()
{

    unsigned char status;
    unsigned char i = 0;
    unsigned char cmd_buf[512];
    unsigned short cmd_len = 0;
    char *offset = NULL;
    SYSCTRL_SYSCLKSourceSelect(SELECT_EXT12M);
    SYSCTRL_PLLConfig(SYSCTRL_PLL_96MHz);
    SYSCTRL_HCLKConfig(SYSCTRL_HCLK_Div_None);//HCLK = 96M
    SYSCTRL_PCLKConfig(SYSCTRL_PCLK_Div2);//PCLK = 48M
    SYSCTRL_APBPeriphClockCmd(SYSCTRL_APBPeriph_UART0 | SYSCTRL_APBPeriph_SPI0| SYSCTRL_APBPeriph_TIMM0|SYSCTRL_APBPeriph_GPIO, ENABLE);
    SYSCTRL_APBPeriphResetCmd(SYSCTRL_APBPeriph_UART0 | SYSCTRL_APBPeriph_SPI0| SYSCTRL_APBPeriph_TIMM0|SYSCTRL_APBPeriph_GPIO, ENABLE);
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_2);//NVIC分组2  2位抢占 2位子优先级  
	  
    ISO7816_Bspinit();//初始化7816接口
    init_timer0();
    uart_Config(115200, UART_Parity_No);
	
    printf("金融读卡器_V%s start...\n", VERSION);
    gpio_config();
    spi_config();

    IIC_Init();
    parameter_init();//软、硬件版本号初始化
    NVIC_Configuration();
    key_init();
		  BSP_LED_Init();
		//Key_Bsp_init(); 
    //RTC_Init();
    //pcd_init();//NFC芯片初始化
    dig_bsp_init();
    //IIC_Test();
    tst_SCIWarmReset(0);
//		timess[0]= 19;
//		timess[1]= 8;
//		timess[2]= 27;
//
//		timess[3]= 15;
//		timess[4]= 21;
//		timess[5]= 00;
    //SetTime(timess);

    //GetTimes(timess);
    //printf("-->get data %02d-%02d-%02d %02d:%02d:%02d\r\n",timess[0],timess[1],timess[2],timess[3],timess[4],timess[5]);
     //dig_test(0x3F);
		
    while(1)
    {
			  //Task_KEY_Scan();
			  //Key_Scan(); 
       // printf("%d-%d-%d %d-%d-%d",calendar.w_year,calendar.w_month,calendar.w_date,calendar.hour,calendar.min,calendar.sec);
        uart_cmd_Receive(cmd_buf,&cmd_len);
        UploadCardSn();//挥卡上送
			  SendKeyValue();//键值上送
        if(cmd_len != 0)
        {
            if(ProtocolFlag.receive_ok == 1)
            {
                SerJRReaderHandle(&receivePacket);
                protocolflag_init();
            }
            else if(cmd_buf[0]==0x09 && cmd_buf[1]==0x00)
            {
                PSAM_TestCommond(&cmd_buf[2]);
                printf("21\n");
            }
            else if(NULL!=(offset=strstr((const char*)cmd_buf,"update")))
            {
                UART_ITConfig(UART0, UART_IT_RX_RECVD, DISABLE);//关闭串口中断
                DownloadFirmware();//固件升级
                UART_ITConfig(UART0, UART_IT_RX_RECVD, ENABLE);//开启串口中断
            }
            else if(NULL!=(offset=strstr((const char*)cmd_buf,"set hv=")))//设置硬件版本号
            {
                SetHardVersionStat((unsigned char *)(offset+strlen("set hv=")));
                printf("hard = %s\r\n",(offset+strlen("set hv=")));
            }
            else if(NULL!=(offset=strstr((const char*)cmd_buf,"set sv=")))//设置软件版本号
            {
                SetSoftVersionStat((unsigned char *)(offset+strlen("set sv=")));
                printf("soft = %s\r\n",(offset+strlen("set sv=")));
            }
            else
            {
                if(NULL!=(offset=strstr((const char*)cmd_buf,"set price=")))
                {
                    printf("票价 = %s\r\n",(offset+strlen("set price=")));
                }
            }
        }
    }
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {
    }
}
#endif

void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    //NVIC_SetVectorTable(NVIC_VectTab_RAM,0);
    NVIC_SetPriorityGrouping(NVIC_PriorityGroup_1);

    NVIC_InitStructure.NVIC_IRQChannel = SCI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);


    NVIC_InitStructure.NVIC_IRQChannel = UART0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*
    NVIC_InitStructure.NVIC_IRQChannel = SCI1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = SCI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    */
}


void SystemReset(void)//系统软复位函数
{
    SYSCTRL ->LOCK_R &=~ BIT31;      //允许数字全局软复位
    SYSCTRL ->SOFT_RST2 |= BIT31;   //数字部分全局复位

    SYSCTRL ->LOCK_R &=~ BIT30;      //允许内核软复位
    SYSCTRL ->SOFT_RST2 |= BIT30;   //内核复位
}






