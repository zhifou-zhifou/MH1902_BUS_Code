#include "mhscpu.h"
#include "delay.h"
#include "uart.h"
#include "Ymodem.h"
#include "Iap_Flash.h"
#include "Download.h"

#include "core_cmFunc.h" //设置堆栈指针函数

extern unsigned char file_name[FILE_NAME_LENGTH];
unsigned char tab_1024[1024] = {0};
typedef  void (*pFunction)(void);
pFunction Jump_To_Application;
/*************************************************************
  Function   : IAP_JumpToApplication
  Description: 跳转到升级程序处
  Input      : none
  return     : none
*************************************************************/
void IAP_JumpToApplication(void)
{
    unsigned int JumpAddress;//跳转地址

	   //printf("addr= %ld\r\n",(*(__IO unsigned int *)IAP_ADDR));
    if(((*(__IO unsigned int *)IAP_ADDR) & 0x2FFE0000) == 0x20000000)//有升级代码，IAP_ADDR地址处理应指向主堆栈区，即0x20000000
    {  
        JumpAddress = *(__IO unsigned int *)(IAP_ADDR + 4);//获取复位地址
        Jump_To_Application = (pFunction)JumpAddress;//函数指针指向复位地址
        __set_MSP(*(__IO unsigned int*)IAP_ADDR);//设置主堆栈指针MSP指向升级机制IAP_ADDR
        Jump_To_Application();//跳转到升级代码处
    }
}
/*************************************************************
  Function   : DownloadFirmware
  Description: 下载升级固件
  Input      : none
  return     : none
*************************************************************/
void DownloadFirmware(void)
{
    unsigned char number[10]= "          ";        //文件的大小字符串
    int size = 0;
	  IAP_SerialSendStr("\r\nWaiting for the file to be send...(press 'a' or 'A' to abort)\r\n");
    size = YModem_Receive(&tab_1024[0]);//开始接收升级程序
    delayms(1000);//延时1s，让secureCRT有足够时间关闭ymodem对话，而不影响下面的信息打印
    if(size > 0)
    {
        IAP_SerialSendStr("+-----------------------------------+\r\n");
        IAP_SerialSendStr("Proramming completed successfully!\r\nName: ");
        IAP_SerialSendStr(file_name);//显示文件名
        YModem_Int2Str(number, size);
        IAP_SerialSendStr("\r\nSize:");
        IAP_SerialSendStr(number);//显示文件大小
        IAP_SerialSendStr("Bytes\r\n");
        IAP_SerialSendStr("+-----------------------------------+\r\n");
        IAP_JumpToApplication();
    }
    else if(size == -1)//固件的大小超出处理器的flash空间
    {
        IAP_SerialSendStr("The image size is higher than the allowed space memory!\r\n");
    }
    else if(size == -2)//程序烧写不成功
    {
        IAP_SerialSendStr("Verification failed!\r\n");
    }
    else if(size == -3)//用户终止
    {
        IAP_SerialSendStr("Aborted by user!\r\n");
    }
    else //其他错误
    {
        IAP_SerialSendStr("Failed to receive the file!\r\n");
    }
}
