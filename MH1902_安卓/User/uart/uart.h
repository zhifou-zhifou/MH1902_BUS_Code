
#ifndef MHSMCU_UART
#define MHSMCU_UART
#include "type.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>


#define RECV_INT  (BIT0)
#define SEND_INT  (BIT1 | BIT7)


#define UART_SEND_BUF_SIZE      256
#define UART_RECV_BUF_SIZE      256

    typedef struct uart_param_s
    {
        int     tx_transmited:1;        //数据传输结束标记，0表示正在传输数据；1表示传输结束，可以开始传输下一个字节
        int     baudrate;                //串口波特率
        struct {
            int read_index;             //中断发送索引
            int write_index;            //写缓冲区索引
            //u8 send_bytes;            //发送字节数
            //u8 send_int;              //产生发送完成中断的次数
            //u32 cnt;
            volatile int bytes;                 //缓冲区中的字节数
            uint8_t buf[UART_SEND_BUF_SIZE];    //发送缓冲区
        } send;
        struct {
            int read_index;             //读缓冲区索引
            int write_index;            //中断写缓冲区索引
            volatile int bytes;                 //缓冲区中的字节数
            //u32   cnt;            //接收字节数
            //u32   error_bytes;            //偶校验出错字节数
            uint8_t buf[UART_RECV_BUF_SIZE];    //接收缓冲区
            int overflow;           //接收缓冲区溢出标记
        } recv;
    } uart_param;

		
		typedef struct 
		{
			/*unsigned char  Head ;//数据头 
			unsigned char  Type;
      unsigned char  Instruction; 			
			unsigned char  DataLen;
			unsigned char  Tail;
			unsigned char  EndFlg;
			unsigned char  RecSucc;
			unsigned int   Num; //已经接收数据长度 
			unsigned char  BCC;//BCC 检验*/
			
			
			unsigned char  Head_flag;//数据头标志位
			unsigned char  sn_flag; //sn号 标志位
      unsigned char  unuse_flag;//预留字节标志位			
      unsigned char  command_flag;//命令类型	
      unsigned char  datalen_flag;//数据长度字节标志位			
			unsigned int   datalen;//已经接收数据长度
			unsigned char  crc16_flag;			
			unsigned char  tail;//结束标识符
			unsigned char  receive_ok;//接收完毕
			
		}Errorflag;

		
    void uart_Config(uint32_t baudrate, uint32_t parity);
   
    unsigned char UART0_GetChar(void);
    void UART0_SendByte(unsigned char DAT);
		void protocolflag_init(void);
		void uart_cmd_Receive(unsigned char *buf,unsigned short *len); 
    void UART0_SendBuf(unsigned char *pbuf,unsigned short len);

#ifdef __cplusplus
}
#endif

#endif

