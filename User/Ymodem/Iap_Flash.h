#ifndef Iap_Flash_H
#define Iap_Flash_H
#define FLASH_BASE_ADDR 0x01000000      //Flash基地址
#define IAP_ADDR        (FLASH_BASE_ADDR+0xD400+100*1024)     //升级代码地址


//0x01026400    0x0100F000
#define IAP_PARAM_SIZE  1
#define IAP_PARAM_ADDR  (FLASH_BASE_ADDR + FLASH_SIZE - PAGE_SIZE) //Flash空间最后1页地址开始处存放参数


#define PAGE_SIZE          (0x1000)    //页的大小4K
//#define FLASH_SIZE         (0x80000)  //Flash空间512K

void IAP_DisableFlashWPR(void);
signed char IAP_UpdataParam(unsigned int *param);
void IAP_FlashEease(unsigned int size);
signed char IAP_UpdataProgram(unsigned int addr, unsigned int size);
#endif





