#include <stdio.h>
#include <stdlib.h>
#include <string.h>//C语言标准库

#include "mhscpu.h"
#include "mhscpu_flash.h"
#include "flash.h"

#include "math.h"

u32tou8 u32data;//定义一个联合体

//==================================================================================
// 获取某个地址所在的扇区
// addr:FLASH地址
// 返回： 该地址所在的扇区
//==================================================================================
uint16_t FLASH_GetFlashSector(unsigned int addr)
{
    if(addr<ADDR_FLASH_SECTOR_1)return FLASH_Sector_0;
    else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_Sector_1;
    else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_Sector_2;
    else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_Sector_3;
    else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_Sector_4;
    else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_Sector_5;
    else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_Sector_6;
    else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_Sector_7;
    else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_Sector_8;
    else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_Sector_9;
    else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_Sector_11;
    else if(addr<ADDR_FLASH_SECTOR_12)return FLASH_Sector_12;
    else if(addr<ADDR_FLASH_SECTOR_13)return FLASH_Sector_13;
    else if(addr<ADDR_FLASH_SECTOR_14)return FLASH_Sector_14;
    else return FLASH_Sector_15;
}


//==================================================================================
// 从FLASH中读取 一个字（32位）
// addr:读取地址
// 返回： 读到的字数据
//备注： 地址为4字节对齐
//==================================================================================
unsigned int FLSAH_ReadWord(unsigned int addr)
{
    return (*(unsigned int *)addr);
}

//==================================================================================
// 向FLASH指定地址 写入大量数据
// WriteAddr:写入首地址
// pBuffer:数据首地址
// NumToWrite:需要写入数据的大小
// 返回： 4=成功  1,2,3,5=失败
//备注： 写入的数据类型必须是32位  写入地址为4字节对齐
//==================================================================================
FLASH_Status  FLASH_Write(unsigned int	WriteAddr,unsigned int *pBuffer,unsigned int NumToWrite)
{

    FLASH_Status status = FLASH_COMPLETE;
    unsigned int startaddr,endaddr=0;
    startaddr = WriteAddr;
    endaddr = startaddr +4*NumToWrite;//结束地址

    FLASH_Unlock();
    FCU->RO = 0;//去掉所有扇区写保护
    //==================================================================================
    // 判断写入地址是否非法
    //==================================================================================
    if((startaddr < USER_FLASH_START_ADDR) || (endaddr >=ADDR_FLASH_SECTOR_15)) return FLASH_BUSY;

    //==================================================================================
    // 判断要写入的区域是否要擦除
    //==================================================================================
    while(WriteAddr < endaddr)
    {
        if(*(unsigned int *)WriteAddr != 0xFFFFFFFF)
        {
            status = FLASH_ErasePage(WriteAddr);
            if(status != FLASH_COMPLETE) return status;
        }
        WriteAddr +=4;
    }
    //==================================================================================
    // 写入内部FLASH
    //==================================================================================
    while(startaddr < endaddr)
    {
        if(FLASH_ProgramWord(startaddr, *pBuffer) != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return status;
        }
        startaddr +=4;
        pBuffer++;
    }
    FLASH_Lock();
}

//==================================================================================
//从FLASH指定地址 读取数据
//备注： 读取数据类型为32位  读取地址为4字节对齐
//==================================================================================
void  FLASH_Read(unsigned int	ReadAddr,unsigned int *pBuffer,unsigned int NumToRead)
{
    unsigned int i;
    for(i=0; i<NumToRead; i++)
    {
        pBuffer[i++] =FLSAH_ReadWord(ReadAddr);
        ReadAddr+=4;
    }
}


void FLASH_WriteMoreData(unsigned int WriteAddr,unsigned char *pBuffer,unsigned int NumToWrite)
{
    unsigned int i;
    unsigned count,remin = 0;
    count = NumToWrite/4;
    remin = NumToWrite%4;
    for(i = 0; i< count; i++)
    {
        u32data.buf[0] = *pBuffer;
        u32data.buf[1] = *(pBuffer+1);
        u32data.buf[2] = *(pBuffer+2);
        u32data.buf[3] = *(pBuffer+3);
        FLASH_Write(WriteAddr,&u32data.data,1);
        pBuffer+=4;//地址偏移4
        WriteAddr+=4;
    }
    if(NumToWrite%4)//继续写入剩余字节
    {
        for(i=0; i<remin; i++)
        {
            u32data.buf[i]=*(pBuffer+i);
        }
        FLASH_Write(WriteAddr,&u32data.data,1);
    }
}


void FLASH_ReadMoreData(unsigned int ReadAddr,unsigned char *pBuffer,unsigned int NumToWrite)
{
    unsigned int i;
    unsigned count,remin = 0;
    count = NumToWrite/4;
    remin = NumToWrite%4;
    for(i = 0; i< count; i++)
    {
        FLASH_Read(ReadAddr,&u32data.data,1);
        *pBuffer = u32data.buf[0];
        *(pBuffer+1)=u32data.buf[1];
        *(pBuffer+2)=u32data.buf[2];
        *(pBuffer+3)=u32data.buf[3];
        pBuffer+=4;//地址偏移4
        ReadAddr+=4;
    }
    if(remin)
    {

        FLASH_Read(ReadAddr,&u32data.data,1);
        for(i=0; i<remin; i++)
        {
            *(pBuffer+i) = u32data.buf[i];
        }
    }








}









