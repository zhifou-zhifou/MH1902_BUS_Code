#include "mhscpu.h"
#include "delay.h"
uint32_t fac_us=96;

void delayus(uint32_t usec)
{		
	uint32_t temp;	
	if(usec == 0)return;
	SysTick->LOAD=usec*fac_us; 				//时间加载	  		 
	SysTick->VAL=0x00;        				//清空计数器
	SysTick->CTRL|=(SysTick_CTRL_CLKSOURCE_Msk|SysTick_CTRL_ENABLE_Msk) ; //开始倒数 	 
	do
	{
		temp=SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//等待时间到达   
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; //关闭计数器
	SysTick->VAL =0X00;       				//清空计数器 
}




//__ASM void delayus(uint32_t usec)
//{
//    ALIGN
//    MOV r1,#24
//    MUL r0,r1
//    NOP
//    NOP
//    NOP
//loop
//    SUBS r0,#1
//    NOP
//    BNE loop
//    NOP
//    BX lr
//}

void delayms(uint32_t ms)
{
    if(ms==0)return;
    for(; ms>0; ms--)
    {
        delayus(1000);
    }
}








































