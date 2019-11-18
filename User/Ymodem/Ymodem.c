#include "mhscpu.h"
#include <stdio.h>
#include <stdlib.h>//使用了atoi函数
#include "delay.h"
#include "uart.h"
#include "Ymodem.h"
#include "Iap_Flash.h"


unsigned char file_name[FILE_NAME_LENGTH];


/*************************************************************
  Function   : IAP_SerialSendByte 
  Description: 串口发送字节
  Input      : c-要发送的字节        
  return     : none    
*************************************************************/
void IAP_SerialSendByte(unsigned char c)
{
	while(!UART_IsTXEmpty(UART0));
	UART_SendData(UART0,c);
}

/*************************************************************
  Function   : IAP_SerialSendStr 
  Description: 串口发送字符串
  Input      : none        
  return     : none    
*************************************************************/
void IAP_SerialSendStr(unsigned char *s)
{
	while(*s != '\0')
	{
		IAP_SerialSendByte(*s);
		s++;
	}
}
/*************************************************************
  Function   : IAP_SerialGetByte 
  Description: 接收一个字节数据
  Input      : none        
  return     : 返回结果值，0-没有接收到数据；1-接收到数据    
*************************************************************/
unsigned char IAP_SerialGetByte(unsigned char *c)
{
  if(UART_IsRXReceived(UART0))
	{
	  *c = UART_ReceiveData(UART0);
    return 1;		
	}
  return 0; 		
}
/*************************************************************
  Function   : YModem_RecvByte 
  Description: ymodem接收一个字节
  Input      : c-存放接收到的字节 timeout-超时时间        
  return     : none    
*************************************************************/
signed char YModem_RecvByte(unsigned char *c, unsigned int timeout)
{
	while(timeout-- > 0)
	{
		if(IAP_SerialGetByte(c) == 1)
		{		
			return 0;
		}
 	
	}
	return -1;
}

/*************************************************************
  Function   : YModem_SendByte 
  Description: 发送一个字节
  Input      : c-要发送的字节        
  return     : none    
*************************************************************/
void YModem_SendByte(unsigned char c)
{
	IAP_SerialSendByte(c);
}

/*************************************************************
  Function   : IAP_GetKey 
  Description: 获取键入值
  Input      : none        
  return     : 返回键值    
*************************************************************/
unsigned char IAP_GetKey(void)
{
	unsigned char data;
	while(!IAP_SerialGetByte(&data)){ }
	return data;
}

/*************************************************************
  Function   : UpdateCRC16
  Description: 计算一个字节的CRC16校验码(CRC16-CCITT欧洲标准)
  Input      : crcIn-上一次的CRC码
               byte-一个字节
  return     : 返回crc码
*************************************************************/
unsigned int UpdateCRC16(uint16_t crcIn, uint8_t byte)
{
    uint32_t crc = crcIn;
    uint32_t in = byte|0x100;
    do
    {
        crc <<= 1;
        in <<= 1;
        if (in&0x100)
            ++crc;		//crc |= 0x01
        if (crc&0x10000)
            crc ^= 0x1021;
    }
    while (!(in&0x10000));
    return crc&0xffffu;
}
/*************************************************************
  Function   : Cal_CRC16
  Description: 计算数据的CRC码
  Input      : data-要计算的数据
               size-数据的大小
  return     : 返回计算出的CRC码
*************************************************************/
unsigned int Cal_CRC16(const uint8_t* data, uint32_t size)
{
    uint32_t crc = 0;
    const uint8_t* dataEnd = data+size;
    while (data<dataEnd)
        crc = UpdateCRC16(crc,*data++);
    crc = UpdateCRC16(crc,0);
    crc = UpdateCRC16(crc,0);
    return crc&0xffffu;
}
/*************************************************************
  Function   : Int2Str
  Description: 整型转化成字符串
  Input      : str-字符串指针 intnum-转换值
  return     : none
*************************************************************/
void YModem_Int2Str(unsigned char* str, int intnum)
{
    unsigned int i, Div = 1000000000, j = 0, Status = 0;

    for (i = 0; i < 10; i++)
    {
        str[j++] = (intnum / Div) + '0';//数字转化成字符

        intnum = intnum % Div;
        Div /= 10;
        if ((str[j-1] == '0') & (Status == 0))//忽略最前面的'0'
        {
            j = 0;
        }
        else
        {
            Status++;
        }
    }
}
/*************************************************************
  Function   : YModem_RecvPacket
  Description: 接收一个数据包
  Input      : data-存放接收到的数据
               length-数据包的长度
			   timeout-超时时间
  return     :  0 -正常接收  -1 -接收错误
*************************************************************/
signed char YModem_RecvPacket(unsigned char *data, int *length, unsigned int timeout)
{
    unsigned int i, packet_size;
    unsigned char c;
    unsigned int CRC1=0;
    *length = 0;
	  
    if(YModem_RecvByte(&c, timeout) != 0)//接收数据包的第一个字节
    {	  
        return -1;
    }
	
    switch(c)
    {
    case SOH:				//128字节数据包
        packet_size = PACKET_SIZE;	//记录数据包的长度
        break;
    case STX:				//1024字节数据包
        packet_size = PACKET_1K_SIZE;	//记录数据包的长度
        break;
    case EOT:				//数据接收结束字符
        return 0;   //接收结束
    case CA:				//接收中止标志
        if((YModem_RecvByte(&c, timeout) == 0) && (c == CA))//等待接收中止字符
        {
            *length = -1;		//接收到中止字符
            return 0;
        }
        else				//接收超时
        {
            return -1;
        }
    case ABORT1:				//用户终止，用户按下'A'
    case ABORT2:				//用户终止，用户按下'a'
        return 1;                       //接收终止
    default:
        return -1;                      //接收错误
    }
    *data = c;	                        //保存第一个字节
    for(i = 1; i < (packet_size + PACKET_OVERHEAD); i++)//接收数据
    {
        if(YModem_RecvByte(data + i, timeout) != 0)//接收接下来1024+5个字节
        {
            return -1;
        }
    }
    if(data[PACKET_SEQNO_INDEX] ==(~data[PACKET_SEQNO_COMP_INDEX]))//序号与序号的反
    {
        return -1;                   //接收错误
    }
    if(data[0]==SOH)
    {
        if(Cal_CRC16(&data[3],128)!=(unsigned int)((unsigned int)data[131]*256+data[132]))//CRC校验
        {
            return -1;
        }
    }
    if(data[0]==STX)
    {
        if(Cal_CRC16(&data[3],1024)!=(unsigned int)((unsigned int)data[1027]*256+data[1028]))//CRC校验
        {
            return -1;
        }
    }

    *length = packet_size;               //保存接收到的数据长度
    return 0;                            //正常接收
}

/*************************************************************
  Function   : YModem_Receive
  Description: ymodem接收
  Input      : buf-存放接收到的数据
  return     :  0 -发送端传输中止
               -1 -固件过大
			         -2 -flash烧写错误
			         -3 -用户终止
*************************************************************/
int YModem_Receive(unsigned char *buf)
{  
    unsigned char packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], file_size[FILE_SIZE_LENGTH];     	
	  unsigned char session_done, file_done, session_begin, packets_received, errors;
    unsigned char *file_ptr, *buf_ptr;
    int packet_length = 0;
	  unsigned int  size = 0;
    unsigned int i = 0,RamSource = 0;
    for (session_done = 0, errors = 0, session_begin = 0; ;)//死循环，一个ymodem连接
    {	    
        for (packets_received = 0, file_done = 0, buf_ptr = buf; ; )//死循环，不断接收数据
        {			 
            switch(YModem_RecvPacket(packet_data, &packet_length, NAK_TIMEOUT))//接收一个数据包
            {
            case 0:
                errors = 0;//返回0 表示一切正常
                switch(packet_length)
                {
                case -1: //发送端中止传输
                    YModem_SendByte(ACK);//回复ACK
                    return 0;
                case 0:	//接收结束或接收错误
                    YModem_SendByte(ACK);
                    file_done = 1;//接收完成
                    break;
                default: //接收数据中
                    if((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0xff))//数据包序号不对
                    {
                        YModem_SendByte(NAK);//接收错误的数据，回复NAK
                    }
                    else//接收到正确的数据
                    {
                        if(packets_received == 0)//接收第一帧数据
                        {
                            if(packet_data[PACKET_HEADER] != 0)//包含文件信息：文件名，文件长度等
                            {
                                for(i = 0, file_ptr = packet_data + PACKET_HEADER; (*file_ptr != 0) && (i < FILE_NAME_LENGTH); )
                                {//文件名从数据第三位开始到\0结束
                                    file_name[i++] = *file_ptr++;//保存文件名
                                }
                                file_name[i++] = '\0';//文件名以'\0'结束
																
                                for(i = 0, file_ptr++; (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH); )
                                {
                                    file_size[i++] = *file_ptr++;//保存文件大小
                                }
                                file_size[i++] = '\0';//文件大小以'\0'结束
                                size = atoi((const char *)file_size);//将文件大小字符串转换成整型
                                if(size > (0x80000-1))//升级固件过大 FLASH_SIZE=0x80000  512k
                                {
                                    YModem_SendByte(CA);
                                    YModem_SendByte(CA);//连续发送2次中止符CA
                                    return -1;//返回
                                }
                                IAP_FlashEease(size);//擦除相应的flash空间
                                IAP_UpdataParam(&size);//将size大小烧写进Flash中Parameter区
                                YModem_SendByte(ACK);//回复ACk
                                YModem_SendByte(CRC16);//发送'C',询问数据
                            }
                            else//文件名数据包为空，结束传输
                            {
                                YModem_SendByte(ACK);//回复ACK
                                file_done = 1;//停止接收
                                session_done = 1;//结束对话
                                break;
                            }
                        }
                        else //收到数据包
                        {
                            memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);//拷贝数据
                            RamSource = (unsigned int)buf;//8位强制转化成32为数据
                            if(IAP_UpdataProgram(RamSource, packet_length) != 0)        //烧写升级数据
                            {
                                YModem_SendByte(CA);
                                YModem_SendByte(CA);//flash烧写错误，连续发送2次中止符CA
                                return -2;//烧写错误
                            }
                            YModem_SendByte(ACK);//flash烧写成功，回复ACK
                        }
                        packets_received++;//收到数据包的个数
                        session_begin = 1;//设置接收中标志
                    }
                }
                break;
            case 1:		                //用户中止
                YModem_SendByte(CA);
                YModem_SendByte(CA);    //连续发送2次中止符CA
                return -3;		//烧写中止
            default:
                if(session_begin > 0)   //传输过程中发生错误
                {
                    errors++;
                }
                if(errors > MAX_ERRORS) //错误超过上限
                {
                    YModem_SendByte(CA);
                    YModem_SendByte(CA);//连续发送2次中止符CA
                    return 0;	//传输过程发生过多错误
                }
                YModem_SendByte(CRC16); //发送'C',继续接收
                break;
            }
            if(file_done != 0)//文件接收完毕，退出循环
            {
                break;
            }
        }
        if(session_done != 0)//对话结束，跳出循环
        {
            break;
        }
    }
    return (int)size;//返回接收到文件的大小
}


