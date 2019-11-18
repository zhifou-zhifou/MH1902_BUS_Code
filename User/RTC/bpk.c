#include "mh523.h"
#include <string.h>
#include <stdio.h>
#include "bpk.h"


void BPK_ReadLockFun(void)
{
	  //printf("%s\n", __func__);
    BPK_KeyReadLock(BPK_KEY_Region_0 | BPK_KEY_Region_1 | BPK_KEY_Region_2 | BPK_KEY_Region_3, ENABLE);
}

void BPK_ReadUnlockFun(void)
{
	   //printf("%s\n", __func__);
     BPK_KeyReadLock(BPK_KEY_Region_0 | BPK_KEY_Region_1 | BPK_KEY_Region_2 | BPK_KEY_Region_3, DISABLE);
}

void BPK_WriteLockFun(void)
{
		//printf("%s\n", __func__);
		BPK_KeyWriteLock(BPK_KEY_Region_0 | BPK_KEY_Region_1 | BPK_KEY_Region_2 | BPK_KEY_Region_3, ENABLE);
}

void BPK_WriteUnlockFun(void)
{
	  //printf("%s\n", __func__);
	  BPK_KeyWriteLock(BPK_KEY_Region_0 | BPK_KEY_Region_1 | BPK_KEY_Region_2 | BPK_KEY_Region_3, DISABLE);
}

void printBPK(void)
{
	uint32_t key[32];
	uint32_t index = 0;
	
	while(BPK_IsReady() == RESET);

	printf("%s\n", __func__);
	memset(key, 0, sizeof(key));
	BPK_ReadKey(key,sizeof(key)/sizeof(key[0]),0);
	for(index = 0;index < sizeof(key)/sizeof(key[0]);index++)
	{
		printf("%08X ", key[index]);
		if (3 == index % 4)
		{
			printf("\n");
		}
	}
	printf("\n");
	memset(key, 0, sizeof(key));
}

void clearBPK(void)
{
	uint32_t buf[32];
	
	printf("%s\n", __func__);
	while(BPK_IsReady() == RESET);
	
	//clear buf
	memset(buf, 0, sizeof(buf));
	BPK_WriteKey(buf,sizeof(buf)/sizeof(buf[0]),0);
	printBPK();
}

void setBPK(void)
{
	uint32_t buf[32];
	uint32_t index = 0;
	printf("%s\n", __func__);
	while(BPK_IsReady() == RESET);
	
	//set buf 0~X
	for(index = 0;index < sizeof(buf)/sizeof(buf[0]);index++)
	{
		buf[index] = index;
	}
	BPK_WriteKey(buf,sizeof(buf)/sizeof(buf[0]),0);
	printBPK();
}

void BPK_ReadLock_Test(void)
{
	printf("=======================%s=======================\n", __func__);
	printf("write and read lock disable\n");
	BPK_ReadUnlockFun();
	BPK_WriteUnlockFun();
	clearBPK();
    setBPK();
	printf("read lock enable\n");
	BPK_ReadLockFun();
	printBPK();
}

void BPK_WriteLock_Test(void)
{
	printf("=======================%s=======================\n", __func__);
	printf("write and read lock disable\n");
	BPK_ReadUnlockFun();
	BPK_WriteUnlockFun();
	clearBPK();
    setBPK();
	printf("write lock enable\n");
	BPK_WriteLockFun();
	clearBPK();
}



void writeBPK(unsigned char index,unsigned int dat)
{
	
	
  BPK_KeyWriteLock(0x000F, DISABLE);
	BPK_Lock(BPK_LOCK_KeyWriteLock | BPK_LOCK_KeyReadLock, DISABLE);
	BPK_ReadUnlockFun();
	BPK_WriteUnlockFun();
	BPK_WriteKey(&dat,1,index);
	BPK_WriteLockFun();
	BPK_KeyWriteLock(0x000F, ENABLE);
	BPK_Lock(BPK_LOCK_KeyWriteLock | BPK_LOCK_KeyReadLock, ENABLE);
	
}	


unsigned int readBPK(unsigned char index)
{
	unsigned int ret;
	unsigned char sta = 0;  
  BPK_ReadUnlockFun();
  BPK_ReadKey(&ret,1, index); 
  BPK_ReadLockFun();
	return ret;
}
