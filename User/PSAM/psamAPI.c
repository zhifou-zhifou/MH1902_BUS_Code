#include "psam.h"
#include "psam_bsp.h"//PSAM卡 底层驱动文件

#include "string.h"
#include "stdio.h"//C库函数 头文件

#include "mhscpu.h"
#include "delay.h"//软件延时函数

#include "7816.h"//硬件7816接口

#include "psamAPI.h"

#include "debug.h"

#include "24cxx.h"

//PSAM卡槽选择位   1，2 = 普通PSAM卡  3=互联互通PSAM卡
extern volatile unsigned char selPsam;

ISO7816_ADPU_Commands SC_ADPU;
ISO7816_ADPU_Responce SC_Responce;
extern  ST_APDU_REQ apdu_req;
extern  ST_APDU_RSP apdu_rsp;

unsigned char AID_index=0,AID_indexcmp=0,DID_delay=0;
unsigned char  PsamSendBuf[200],PSAMSendLen=0;

unsigned char pro_APDU_PSAM(unsigned char *sendbuf,unsigned short sendlen,unsigned char *recebuf,unsigned short *recelen)
{
    unsigned char status = 0xFF;
    unsigned short i = 0;
    unsigned int cost = 0;

    if(selPsam == 3)//硬件 智能卡 接口
    {

        memset(&apdu_req,0,sizeof(apdu_req));
        memset(&apdu_rsp,0,sizeof(apdu_rsp));
        if (sendlen > 5)
        {
            apdu_req.cmd[0] = sendbuf[0];
            apdu_req.cmd[1] = sendbuf[1];
            apdu_req.cmd[2] = sendbuf[2];
            apdu_req.cmd[3] = sendbuf[3];
            apdu_req.lc = sendbuf[4];

            memcpy(apdu_req.data_in, &sendbuf[5], apdu_req.lc);

            if(apdu_req.lc+5 != sendlen )//包含Le
            {
                apdu_req.le = sendbuf[5 + apdu_req.lc];
            }
            else//不包含le 仍需要添加le = 0
            {
                apdu_req.le  = 0;
            }

        }
        else if(sendlen == 5 )
        {

            apdu_req.cmd[0] = sendbuf[0];
            apdu_req.cmd[1] = sendbuf[1];
            apdu_req.cmd[2] = sendbuf[2];
            apdu_req.cmd[3] = sendbuf[3];
            apdu_req.lc = 0;
            apdu_req.le = sendbuf[4];
        }
        else//APDU长度错误 直接返回错误
        {
            return status;
        }
        //cost = get_tick();
        status = iso7816_exchange(0, 1, &apdu_req, &apdu_rsp);
        //printf("cost=%dms\r\n",get_tick()-cost);
        if (status)
        {
            //printf("7816exchange=%d\n", status);
            return status;
        }
        //printf("7816=%ld\r\n",get_tick());
        //==============================================================================
        // 获取卡应答数据
        //==============================================================================
        memcpy(recebuf, apdu_rsp.data_out, apdu_rsp.len_out);
        *recelen = apdu_rsp.len_out+2;
        recebuf[apdu_req.le + 0] = apdu_rsp.swa;
        recebuf[apdu_req.le + 1] = apdu_rsp.swb;

    }
    else//软件模拟  PSAM卡
    {
//        memset(&apdu_req,0,sizeof(apdu_req));
//        memset(&apdu_rsp,0,sizeof(apdu_rsp));
//        if (sendlen > 5)
//        {
//            SC_ADPU.Header.CLA = sendbuf[0];
//            SC_ADPU.Header.INS = sendbuf[1];
//            SC_ADPU.Header.P1  = sendbuf[2];
//            SC_ADPU.Header.P2  = sendbuf[3];
//            SC_ADPU.Body.LC    = sendbuf[4];
//            if (SC_ADPU.Body.LC && (SC_ADPU.Body.LC < LCmax)
//                    && (SC_ADPU.Body.LC == sendlen - 6))
//            {
//                memcpy(SC_ADPU.Body.Data, &sendbuf[5], SC_ADPU.Body.LC);
//                SC_ADPU.Body.LE = sendbuf[5 + SC_ADPU.Body.LC];
//            }
//            else
//            {
//                SC_ADPU.Body.LE = sendbuf[5];
//            }
//        }
//        else if(sendlen ==5 )
//        {

//            SC_ADPU.Header.CLA = sendbuf[0];
//            SC_ADPU.Header.INS = sendbuf[1];
//            SC_ADPU.Header.P1  = sendbuf[2];
//            SC_ADPU.Header.P2  = sendbuf[3];
//            SC_ADPU.Body.LC    = 0;
//            SC_ADPU.Body.LE    = sendbuf[4];
//        }
//        else//APDU长度错误 直接返回错误
//        {
//            return status;
//        }
//        psam_iso7816_send_ADPU(selPsam, &SC_ADPU, &SC_Responce);;
//        //==============================================================================
//        // 获取卡应答数据
//        //==============================================================================
//        memcpy(recebuf, apdu_rsp.data_out, apdu_req.le);
//        *recelen = apdu_rsp.len_out;
//        recebuf[apdu_req.le + 0] = apdu_rsp.swa;
//        recebuf[apdu_req.le + 1] = apdu_rsp.swb;

        iso7816_send_ADPU(selPsam, sendbuf, recebuf,sendlen,recelen);

    }
}

int Mystrcmp(unsigned char *str, unsigned char *ptr,int len)
{
    int i = 0;
	  for(i = 0;i < len;i++)
	  {
			if(*(str + i) != '\0' && *(ptr + i) != '\0')
			{
					if(*str > *ptr)
					{
							return 1;
					}
					else if(*str < *ptr)
					{
							return -1;
					}
					i++;
			}	
	  }
    return 0;
}



unsigned char PSAM_TestCommond(unsigned char *commond)
{
	//unsigned char Status=0,i=0,CARDidTEMP[4];
	unsigned short j;
	unsigned char rdtemp[500],okflag=0;	
	PSAMSendLen=0;
	
	memset(PsamSendBuf,0x00,200);
	if(commond[0]==0x00 && commond[1]==0x00)//D′AIDD??￠
	{
		//0900 0000 0027A0000000031010000140D84000A800D84004F8000010000000000000000000000099999F370400
		//0900 0000 0127A0000000032010000140D84000A800D84004F8000010000000000000000000000099999F370400
		if(commond[3]==0x27)
		{
	    SaveConfigData(0x7000+commond[2]*0x40,&commond[3],40);
			ReadConfigData(0x7000+commond[2]*0x40,&rdtemp[0],40);
			if(Mystrcmp(rdtemp,&commond[3],40)==0)
			{
				okflag=0x01;
			}
		}
		else if(commond[3]==0x28)
		{
			SaveConfigData(0x7000+commond[2]*0x40,&commond[3],41);
			ReadConfigData(0x7000+commond[2]*0x40,&rdtemp[0],41);
			if(Mystrcmp(rdtemp,&commond[3],41)==0)
			{
				okflag=0x01;
			}
		}
		else if(commond[3]<=0x40)//14-08-04
		{
			SaveConfigData(0x7000+commond[2]*0x40,&commond[3],commond[3]);
			ReadConfigData(0x7000+commond[2]*0x40,&rdtemp[0],commond[3]);
			if(Mystrcmp(rdtemp,&commond[3],commond[3])==0)
			{
				okflag=0x01;
			}
		}
	}
	if(commond[0]==0x00 && commond[1]==0x02)//?áè?AIDD??￠
	{
	  ReadConfigData(0x7000+commond[2]*0x40,&rdtemp[0],41);
		/*
		printf("\r\nrdtemp->");
		for(j=0;j<41;j++)
		{
			printf("%.2X ",rdtemp[j]);
		}*/
	}
	if(commond[0]==0x00 && commond[1]==0x03)//D′CA1???μ?EEPROMà???
	{
		/*printf("\r\nwrtemp->%.2X   ",commond[3]*256+commond[4]+2); //3ìDò?êìa
		for(j=0;j<commond[3]*256+commond[4]+2;j++)
		{
			printf("%.2X ",commond[j+3]);
		}*/
		
	  SaveConfigData(0x8000+commond[2]*0x200,&commond[3],commond[3]*256+commond[4]+2);
		ReadConfigData(0x8000+commond[2]*0x200,&rdtemp[0],commond[3]*256+commond[4]+2);
		
		//printf("len=%d\r\n",rdtemp[3]*256+rdtemp[4]+2);
		for(j=0;j<rdtemp[3]*256+rdtemp[4]+2;j++)
		{
			printf("%.2X ",rdtemp[j+3]);
		}
		if(Mystrcmp(rdtemp,&commond[3],commond[3]*256+commond[4]+2)==0)
		{
			okflag=0x01;
			printf("CA-SUC\n");
		}
	}
	if(commond[0]==0x00 && commond[1]==0x04)//′óEEPROMà????áè?CA1???
	{
	  ReadConfigData(0x8000+commond[2]*0x200,&rdtemp[0],commond[3]*256+commond[4]+2);
		printf("\r\nRdCAPK->");
		for(j=0;j<commond[3]*256+commond[4]+2;j++)
		{
			printf("%.2X ",rdtemp[j]);
		}
	}
	if(okflag)
	{
		DID_delay=20;
		AID_index++;
		printf("num=%.2X",AID_index);
	}
	else if(commond[0]==0x00 && commond[1]==0xFF)//090000FF
	{
		AID_index=1;
		AID_indexcmp=1;
		printf("num=%.2X",AID_index);
	}
	return AID_index;
}

