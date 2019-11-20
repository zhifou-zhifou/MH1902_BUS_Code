#ifndef __24CXX_H
#define __24CXX_H
  
#include "rtc.h" 	

typedef struct
{
	unsigned char ProgramUpState;//升级标识	1
	unsigned char SoftVersion[7];//软件版本
	unsigned char HardVersion[7];//硬件版本
	unsigned char HardError;//硬件错误标志
	unsigned int  baudrate; //波特率
	
}STTERMINALPARAM;

extern  STTERMINALPARAM  stTerminalParam;//终端参数
 	   		   
//IO方向设置
#define SDA_IN()  {GPIOC->OEN |=1<<15;}	//PC15输入模式
#define SDA_OUT() {GPIOC->OEN &=~(1<<15);} //PC15输出模式
//IO操作函数	 
#define READ_SDA   GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15)  //输入SDA 


#define IIC_SCL_H  GPIO_SetBits(GPIOC,GPIO_Pin_14)
#define IIC_SCL_L  GPIO_ResetBits(GPIOC, GPIO_Pin_14)

#define IIC_SDA_H  GPIO_SetBits(GPIOC,GPIO_Pin_15)
#define IIC_SDA_L  GPIO_ResetBits(GPIOC, GPIO_Pin_15)

#define nop    __NOP 

//IIC所有操作函数
void IIC_Init(void);    //初始化IIC的IO口				 
void IIC_Start(void);		//发送IIC开始信号
void IIC_Stop(void);	  //发送IIC停止信号
void IIC_Send_Byte(unsigned char txd);	//IIC发送一个字节
unsigned char IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
unsigned char IIC_Wait_Ack(void); 			//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号 

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	  8191
#define AT24C128	16383
#define AT24C256	32767  
#define AT24C512	65535    //170228添加

//使用的是24C512，所以定义EE_TYPE为AT24C512
#define EE_TYPE AT24C512

#define fNVR_Addr_Start			0x100 // 参数存储起始地址
#define fPosCheck_off			fNVR_Addr_Start
#define fPosCheck_len			8

// 程序标识 
#define fPosProgram_off			fPosCheck_off+fPosCheck_len
#define fPosProgram_len			1

//软件版本
#define fSoftVersion_off		fPosProgram_off+fPosProgram_len	
#define fSoftVersion_len		6

//硬件版本
#define fHardVersion_off				fSoftVersion_off+fSoftVersion_len	
#define fHardVersion_len				6

//硬件错误标志位
#define fHard_Erroroff				fHardVersion_off+fHardVersion_len
#define fHard_Error_len				1


//预留
#define fFixValue_off		fIsLock_off+fIsLock_len
#define fFixValue_len		4

//预留
#define fTerminalId_off		fFixValue_off+fFixValue_len	
#define fTerminalId_len		8

//预留
#define fMerchantId_off		fTerminalId_off+fTerminalId_len	
#define fMerchantId_len		15


//预留
#define fTraceNo_off				fMerchantId_off+fMerchantId_len
#define fTraceNo_len				6  //2aê? μ??·?à′?D′è????¤


#define fNVR_Addr_End   fTraceNo_off+fReversalData_len

		  
unsigned char AT24CXX_ReadOneByte(unsigned short ReadAddr);							//指定地址读取一个字节
void AT24CXX_WriteOneByte(unsigned short WriteAddr,unsigned char DataToWrite);		//指定地址写入一个字节
void AT24CXX_WriteLenByte(unsigned short WriteAddr,unsigned int DataToWrite,unsigned char Len);//指定地址开始写入指定长度的数据
unsigned int AT24CXX_ReadLenByte(unsigned short ReadAddr,unsigned char Len);					//指定地址开始读取指定长度数据
unsigned char AT24CXX_Write(unsigned short WriteAddr,unsigned char *pBuffer,unsigned short NumToWrite);	//从指定地址开始写入指定长度的数据
unsigned char AT24CXX_Read(unsigned short ReadAddr,unsigned char *pBuffer,unsigned short NumToRead);   	//从指定地址开始读出指定长度的数据

unsigned char AT24CXX_Check(void);  //检查器件
void AT24CXX_Init(void); //初始化IIC


///////////////////////////////////////////////////
void SaveConfigDataNew( unsigned int StartAddr, unsigned char *s, unsigned int number );
void ReadConfigDataNew( unsigned int StartAddr, unsigned char *s, unsigned int number );
void SaveConfigData( unsigned int StartAddr, unsigned char *s, unsigned int number );
void ReadConfigData( unsigned int StartAddr, unsigned char *s, unsigned int number );
unsigned char IIC_Test(void);



unsigned char GetSoftVersionStat(unsigned char *softversion);// 获取软件版本标识


unsigned char SetSoftVersionStat(unsigned char *softversion);// 设置软件版本标识


unsigned char GetHardVersionStat(unsigned char *hardversion);//获取硬件版本标识 


unsigned char SetHardVersionStat(unsigned char *hardversion);//设置硬件版本标识 


unsigned char GetHardErrorStat(unsigned char *harderror);//获取硬件错误标识 


unsigned char SetHardErrorStat(unsigned char *harderror);//设置硬件错误标识 


void ReadJRreaderTerminalParam(void);// 读取参数

void parameter_init(void);//参数初始化

#endif



















