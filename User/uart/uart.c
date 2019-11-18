#include <stdlib.h>
#include <stdio.h>
#include "uart.h"
#include "mhscpu.h"
#include "mhscpu_uart.h"
#include "type.h"
#include "delay.h"

uart_param g_uart_param;
Errorflag ProtocolFlag;

unsigned short uart_len=0;
unsigned char uart_buf[512]={0};

extern Packet receivePacket;


//支持 printf 函数
int fputc(int ch, FILE *f)
{
    /* Place your implementation of fputc here */
    /* e.g. write a character to the USART */
    if (ch == '\n')
    {
        fputc('\r', f);
    }
    while(!UART_IsTXEmpty(UART0));
    UART_SendData(UART0, (uint8_t) ch);
    return ch;
}

/**
 ****************************************************************
 * @brief uart_Config(uint32_t baudrate, uint32_t parity)
 *
 * 功能：串口初始化
 *
 * @param:baudrate = 波特率
 * @param:parity:奇偶校验位
 * @return: NONE
 *
 ****************************************************************/
void uart_Config(uint32_t baudrate, uint32_t parity)
{
    UART_InitTypeDef uart;
    NVIC_InitTypeDef nvic;
	
    uart.UART_BaudRate = baudrate;
    uart.UART_Parity = parity;
    uart.UART_StopBits = UART_StopBits_1;
    uart.UART_WordLength = UART_WordLength_8b;
    UART_Init(UART0, &uart);
    memset(&g_uart_param, 0, sizeof(g_uart_param));
	 
	  nvic.NVIC_IRQChannel = UART0_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvic);
	  

    GPIO_PinRemapConfig(GPIOA, GPIO_Pin_0 | GPIO_Pin_1, GPIO_Remap_0);

    //Set Enable FIFO Receive Threshold 1/2 Send Threshold 2 characters in the FIFO
    //UART0->OFFSET_8.FCR = (2 << 6) | (1 << 4) | BIT2 | BIT1 | BIT0;
		
    UART_ITConfig(UART0, UART_IT_RX_RECVD, ENABLE);
		
    NVIC_EnableIRQ(UART0_IRQn);
}


/**
 ****************************************************************
 * @brief protocolflag_init(void)
 *
 * 功能：清空所有与协议相关标志位 
 *
 * @return: NONE
 *
 ****************************************************************/
void protocolflag_init(void)
{
 memset(&ProtocolFlag,0,sizeof(ProtocolFlag));
 memset(&receivePacket,0,sizeof(receivePacket));
}	


void uart_cmd_Receive(unsigned char *buf,unsigned short *len)
{
	unsigned short rxlen=uart_len;
	unsigned short i=0;
	*len=0;				//默认为0
	
	delayus(100);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束 
	
	if(rxlen==uart_len&&rxlen)//接收到了数据,且接收完成了
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=uart_buf[i];	
		}		
		*len=uart_len;	//记录本次数据长度
		uart_len=0;		  //清零
	}
}


void UART0_IRQHandler(void)
{
    uint8_t Res;
    static unsigned char bits = 0;
    if(0x04==(UART0->OFFSET_8.IIR & 0x0F))//串口接收到有效数据
    {
        Res =UART_ReceiveData(UART0);	//读取接收到的数据
			  if(uart_len < 512) uart_buf[uart_len++] = Res;//串口接收的所有数据缓存	 
        if(ProtocolFlag.receive_ok !=0) return;//ProtocolFlag.receive_ok==0时 才开始接收数据
        if(ProtocolFlag.Head_flag)//数据头
        {
            if(ProtocolFlag.sn_flag)//序列号标志位
            {
                if(ProtocolFlag.unuse_flag)//预留字节标志位
                {
                    if(ProtocolFlag.command_flag)//命令 1字节
                    {
                        if(ProtocolFlag.datalen_flag)//数据长度标志位 2字节
                        {
                            if(ProtocolFlag.datalen == receivePacket.length)//接收到数据域所有数据
                            {
                                if(ProtocolFlag.crc16_flag)//crc 1字节
                                {
																		if(Res == 0x03)
																		{
																				receivePacket.tail = 0x03;	//数据结尾
																			  ProtocolFlag.receive_ok = 1;
																		}
																		else
																		{
																				protocolflag_init();//数据结尾错误 清0所有标志位			
																		} 
                                }
                                else
                                {
                                    receivePacket.crc16 = Res;
																	  //CRC校验正确 
																	  if(1)
																	 // if(receivePacket.crc16 == crc16(receivePacket.packet_buf,receivePacket.packrt_buf_len))
																		{
																		    ProtocolFlag.crc16_flag = 1;
																		
																		}
																		else
																		{
																		   protocolflag_init();//CRC校验错误 清0所有标志位		
																		}
                                }

                            }
                            else//接收所有数据 
                            {
                                receivePacket.data[ProtocolFlag.datalen++]= Res;
															  receivePacket.packet_buf[receivePacket.packrt_buf_len++]=Res;
                            }
                        }
                        else// 接收数据长度 2字节
                        {
                            bits++;
                            if(bits == 2)
                            {
                                bits = 0;
                                ProtocolFlag.datalen_flag = 1;//数据包长度接收完成标志位 置1
                                receivePacket.packet_buf[receivePacket.packrt_buf_len++]=Res;
															  receivePacket.length |=Res;
															  if(receivePacket.length >400) protocolflag_init();//长度错误 丢弃所有数据
															
															
                            }
                            else
                            {
                                receivePacket.length = Res*256;
															  receivePacket.packet_buf[receivePacket.packrt_buf_len++]=Res;
                            }
                        }
                    }
                    else//接收指令字节 1字节
                    {
                        ProtocolFlag.command_flag = 1;
                        receivePacket.command = Res;//指令
											  receivePacket.packet_buf[receivePacket.packrt_buf_len++]=Res;
                    }
                }
                else//接收预留字节 4字节
                {
										receivePacket.unuse[bits++]=Res;
										receivePacket.packet_buf[receivePacket.packrt_buf_len++]=Res;
										if(bits == 4)
										{
											bits = 0;
											ProtocolFlag.unuse_flag = 1;//数据包长度接收完成标志位 置1
										}
                }

            }
            else//接收到序列号 1字节
            {
                ProtocolFlag.sn_flag = 1;
                receivePacket.sn = Res;
                receivePacket.packet_buf[receivePacket.packrt_buf_len++]=Res; 
            }
        }
        else//接收到数据头
        {
            if(Res == 0x02)
            {
                ProtocolFlag.Head_flag = 1;//数据头标志位置1
                receivePacket.head = 0x02;
							  receivePacket.packet_buf[receivePacket.packrt_buf_len++]=Res;
            }
            else
            {
                protocolflag_init();//数据头错误 清0所有标志位						  
            }
        }
    }

}

//串口接收一个字节 阻塞方式
unsigned char UART0_GetChar()
{
    while(!UART_IsRXReceived(UART0));
    return UART_ReceiveData(UART0);
}
//串口发送一个字节 阻塞方式
void UART0_SendByte(unsigned char DAT)
{
    while(!UART_IsTXEmpty(UART0));
    UART_SendData(UART0, (uint8_t) DAT);
}

//串口发送大量字节 阻塞方式
void UART0_SendBuf(unsigned char *pbuf,unsigned short len)
{
	while(len--)
  {
	  UART0_SendByte(*(pbuf++)); 
	}
}

