////AT24C512内部有512页，每页为128字节，任意单元的地址为16位
////地址范围为0000H-FFFFH，可按页或字节显示

#include "24cxx.h"
#include "delay.h"
#include "delay.h"//基本外设头文件
#include "mhscpu.h"
#include "psamAPI.h"
#include "stdio.h"


STTERMINALPARAM  stTerminalParam;//终端参数


#if 0
void iic_dir_set(GPIO_TypeDef* GPIOx,unsigned short GPIO_Pin, unsigned char in_or_out)
{
    GPIO_InitTypeDef         GPIO_InitStructure;
    //===================================================
    // iic SCL或SDA引脚选择  SCL==P14       SDA==PC15
    //===================================================
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    //===================================================
    //   输入或输出选择
    //===================================================
    if (in_or_out == 0)//输出
    {
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Remap = GPIO_Remap_1;
        GPIO_Init(GPIOx, &GPIO_InitStructure);
    }
    else//输入
    {
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
        GPIO_InitStructure.GPIO_Remap = GPIO_Remap_1;
        GPIO_Init(GPIOx, &GPIO_InitStructure);
    }
}
#endif



//初始化24C512接口
void IIC_Init(void)
{
    //========================================
    // SCL = PC14   SDA=PC15
    //========================================
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    IIC_SCL_H;
    IIC_SDA_H;

}

//产生IIC起始信号
void IIC_Start(void)
{
    SDA_OUT();//sda线输出
    IIC_SDA_H;
    IIC_SCL_H;
    nop;
    IIC_SDA_L;//START:when CLK is high,DATA change form high to low
    nop;
    IIC_SCL_L;//钳住I2C总线，准备发送或接收数据
}

//产生IIC停止信号
void IIC_Stop(void)
{
    SDA_OUT();//sda线输出
    IIC_SCL_L;
    IIC_SDA_L;//STOP:when CLK is high DATA change form low to high
    nop;
    IIC_SCL_H;
    nop;
    IIC_SDA_H;//发送I2C总线结束信号
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
unsigned char IIC_Wait_Ack(void)
{
    unsigned char ucErrTime=0;
    SDA_IN();      //SDA设置为输入
    IIC_SDA_H;
    nop;
    IIC_SCL_H;
    nop;
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL_L;//时钟输出0
    return 0;
}

//产生ACK应答
void IIC_Ack(void)
{
    IIC_SCL_L;
    SDA_OUT();
    IIC_SDA_L;
    nop;
    IIC_SCL_H;
    nop;
    IIC_SCL_L;
}

//不产生ACK应答
void IIC_NAck(void)
{
    IIC_SCL_L;
    SDA_OUT();
    IIC_SDA_H;
    nop;
    IIC_SCL_H;
    nop;
    IIC_SCL_L;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(unsigned char txd)
{
    unsigned char t;
    SDA_OUT();
    IIC_SCL_L;//拉低时钟开始数据传输
    for(t=0; t<8; t++)
    {
        if((txd&0x80)>>7)IIC_SDA_H;
        else IIC_SDA_L;
        txd<<=1;
        nop;   //对TEA5767这三个延时都是必须的
        IIC_SCL_H;
        nop;
        IIC_SCL_L;
        nop;
    }
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
unsigned char IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA设置为输入
    for(i=0; i<8; i++ )
    {
        IIC_SCL_L;
        nop;
        IIC_SCL_H;
        receive<<=1;
        if(READ_SDA)receive++;
        nop;
    }
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK
    return receive;
}

//初始化IIC接口
void AT24CXX_Init(void)
{
    IIC_Init();
}

//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址
//返回值  :读到的数据
unsigned char AT24CXX_ReadOneByte(unsigned short ReadAddr)
{
    unsigned char temp=0;
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	   //发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(ReadAddr>>8);//发送高地址
    } else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据
    IIC_Wait_Ack();
    IIC_Send_Byte(ReadAddr%256);   //发送低地址
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(0XA1);           //进入接收模式
    IIC_Wait_Ack();
    temp=IIC_Read_Byte(0);
    IIC_Stop();//产生一个停止条件
    return temp;
}

//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(unsigned short WriteAddr,unsigned char DataToWrite)
{
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	    //发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr>>8);//发送高地址
    }
    else IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据
    IIC_Wait_Ack();
    IIC_Send_Byte(WriteAddr%256);   //发送低地址
    IIC_Wait_Ack();
    IIC_Send_Byte(DataToWrite);     //发送字节
    IIC_Wait_Ack();
    IIC_Stop();//产生一个停止条件
    //delay_us(100);//在os里面,这里必须用delay us 代替
}

//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(unsigned short WriteAddr,unsigned int DataToWrite,unsigned char Len)
{
    unsigned char t;
    for(t=0; t<Len; t++)
    {
        AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
    }
}

//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址
//返回值     :数据
//Len        :要读出数据的长度2,4
unsigned int AT24CXX_ReadLenByte(unsigned short ReadAddr,unsigned char Len)
{
    unsigned char t;
    unsigned int temp=0;
    for(t=0; t<Len; t++)
    {
        temp<<=8;
        temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1);
    }
    return temp;
}

//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
unsigned char AT24CXX_Check(void)
{
    unsigned char temp;
    temp=AT24CXX_ReadOneByte(65535);//避免每次开机都写AT24CXX
    if(temp==0X55)return 0;
    else//排除第一次初始化的情况
    {
        AT24CXX_WriteOneByte(65535,0X55);
        temp=AT24CXX_ReadOneByte(65535);
        if(temp==0X55)return 0;
    }
    return 1;
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
unsigned char AT24CXX_Read(unsigned short ReadAddr,unsigned char *pBuffer,unsigned short NumToRead)
{
    unsigned char temp=0;
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	   //发送写命令
        if(IIC_Wait_Ack())  return(0);
        IIC_Send_Byte(ReadAddr>>8);//发送高地址
    }
    else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据
    if(IIC_Wait_Ack())  return(0);
    IIC_Send_Byte(ReadAddr%256);   //发送低地址

    if(IIC_Wait_Ack())  return(0);
    IIC_Start();
    IIC_Send_Byte(0XA1);           //进入接收模式
    if(IIC_Wait_Ack())  return(0);
    while(NumToRead)
    {
        if(NumToRead==1) *pBuffer=IIC_Read_Byte(0);//读数据,发送nACK
        else *pBuffer=IIC_Read_Byte(1);		//读数据,发送ACK
        NumToRead--;
        pBuffer++;
    }

    IIC_Stop();//产生一个停止条件
    return 1;
}
//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
unsigned char AT24CXX_Write(unsigned short WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite)
{
    unsigned char i;
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	    //发送写命令
        if(IIC_Wait_Ack())  return(0);
        IIC_Send_Byte(WriteAddr>>8);//发送高地址
    }

    else IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据

    if(IIC_Wait_Ack())  return(0);
    IIC_Send_Byte(WriteAddr%256);   //发送低地址
    if(IIC_Wait_Ack())  return(0);
    for(i=0; i<NumToWrite; i++)
    {
        IIC_Send_Byte(*pBuffer);     //发送字节
        if(IIC_Wait_Ack())  return(0);
        pBuffer++;
    }
    IIC_Stop();//产生一个停止条件
    return(1);
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//
void SaveConfigDataNew( unsigned int StartAddr, unsigned char *s, unsigned int number )
{
    unsigned char a,b,i;

    if( (StartAddr%128) && ( ((StartAddr%128) + number)>128) )//待写数据 超过从设定地址开始的一页范围13-03-18
    {
        b=128-(StartAddr%128);

        AT24CXX_Write(StartAddr,s,b);

        delayms(10);//////////////////////////////  3-4  5

        StartAddr+=b;//更新地址
        s+=b;        //更新buf地址

        number=number-b;
        a=number/128;
        b=number%128;
    }
    else
    {
        a=number/128;
        b=number%128;
    }
    for(i=0; i<a; i++)
    {
        AT24CXX_Write(StartAddr+i*128,s+i*128,128);
        //delay(1);
        delayms(10);  //3-5
    }
    if(b)
    {
        AT24CXX_Write(StartAddr+a*128,s+a*128,b);
    }
    delayms(10); //3-5
    //delay(2);
    //delay_us(10000);//在os里面,这里必须用delay us 代替
}

////////////////////////////////////////////////////////////////////////////
void ReadConfigDataNew( unsigned int StartAddr, unsigned char *s, unsigned int number )
{
    unsigned char a,b,i;
    if( (StartAddr%128) && ( ((StartAddr%128) + number)>128) )//待写数据 超过从设定地址开始的一页范围13-03-18
    {
        b=128-(StartAddr%128);
        AT24CXX_Read(StartAddr,s,b);

        delayms(10);

        StartAddr+=b; //更新地址
        s+=b;         //更新buf地址

        number=number-b;
        a=number/128;
        b=number%128;
    }
    else
    {
        a=number/128;
        b=number%128;
    }

    for(i=0; i<a; i++)
    {
        AT24CXX_Read(StartAddr+i*128,s+i*128,128);
        //I2C_ReadNByte (CATaddr,TWO_BYTE_SUBA, StartAddr+i*128, s+i*128,128);
        delayms(10);
    }
    if(b)
    {
        AT24CXX_Read(StartAddr+a*128,s+a*128,b);
        //I2C_ReadNByte (CATaddr,TWO_BYTE_SUBA, StartAddr+a*128, s+a*128,b);
    }
    //delay_ms(1);

}

void SaveConfigData( unsigned int StartAddr, unsigned char *s, unsigned int number )
{
    delayms(20);
    SaveConfigDataNew( StartAddr, s, number );
}
//
void ReadConfigData( unsigned int StartAddr, unsigned char *s, unsigned int number )
{
    delayms(20);
    ReadConfigDataNew( StartAddr, s, number );
}

/*********************************************************************************************************
** Function name:       IIC_Test
** Descriptions:        串口测试函数
** 注意：在对24C256进行写数据时，每写完一页时要延时一下再进行下一页写
**       刚写完数据，不能立刻读数据，延时一下才可以读，否则读到的数据是不对的
*********************************************************************************************************/
unsigned char IIC_Test(void)
{

    int i;
    unsigned char temp[20]= {"01234567890123456789"};

    unsigned char testbuf[180]= {"\x00\xB2\xA0\x00\x00\x03\x33\x08\x20\x20\x12\x31\x01\x01\xB6\x16\x45\xED\xFD\x54\x98\xFB\x24\x64\x44\x03\x7A\x0F\xA1\x8C\x0F\x10\x1E\xBD\x8E\xFA\x54\x57\x3C\xE6\xE6\xA7\xFB\xF6\x3E\xD2\x1D\x66\x34\x08\x52\xB0\x21\x1C\xF5\xEE\xF6\xA1\xCD\x98\x9F\x66\xAF\x21\xA8\xEB\x19\xDB\xD8\xDB\xC3\x70\x6D\x13\x53\x63\xA0\xD6\x83\xD0\x46\x30\x4F\x5A\x83\x6B\xC1\xBC\x63\x28\x21\xAF\xE7\xA2\xF7\x5D\xA3\xC5\x0A\xC7\x4C\x54\x5A\x75\x45\x62\x20\x41\x37\x16\x96\x63\xCF\xCC\x0B\x06\xE6\x7E\x21\x09\xEB\xA4\x1B\xC6\x7F\xF2\x0C\xC8\xAC\x80\xD7\xB6\xEE\x1A\x95\x46\x5B\x3B\x26\x57\x53\x3E\xA5\x6D\x92\xD5\x39\xE5\x06\x43\x60\xEA\x48\x50\xFE\xD2\xD1\xBF\x03\xEE\x23\xB6\x16\xC9\x5C\x02\x65\x2A\xD1\x88\x60\xE4\x87\x87\xC0\x79\xE8\xE8\x5A\x72"} ;
    unsigned char temp1[300];
		
    SaveConfigDataNew(41984,testbuf,180);

		printf("AT24C512test!\n");	 
		
    ReadConfigDataNew(41984,testbuf,180);

		for(i=0;i<180;i++)printf("%.2x ",testbuf[i]); printf("\r\n");

    //if(temp1[9]==0x39) printf("IIC-Test-Well\n");

}


unsigned char E2PROM_Check()
{
   
   unsigned char temp[20]= {"01234567890123456789"};
	 unsigned char readbuf[20]={0}; 
   SaveConfigDataNew(0xF000,temp,20);
   ReadConfigDataNew(0xF000,readbuf,20);
	 if(Mystrcmp(temp,readbuf,20)!= 0)
	 {
	    
	   
	 
	 
	 }
	 

}

//eeprom
int getvar(unsigned char *obuf,int off,int len)
{
	
	ReadConfigData(off, obuf, len); 
	
	return 0;
}

int savevar(const char *ibuf,int off,int len)
{
	int i;
	
  SaveConfigData( off,(unsigned char *)ibuf, len ); 
	
	return 0;
}

/**
* @fn CalcLocalDesPubSaveVar
* @brief 保存信息至掉电保护区的
*			函数内部调用相关api并判断api执行的成功与否，
*			如api执行不成功，再次执行，若执行20次都不成功，
*			返回错误。
* @param in const char *psSaveBuf 		需保存内容的指针
* @param in const int nStart 			起始位置
* @param in const int nLen 				保存的长度
* @return li FILE_SUCC 成功
*		li FILE_FAIL   失败
*/

// savevar--> 保存变量 断电保护
int PubSaveVar (const  char *psSaveBuf, const int nStart, const int nLen)
{
    int nSaveTimes = 0;

    for (;;)
    {
        if (savevar(psSaveBuf, nStart, nLen) == 0)// eeprom
        {
            break;
        }
        else
        {
            nSaveTimes ++;
            if(20 == nSaveTimes)
            {
                return 1;
            }
        }
    }
    return 0;
}

/**
* @fn PubGetVar
* @brief  从掉电保护区的取信息。
*			函数内部调用相关api并判断api执行的成功与否，
*			如api执行不成功，再次执行，若执行20次都不成功，
*			返回错误。
* @param in char *psBuf	 			读出信息的指针
* @param in const int nStart 			起始位置
* @param in const int nLen 				读出的长度
* @return li		FILE_SUCC 成功
*		li FILE_FAIL   失败
*/
int PubGetVar (unsigned char *psBuf, const int nStart, const int nLen)
{
    int nSaveTimes = 0;

    for (;;)
    {
        if (getvar(psBuf, nStart, nLen) == 0)
        {
            break;
        }
        else
        {
            nSaveTimes ++;
            if(20 == nSaveTimes)
            {
                return 1;
            }
        }
    }
    return 0;
}



// 获取升级标识
unsigned char GetProgramUpStat(unsigned char *ProgramUpStat)
{
	 *ProgramUpStat=stTerminalParam.ProgramUpState;
}

//设置升级标识 
unsigned char SetProgramUpStat(unsigned char ProgramUpStat)
{
	stTerminalParam.ProgramUpState=ProgramUpStat;
	PubSaveVar(&ProgramUpStat,fPosProgram_off,fPosProgram_len); 
}

// 获取软件版本标识
unsigned char GetSoftVersionStat(unsigned char *softversion)
{
	 *softversion=(unsigned char)stTerminalParam.SoftVersion;
}

// 设置软件版本标识
unsigned char SetSoftVersionStat(unsigned char *softversion)
{
	PubSaveVar((const char *)softversion,fSoftVersion_off,fSoftVersion_len);  
}


//获取硬件版本标识 
unsigned char GetHardVersionStat(unsigned char *hardversion)
{
	
	*hardversion=(unsigned char)stTerminalParam.SoftVersion;
}

//设置硬件版本标识 
unsigned char SetHardVersionStat(unsigned char *hardversion)
{
	PubSaveVar((const char *)hardversion,fHardVersion_off,fHardVersion_len);  
}

//获取硬件错误标识 
unsigned char GetHardErrorStat(unsigned char *harderror)
{
	*harderror=(unsigned char)stTerminalParam.HardError;
}

//设置硬件错误标识 
unsigned char SetHardErrorStat(unsigned char *harderror)
{
	PubSaveVar((const char *)harderror,fHard_Erroroff,fHard_Error_len);  
}

// 读取参数
void ReadJRreaderTerminalParam(void)
{
	PubGetVar(&stTerminalParam.ProgramUpState,fPosProgram_off,fPosProgram_len);//参数
	PubGetVar(stTerminalParam.SoftVersion,fSoftVersion_off,fSoftVersion_len);//软件版本
	PubGetVar(stTerminalParam.HardVersion,fHardVersion_off,fHardVersion_len);//硬件版本
	PubGetVar(&stTerminalParam.HardError,fHard_Erroroff,fHard_Error_len);//硬件错误标志
}

void parameter_init(void)//参数初始化
{
  ReadJRreaderTerminalParam();//获取参数
  if(NULL==(strstr((const char*)stTerminalParam.SoftVersion,"jrs")))
  {
	   SetSoftVersionStat((unsigned char*)"jrs1.0");  
	}
  if(NULL==(strstr((const char*)stTerminalParam.HardVersion,"jrh")))
  {
	   SetHardVersionStat((unsigned char*)"jrh1.0");  
	}
}	











