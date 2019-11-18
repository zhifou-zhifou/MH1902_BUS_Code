#include "mhscpu.h"
#include "mhscpu_flash.h"
#include <stdio.h>
#include "Iap_Flash.h"
#include "uart.h"
/*************************************************************
  Function   : IAP_DisableFlashWPR 
  Description: 关闭flash的写保护
  Input      : none        
  return     : none    
*************************************************************/
void IAP_DisableFlashWPR(void)
{
//	u32 blockNum = 0, UserMemoryMask = 0;

//    blockNum = (IAP_ADDR - FLASH_BASE_ADDR) >> 12;   //计算flash块
//	 UserMemoryMask = ((u32)(~((1 << blockNum) - 1)));//计算掩码

//	if((FLASH_GetWriteProtectionOptionByte() & UserMemoryMask) != UserMemoryMask)//查看块所在区域是否写保护
//	{
//		FLASH_EraseOptionBytes ();//擦除选择位
//	}
	
	 //判断是否解锁
    if (1 == FLASH_IsProtect(IAP_ADDR))
    {
        printf("addr %08X in block is protect,please unprotect it.\n",IAP_ADDR);
    }
    //解锁
    FLASH_Unlock();
    FLASH_UnProtect(IAP_ADDR);
    printf("Flash Unprotect.\n");
	
	
}


/*************************************************************
  Function   : 
  Description: 向FLASH的最后一页写入参数
  Input      : size-擦除的大小         
  return     : none    
*************************************************************/
signed char IAP_UpdataParam(unsigned int *param)
{
	unsigned int i;
	unsigned int flashptr = IAP_PARAM_ADDR;

	FLASH_Unlock();//flash解锁
	FLASH_UnProtect(IAP_PARAM_ADDR);
	for(i = 0; i < IAP_PARAM_SIZE; i++)
	{	
		//FLASH_ProgramWord(flashptr + 4 * i, *param);
		FLASH_ProgramWord(IAP_PARAM_ADDR+ 4 * i, *param);	
		if(*(unsigned int *)(flashptr + 4 * i) != *param)//编程完毕回读
		{
			return -1;
		}	
		param++;
	}
	FLASH_Lock();//flash上锁
	return 0;
}

/*************************************************************
  Function   : IAP_FlashEease 
  Description: 擦除Flash
  Input      : size-擦除的大小        
  return     : none    
*************************************************************/
void IAP_FlashEease(unsigned int size)
{
	unsigned int eraseCounter = 0;
	unsigned int nbrOfPage = 0;
	FLASH_Status FLASHStatus = FLASH_COMPLETE;	  

	if(size % PAGE_SIZE != 0)//计算需要擦写的页数
	{										  
		nbrOfPage = size / PAGE_SIZE + 1; 
	}
	else
	{
		nbrOfPage = size / PAGE_SIZE;
	}
	
	FLASH_Unlock();//解除flash擦写锁定
	
	FCU->RO = 0;//关闭所有区 的写保护
	for(eraseCounter = 0; (eraseCounter < nbrOfPage) && ((FLASHStatus == FLASH_COMPLETE)); eraseCounter++)//开始擦除
	{
		FLASHStatus = FLASH_ErasePage(IAP_ADDR + (eraseCounter * PAGE_SIZE));//擦除
		//IAP_SerialSendStr(".");//打印'.'以显示进度
	}
	FLASH_ErasePage(IAP_PARAM_ADDR);//擦除参数所在的flash页
	FLASH_Lock();//flash擦写锁定
}
/*************************************************************
  Function   : IAP_UpdataProgram 
  Description: 升级程序
  Input      : addr-烧写的地址 size-大小        
  return     : 0-OK 1-error    
*************************************************************/
signed char IAP_UpdataProgram(unsigned int addr, unsigned int size)
{
	unsigned int i;
	static unsigned int flashptr = IAP_ADDR;

	FLASH_Unlock();//flash解锁
	for(i = 0; i < size; i += 4)
	{	
		FLASH_ProgramWord(flashptr, *(unsigned int *)addr);//烧写1个字
		if(*(unsigned int *)flashptr != *(unsigned int *)addr)//判断是否烧写成功
		{
			return -1;
		}
		flashptr += 4;
		addr += 4;
	}
	FLASH_Lock();//flash解锁
	return 0;
}
