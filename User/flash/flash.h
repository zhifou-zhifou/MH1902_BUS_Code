#ifndef _FLASH_H
#define _FLASH_H


//==================================================================================
// 下面定义了 FLASH每个扇区的结束地址  共16个扇区 每个扇区32KB
//==================================================================================
#define ADDR_FLASH_SECTOR_0   0x01007FFF
#define ADDR_FLASH_SECTOR_1   0x0100FFFF
#define ADDR_FLASH_SECTOR_2   0x01017FFF
#define ADDR_FLASH_SECTOR_3   0x0101FFFF
#define ADDR_FLASH_SECTOR_4   0x01027FFF
#define ADDR_FLASH_SECTOR_5   0x0102FFFF
#define ADDR_FLASH_SECTOR_6   0x01037FFF
#define ADDR_FLASH_SECTOR_7   0x0103FFFF
#define ADDR_FLASH_SECTOR_8   0x01047FFF
#define ADDR_FLASH_SECTOR_9   0x0104FFFF
#define ADDR_FLASH_SECTOR_10  0x01057FFF
#define ADDR_FLASH_SECTOR_11  0x0105FFFF
#define ADDR_FLASH_SECTOR_12  0x01067FFF
#define ADDR_FLASH_SECTOR_13  0x0106FFFF
#define ADDR_FLASH_SECTOR_14  0x01077FFF
#define ADDR_FLASH_SECTOR_15  0x0107FFFF


//==================================================================================
// 下面定义了 每个扇区对应的  FCU_RO寄存器中写保护位
//==================================================================================
#define FLASH_Sector_0   0x0001
#define FLASH_Sector_1   0x0002
#define FLASH_Sector_2   0x0004
#define FLASH_Sector_3   0x0008
#define FLASH_Sector_4   0x0010
#define FLASH_Sector_5   0x0020
#define FLASH_Sector_6   0x0040
#define FLASH_Sector_7   0x0080
#define FLASH_Sector_8   0x0100
#define FLASH_Sector_9   0x0200
#define FLASH_Sector_10  0x0400
#define FLASH_Sector_11  0x0800
#define FLASH_Sector_12  0x1000
#define FLASH_Sector_13  0x2000
#define FLASH_Sector_14  0x4000
#define FLASH_Sector_15  0x8000


//==================================================================================
// 参数定义
//==================================================================================
#define USER_FLASH_START_ADDR   0x01070000   //FLASH最后两个扇区  供用户使用



//==================================================================================
// STM32 为大端结构
//==================================================================================
typedef union
{
    unsigned int  data;
    unsigned char buf[4];
}
u32tou8;

extern u32tou8 u32data;


void FLASH_WriteMoreData(unsigned int WriteAddr,unsigned char *pBuffer,unsigned int NumToWrite);
void FLASH_ReadMoreData(unsigned int ReadAddr,unsigned char *pBuffer,unsigned int NumToWrite);

#endif


