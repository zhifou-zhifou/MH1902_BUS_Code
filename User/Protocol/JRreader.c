#include "mh523.h"
#include "uart.h"
#include "stdio.h"

#include "debug.h"//调试接口 头文件

#include "jrreader.h"//金融读卡器协议头文件

#include "gpio.h"//Beep led控制

#include "24cxx.h"//E2PROM 
 
#include "iso14443_4.h"
#include "iso14443a.h"
#include "iso14443b.h"//射频卡相关头文件
#include "ContactlessAPI.h"//非接触式卡APDU接口头文件

#include "7816.h"
#include "mhscpu_sci.h"//
#include "iso7816_3.h"//接触式卡头文件
#include "emv_errno.h"
#include "emv_hard.h"

#include "Packet.h"
#include "define.h" //上位机通讯协议相关

#include "psam.h" //PSAM 头文件 
#include "psamAPI.h"//PSAM APDU接口头文件

#include "crc.h"//软件CRC算法

#include "rtc.h"//rtc时钟
#include "bpk.h"//备份寄存器
#include "Digital.h"//数码管
#include "led.h" //LED灯 
#include "key.h"//按键

extern Packet receivePacket;//上位机通信接收结构体
extern Packet sendPacket;//上位机通信发送结构体
extern MH523TypeDef MH523;//读卡芯片通信相关结构体
extern Errorflag ProtocolFlag;//协议接收标志位

unsigned char PCardSN[4]= {0}; //4位卡号
unsigned char CardIN = 0;
unsigned char KeyVoice = 1;//按键音


void UploadCardSn()
{
    unsigned char checkNum = 0;//CRC计算结果
    unsigned char status;
    unsigned char s[200];
    unsigned char Ctype;
    status = pro_GetCardID(PCardSN);
    if(status==0)
    {
        //	printf("挥卡上报--->\n");
        //CardIN = 1;//有卡片进场 
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x33;
        s[7]= 0x00;
        s[8]= 0x06;
        s[9]= RC_DATA;

        Ctype=CaptureCardType(); //返回卡类
        if(Ctype==M1_CARD)           s[10]=0x01;    //M1卡
        else if(Ctype==CPUAPP)       s[10]=0x02;    //纯行业卡
        else if(Ctype==UNION_PAY)    s[10]=0x03;    //纯银行卡
        else if(Ctype==UNIONPAYCPU)  s[10]=0x04;    //复合银行卡
        else
        {
            s[10]=0x0F;
        }		
        memcpy(s+11,PCardSN,4);
        checkNum = crc16(s+1,14);
        s[15]= checkNum;
        s[16]= 0x03;
        UART0_SendBuf(s,17);
    }
    else
    {
			 //CardIN = 0;
       //printf("no card \r\n");
    }
}
void SendKeyValue()
{
    unsigned char checkNum = 0;//CRC计算结果
    unsigned char KeyValue = 0;
    unsigned char s[50];
    
    KeyValue = GetKey();
	  if(KeyValue !=0)//有键按下
		{
		    s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x70;
        s[7]= 0x00;
        s[8]= 0x02;
        s[9]= 0x00;		    
			  switch(KeyValue)
				{
				  case 1:s[10]=0x35;break;//touch
				  case 2:s[10]=0x31;break;//down
				  case 3:s[10]=0x30;break;//up
				  case 4:s[10]=0x32;break;//ok
					case 5:s[10]=0x33;break;//fun
					case 6:s[10]=0x34;break;//dowm+fun
				}
		    checkNum = crc16(s+1,10);
		    s[11]= checkNum;
        s[12]= 0x03;
        UART0_SendBuf(s,13);
				if(KeyVoice)beep();//按键音可选
				
		}
}	

void SerJRReaderHandle(Packet *recPacket)
{

    unsigned char checkNum = 0;//CRC计算结果
    unsigned char Status=0;
    unsigned short ren=0;
    unsigned char  UARTRevbuf[300]= {0};
    unsigned char s[200]= {0};
		unsigned char i = 0;
		unsigned int cost = 0;
		//unsigned char UARTRevbuf[500],okflag=0;	
//    checkNum = crc16(recPacket->packet_buf,recPacket->packrt_buf_len);
//    if(checkNum != recPacket->crc16)
//    {
//        printf("crc error! rec:%02X now:%02X\r\n",recPacket->crc16,checkNum);
//        return;
//    }
    switch(recPacket->command)
    {
    case 0x08://ECHO
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"银通",4);
        s[6]=0x08;
        s[7]= recPacket->length>>8;
        s[8]= recPacket->length & 0xFF;
        memcpy(s+10,recPacket->data,recPacket->length);
        checkNum = crc16(s+1,9+recPacket->length);//CRC校验
        s[10+recPacket->length]= checkNum;
        s[11+recPacket->length]= 0x03;
        UART0_SendBuf(s,12+recPacket->length);
        break;
    case 0x30:
        ;
        break;
    case 0x34:
        if(recPacket->data[0] == 0x04)//非接卡
        {
					  //ouputRes("APDU:",recPacket->data,recPacket->length);
            Status = pro_APDU(&recPacket->data[1],recPacket->length-1,UARTRevbuf,&ren);
            if(Status==0)s[9]= RC_DATA;
            else s[9]=RC_FAILURE;
            s[0]=STX;
            s[1]=0x01;
            memcpy(s+2,"银通",4);
            s[6]=0x34;
            s[7]= (ren+1)>>8;
            s[8]= ren+1;
            memcpy(s+10,UARTRevbuf,ren);
            checkNum = crc16(s+1,9+ren);//CRC校验
            s[10+ren]= checkNum;
            s[11+ren]= 0x03;
            //Uart1Send(s,12+ren);
            UART0_SendBuf(s,12+ren);
        }
        else
        {
            if(recPacket->data[0]==1)//卡槽1 
            {
                selPsam=1;
            }
            else if(recPacket->data[0]==2)//卡槽2
            {
                selPsam=2;
            }
            else if(recPacket->data[0]==3)//卡槽3 硬件PSAM卡接口
            {
                selPsam=3;
            }
            else
            {
                printf("error psam num!\r\n");
            }
										
						//cost = get_tick();
						Status=pro_APDU_PSAM(&recPacket->data[1],recPacket->length-1,UARTRevbuf,&ren);		
						//printf("cost1=%dms\r\n",get_tick()-cost);
            if(Status==0)s[9]= RC_DATA;
            else s[9]=RC_FAILURE;

            s[0]=STX;
            s[1]=0x01;
            memcpy(s+2,"\x0E\x01\x0b\x01",4);
            s[6]=0x34;
            s[7]= (ren+1)>>8;
            s[8]= ren+1;
            memcpy(s+10,UARTRevbuf,ren);
            checkNum = crc16(s+1,9+ren);
            s[10+ren]= checkNum;
            s[11+ren]= 0x03;
            UART0_SendBuf(s,12+ren);
        }
        break;
				
    case 0x35://接触式卡 上电
        if(recPacket->data[0]==1)//卡槽1
        {
            selPsam=1;
        }
        else if(recPacket->data[0]==2)//卡槽2
        {
            selPsam=2;
        }
        else if(recPacket->data[0]==3)//卡槽3
        {
            selPsam=3;
        }
        else
        {
            printf("no psam num!\r\n");
        }
				if(selPsam ==1 || selPsam ==2 )
				{
					if(psam_reset(selPsam))
					{
							s[9]=RC_DATA;
					}
					else s[9]=RC_FAILURE;
			  }
				else if(selPsam == 3)//硬件PSAM卡接口
				{
				  if(tst_SCIWarmReset(0)==0)
					{
					   memcpy(psam_atr_tab,&atr_buf[1],atr_buf[0]);//atr
						 psam_atr_len = atr_buf[0];//atr 长度
					   s[9]=RC_DATA;
					}
					else
					{
					   s[9]=RC_FAILURE;			
					}
				}
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x35;
        s[7]= (psam_atr_len+1)>>8;
        s[8]= psam_atr_len+1;

        memcpy(s+10,psam_atr_tab,psam_atr_len);

        checkNum = crc16(s+1,9+psam_atr_len); //′í?ó??

        s[10+psam_atr_len]=checkNum;
        s[11+psam_atr_len]= 0x03;
        UART0_SendBuf(s,12+psam_atr_len);
        break;
				
    case 0x36: // 开启/关闭读卡器 （开射频天线）
        if(recPacket->data[0]==0x01)
        {
            pcd_antenna_on();//开启天线
            beep();
            // delay_ms(30);
        }
        else if(recPacket->data[0]==0x02)
        {
            pcd_antenna_off();//关闭天线
            beep();
        }
        else
        {
            printf("36--->\n");
        }
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x36;
        s[7]= 0x00;
        s[8]= 0x01;
        s[9]= RC_DATA;
        checkNum = crc16(s+1,9);
        s[10]=checkNum;
        s[11]= 0x03;
        UART0_SendBuf(s,12);
        break;
				
		case 0x37:// 蜂鸣器测试		
				for(i = 0;i<recPacket->data[0];i++)
				{
				  beep();
				}	
        break;
		case 0x40:// 数码管显示设置
				if(recPacket->data[0]==0x30)//数码管
        {
            dig_Display(recPacket->data[1]<<8|recPacket->data[2]);//显示数值 高8位在前
        }
        else
        {
            printf("dig error--->\n");
        }
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x40;
        s[7]= 0x00;
        s[8]= 0x01;
        s[9]= RC_SUCCESS;
        checkNum = crc16(s+1,9);
        s[10]=checkNum;
        s[11]= 0x03;
        UART0_SendBuf(s,12);
        break;			
		case 0x43://获取时间
		    s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"银通",4);
        s[6]=0x45;
        s[7]= 0x00;
        s[8]= 0x08;//返回数据长度
		    s[9] = RC_SUCCESS;
		    s[10]= calendar.w_year >> 8;
        s[11]= calendar.w_year & 0xFF;	
        s[12]= calendar.w_month;
		    s[13]= calendar.w_date;
		    s[14]= calendar.hour;
        s[15]= calendar.min;	
		    s[16]= calendar.sec;
		    checkNum = crc16(s+1,16);
        s[17]=checkNum;
        s[18]= 0x03;
		     UART0_SendBuf(s,19);		
		break;			
		case 0x44://设置时间
			  writeBPK(1,0x80808080);
			  RTC_Set(recPacket->data[0]<<8|recPacket->data[1],recPacket->data[2],recPacket->data[3],\
		    recPacket->data[4],recPacket->data[5],recPacket->data[6]);
		    s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"银通",4);
        s[6]=0x44;
        s[7]= 0x00;
        s[8]= 0x01;//返回数据长度
			  s[9]= RC_SUCCESS;	
        checkNum = crc16(s+1,9);
				s[10]=checkNum;
				s[11]= 0x03;
				UART0_SendBuf(s,12);
		break;
		
		case 0x45://获取参数
        ReadJRreaderTerminalParam();//获取参数
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"银通",4);
        s[6]=0x45;
        s[7]= 0x00;
        s[8]= 0x17;//返回数据长度
        s[9]= RC_DATA;
		    s[10] =0x00;
		    s[11]= 0x01;
		    s[12]= 0x06;	
		    memcpy(&s[13],stTerminalParam.SoftVersion,fSoftVersion_len);//软件版本
		    s[19]= 0x00;
		    s[20]= 0x02;
		    s[21]= 0x06;
		    memcpy(&s[22],stTerminalParam.HardVersion,fHardVersion_len);//硬件版本
		    s[28]= 0x00;
		    s[29]= 0x03;
		    s[30]= 0x01;
		    s[31]=stTerminalParam.HardError;
		    checkNum = crc16(s+1,31);
        s[32]=checkNum;
        s[33]= 0x03;
        UART0_SendBuf(s,34);	
		 break;		
		 
		case 0x46:	//设置软件版本
		    s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"银通",4);
        s[6]=0x46;
		    s[7]= 0x00;
			  s[8]= 0x01;//返回数据长度
		   if(((recPacket->data[0]<<8)+recPacket->data[1])==1)//只有1个参数
			 {
			    if(recPacket->data[2]==0x00 && recPacket->data[3] == 0x01)//设置软件版本
					{
						SetSoftVersionStat(&recPacket->data[5]);//设置参数	
						ReadJRreaderTerminalParam();//获取参数
						if(0!=(Mystrcmp(stTerminalParam.SoftVersion,&recPacket->data[5],recPacket->data[4])))
						{
                s[9]= RC_FAILURE;	
						}
						else
						{
						    s[9]= RC_SUCCESS;
						}	
					}
					else if(recPacket->data[2]==0x00 && recPacket->data[3] ==0x02)
					{
						SetHardVersionStat(&recPacket->data[5]);	
						ReadJRreaderTerminalParam();//获取参数
						if(0!=(Mystrcmp(stTerminalParam.HardVersion,&recPacket->data[5],recPacket->data[4])))
						{
                s[9]= RC_FAILURE;	
						}
						else
						{
						    s[9]= RC_SUCCESS;
						}	
					}
			 }
       else if(((recPacket->data[0]<<8)+recPacket->data[1])==2)//2个参数
			 {   
			    if(recPacket->data[2]==0x00 && recPacket->data[3] ==0x01)//设置软件版本
					{
						SetSoftVersionStat(&recPacket->data[5]);	
					}
					if(recPacket->data[11]==0x00 && recPacket->data[12] ==0x02)
					{
						SetHardVersionStat(&recPacket->data[14]);	
					}	
          ReadJRreaderTerminalParam();//获取参数
					if(0!=(Mystrcmp(stTerminalParam.SoftVersion,&recPacket->data[5],recPacket->data[4])))
				  {
                s[9]= RC_FAILURE;	
					}
					else
				  {
						if(0!=(Mystrcmp(stTerminalParam.HardVersion,&recPacket->data[14],recPacket->data[13])))
						{
                s[9]= RC_FAILURE;	
						}
						else
						{
						    s[9]= RC_SUCCESS;
						}		
					}		
			 }
			 checkNum = crc16(s+1,9);
			 s[10]=checkNum;
			 s[11]= 0x03;
			 UART0_SendBuf(s,12);	 
		break;
		case 0x52://获取串口波特率
		    stTerminalParam.baudrate =115200;
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x52;
		    if(stTerminalParam.baudrate == 115200)
				{
				  s[10]=0x00;
				}
			  else if(stTerminalParam.baudrate == 57600)
				{
				  s[10]=0x01;
				}
		    else if(stTerminalParam.baudrate == 38400)
				{
				  s[10]=0x02;
				}
			  else if(stTerminalParam.baudrate == 28800)
				{
				  s[10]=0x03;
				}		
			  else if(stTerminalParam.baudrate == 19200)
				{
				  s[10]=0x04;
				}	
        else
        {
				  s[7]= 0x00;
					s[8]= 0x01;//返回数据长度
					s[9]= RC_FAILURE;	
          checkNum = crc16(s+1,9);
					s[10]=checkNum;
					s[11]= 0x03;
					UART0_SendBuf(s,12);	
          break;					
				}					
				s[7]= 0x00;
				s[8]= 0x02;//返回数据长度
				s[9]= RC_SUCCESS;	
        checkNum = crc16(s+1,10);
				s[11]=checkNum;
				s[12]= 0x03;
				UART0_SendBuf(s,13);
		    break; 	
		case 0x53:
			  ReadJRreaderTerminalParam(); 
		    s[0]=STX;
				s[1]=0x01;
				memcpy(s+2,"银通",4);
				s[6]=0x53;
        if(recPacket->data[0] == 0x00)
				{
				  stTerminalParam.baudrate = 115200;
				}
			  else if(recPacket->data[0] == 0x01)
				{
				  stTerminalParam.baudrate = 57600;
				}
		    else if(recPacket->data[0] == 0x02)
				{
				  stTerminalParam.baudrate = 38400;
				}
			  else if(recPacket->data[0] == 0x03)
				{
				  stTerminalParam.baudrate = 28800;
				}		
			  else if(recPacket->data[0] == 0x04)
				{
				  stTerminalParam.baudrate = 19200;
				}	
        else
        {
				  s[7]= 0x00;
					s[8]= 0x01;//返回数据长度
					s[9]= RC_FAILURE;	
          checkNum = crc16(s+1,9);
					s[10]=checkNum;
					s[11]= 0x03;
					UART0_SendBuf(s,12);	
          break;					
				}	
				//SetJRreaderTerminalParam(&stTerminalParam);
				s[7]= 0x00;
			  s[8]= 0x01;
			  checkNum = crc16(s+1,9);
				s[9]= RC_SUCCESS;	
        s[10]=checkNum;
        s[11]= 0x03;
        UART0_SendBuf(s,12);
				uart_Config(stTerminalParam.baudrate, UART_Parity_No);//重新配置 bps
        break;
								
//11 09 00 00 03 00 00a1 A00000000301201212310101C696034213D7D8546984579D1D0F0EA519CFF8DE0
//命令(1)+卡组织ID(4)+索引(1)+长度(2)
		case 0x61:		
       if(recPacket->data[0] == 0x11)//设置CA
			 {
			   if(recPacket->data[3] == 0x00 && recPacket->data[4]==0x03)
				 {
				   SaveConfigData(0x8000+recPacket->data[5]*0x200,&recPacket->data[6],recPacket->data[6]*256+recPacket->data[7]+2);
		       ReadConfigData(0x8000+recPacket->data[5]*0x200,&UARTRevbuf[0],recPacket->data[6]*256+recPacket->data[7]+2);	 
//					for(i=0;i<UARTRevbuf[0]*256+UARTRevbuf[1]+2;i++)
//					{
//					   printf("%.2X ",UARTRevbuf[i+3]);
//					}
					if(Mystrcmp(UARTRevbuf,&recPacket->data[6],recPacket->data[6]*256+recPacket->data[7]+2)==0)
					{			
					   // printf("CA-SUC\n");
						 s[9] = RC_SUCCESS;
					}
					else
					{
					   s[9] = RC_FAILURE;
					}
				 }
         else
				 {
				     s[9] = RC_FAILURE;
				 }					 
			 }
			 else
			 {
			   s[9] = RC_FAILURE;
			 }
			  s[0]=STX;
				s[1]=0x01;
				memcpy(s+2,"银通",4);
				s[6]=0x61;
				s[7]= 0x00;
			  s[8]= 0x01;
			  checkNum = crc16(s+1,9);
        s[10]=checkNum;
        s[11]= 0x03;
        UART0_SendBuf(s,12);
       break;
		case 0x68:
			
//AID参数从 27 开始存		
//11 09 00 00 00 01 27 A0000000032010000140D84000A800D84004F8000010000000000000000000000099999F370400		
		  if(recPacket->data[0] == 0x11)//设置AID
			{
					if(recPacket->data[3] == 0x00 && recPacket->data[4]==0x00)
					{
						if(recPacket->data[6]==0x27)
						{
							SaveConfigData(0x7000+recPacket->data[5]*0x40,&recPacket->data[6],40);
							ReadConfigData(0x7000+recPacket->data[5]*0x40,&UARTRevbuf[0],40);
							if(Mystrcmp(UARTRevbuf,&recPacket->data[6],40)==0)
							{
								s[9] = RC_SUCCESS;
							}
						}
						else if(recPacket->data[6]==0x28)
						{
							SaveConfigData(0x7000+recPacket->data[5]*0x40,&recPacket->data[6],41);
							ReadConfigData(0x7000+recPacket->data[5]*0x40,&UARTRevbuf[0],41);
							if(Mystrcmp(UARTRevbuf,&recPacket->data[6],41)==0)
							{
								s[9] = RC_SUCCESS;
							}
						}
						else if(recPacket->data[6]<=0x40)//14-08-04
						{
							SaveConfigData(0x7000+recPacket->data[5]*0x40,&recPacket->data[6],recPacket->data[6]);
							ReadConfigData(0x7000+recPacket->data[5]*0x40,&UARTRevbuf[0],recPacket->data[6]);
							if(Mystrcmp(UARTRevbuf,&recPacket->data[3],recPacket->data[6])==0)
							{
								s[9] = RC_SUCCESS;
							}
						}					 
				 }
         else
				 {
				     s[9] = RC_FAILURE;
				 }					 
			 }
			 else
			 {
			   s[9] = RC_FAILURE;
			 }
				s[0]=STX;
				s[1]=0x01;
				memcpy(s+2,"银通",4);
				s[6]=0x68;
				s[7]= 0x00;
				s[8]= 0x01;
				checkNum = crc16(s+1,9);
				s[10]=checkNum;
				s[11]= 0x03;
				UART0_SendBuf(s,12);
				break;		

      case 0x71:// LED
				if(recPacket->data[1]==0x30)//指示灯
        {
					switch(recPacket->data[3])//选择某个LED
					{
						case 0x30: LED_Control(LED1,recPacket->data[2]-0x30);break;
					  case 0x31: LED_Control(LED2,recPacket->data[2]-0x30);break;
						case 0x32: LED_Control(LED3,recPacket->data[2]-0x30);break;
						default:break;
					}         
        }
        else if(recPacket->data[1]==0x31)//补光灯
        {
            LED_Control(BGLED,recPacket->data[2]-0x30);//控制补光灯
        }
				else
				{
				  printf("led type error--->\n");			
				}
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x71;
        s[7]= 0x00;
        s[8]= 0x01;
        s[9]= RC_SUCCESS;
        checkNum = crc16(s+1,9);
        s[10]=checkNum;
        s[11]= 0x03;
        UART0_SendBuf(s,12);
        break;	
		case 0x72:// 光敏 按键声音控制
				if(recPacket->data[1]==0x31)//光敏
        {
       
        }
        else if(recPacket->data[1]==0x32)//按键音
        {
          if(recPacket->data[2]==0x30)  KeyVoice = 1;
					else if(recPacket->data[2]==0x31) KeyVoice = 0;//关闭按键音		
        }
        s[0]=STX;
        s[1]=0x01;
        memcpy(s+2,"\x0E\x01\x0b\x01",4);
        s[6]=0x72;
        s[7]= 0x00;
        s[8]= 0x01;
        s[9]= RC_SUCCESS;
        checkNum = crc16(s+1,9);
        s[10]=checkNum;
        s[11]= 0x03;
        UART0_SendBuf(s,12);
        break;	
	 
		} 
}  	






