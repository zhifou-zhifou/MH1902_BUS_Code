/**
 ****************************************************************
 * @file iso14443a.c
 *
 * @brief  iso1443a protocol driver
 *
 * @author
 *
 *
 ****************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include "uart.h"
#include "timer.h"
#include "mh523.h"
#include "iso14443a.h"
#include "iso14443_4.h"
#include "delay.h"

// 14443-4
#define PICC_RATS			  0xE0				 //Request for Answer To Select
#define PICC_NAK			  0xB2 // no CID

unsigned short CODE gausMaxFrameSizeTable[] =
{
    16,  24,  32,  40,  48,  64,  96,  128, 256,
    256, 256, 256, 256, 256, 256, 256, 256, 256,
};

/*
 * DEFINES
 ****************************************************************
 */




/*
 * TYPE DEFINITIONS
 ****************************************************************
 */
/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************
 */
/*
 * FUNCTION DECLARATIONS
 ****************************************************************
 */



/**
 ****************************************************************
 * @brief pcd_request()
 *
 * 功    能：寻卡
 *
 * @param: req_code[IN]:寻卡方式
 *                0x52 = 寻感应区内所有符合14443A标准的卡
 *                0x26 = 寻未进入休眠状态的卡
 * @param: ptagtype[OUT]：卡片类型代码
 *                0x4400 = Mifare_UltraLight
 *                0x4400 = Mifare_One(S50_0)
 *                0x0400 = Mifare_One(S50_3)
 *                0x0200 = Mifare_One(S70_0)
 				  0x4200 = Mifare_One(S70_3)
 *                0x0800 = Mifare_Pro
 *                0x0403 = Mifare_ProX
 *                0x4403 = Mifare_DESFire
 *
 * @return: 成功返回MI_OK
 * @retval:
 ****************************************************************
 */
char pcd_request(u8 req_code, u8 *ptagtype)
{
    char status;
    transceive_buffer  *pi;
    pi = &mf_com_data;
    //pcd_antenna_off();//关闭天线
   // delayms(5);
    pcd_antenna_on();//打开天线
    delayms(5);
    write_reg(BitFramingReg,0x07);	// Tx last bytes = 7

    clear_bit_mask(TxModeReg, BIT7); //不使能发送crc
    clear_bit_mask(RxModeReg, BIT7); //不使能接收crc
    clear_bit_mask(Status2Reg, BIT3);//清零MF crypto1认证成功标记
    pcd_set_tmo(4);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length = 1;
    mf_com_data.mf_data[0] = req_code;

    status = pcd_com_transceive(pi);

    if (!status && mf_com_data.mf_length != 0x10)
    {
        status = MI_BITCOUNTERR;
        return status;
    }
    *ptagtype = mf_com_data.mf_data[0];
    *(ptagtype + 1) = mf_com_data.mf_data[1];
#if (NFC_DEBUG)
    printf("CardType:%d-%d\r\n",*ptagtype,*(ptagtype + 1));
#endif

    return status;
}


/**
 ****************************************************************
 * @brief pcd_anticoll()
 *
 * 防冲撞函数
 * @param: select_code    0x93  cascaded level 1
 *                        0x95  cascaded level 2
 *                        0x97  cascaded level 3
 * @param:  psnr =存放序列号(4byte)的内存单元首地址
 * @param:  coll_position = 冲突位
 * @return: status =值为MI_OK:成功
 * @retval: psnr  =得到的序列号放入指定单元
 ****************************************************************
 */
char pcd_cascaded_anticoll(u8 select_code, u8 coll_position, u8 *psnr)
{
    char status;
    u8 i;
    u8 temp;
    u8 bits;
    u8 bytes;
    //u8 coll_position;
    u8 snr_check;
    //u8 XDATA snr[4];
    u8  snr[5];

    transceive_buffer  *pi;

    pi = &mf_com_data;
    snr_check = 0;
    coll_position = 0;
    memset(snr, 0, sizeof(snr));
#if (NFC_DEBUG)
  //  printf("ANT:\n");
#endif
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
    clear_bit_mask(TxModeReg, BIT7); //不使能发送crc
    clear_bit_mask(RxModeReg, BIT7); //不使能接收crc
    pcd_set_tmo(4);
    do
    {
        bits = coll_position % 8;
        if (bits != 0)
        {
            bytes = coll_position / 8 + 1;
            clear_bit_mask(BitFramingReg, TxLastBits | RxAlign);
            set_bit_mask(BitFramingReg, (TxLastBits & (bits)) | (RxAlign & (bits << 4))); // tx lastbits , rx align
        }
        else
        {
            bytes = coll_position /8;
        }
        mf_com_data.mf_command = PCD_TRANSCEIVE;
        mf_com_data.mf_data[0] = select_code;
        mf_com_data.mf_data[1] = 0x20 + ((coll_position / 8) << 4) + (bits & 0x0F);

        for (i = 0; i < bytes; i++)
        {
            mf_com_data.mf_data[i + 2] = snr[i];
        }
        mf_com_data.mf_length = bytes + 2;

        status = pcd_com_transceive(pi);


        temp = snr[coll_position / 8];
        if (status == MI_COLLERR)
        {
            for (i = 0; (5 >= coll_position / 8) && (i < 5 - (coll_position / 8)); i++)
            {
                snr[i + (coll_position / 8)] = mf_com_data.mf_data[i + 1];
            }
            snr[(coll_position / 8)] |= temp;
            if(mf_com_data.mf_data[0] >= bits)
            {
                coll_position += mf_com_data.mf_data[0] - bits;
            }
            else
            {
#if (NFC_DEBUG)
       // printf("Err:coll_p  mf_data[0]=%02bx < bits=%02bx\n", mf_com_data.mf_data[0] ,  bits);
#endif
            }


            //保留冲突位以前的有效位
            snr[(coll_position / 8)] &= (0xff >> (8 - (coll_position % 8)));
            //选择冲突bit位为1或是0的卡
            snr[(coll_position / 8)] |=  1 << (coll_position % 8);//选择bit=1的卡
            //snr[(coll_position / 8)] &=  ~(1 << (coll_position % 8));//选择bit=0的卡
            coll_position++; //冲突bit位增1
        }
        else if (status == MI_OK)
        {
            for (i=0; i < (mf_com_data.mf_length / 8) && (i <= 4); i++) //增加(i <= 4)防止snr[4-i]溢出
            {
                snr[4 - i] = mf_com_data.mf_data[mf_com_data.mf_length / 8 - i - 1];
            }
            snr[(coll_position / 8)] |= temp;
        }

    } while (status == MI_COLLERR);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(psnr + i) = snr[i];
            snr_check ^= snr[i];
        }
        if (snr_check != snr[i])
        {
            status = MI_COM_ERR;
            return status;
        }
    }
#if (NFC_DEBUG)
    printf("UID:%02x%02x%02x%02x\r\n",*(psnr),*(psnr + 1),*(psnr + 2),*(psnr + 3));
#endif
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0

    return status;
}



/**
 ****************************************************************
 * @brief pcd_select()
 *
 * 选定一张卡
 * @param: select_code    0x93  cascaded level 1
 *                        0x95  cascaded level 2
 *                        0x97  cascaded level 3
 * @param: psnr 存放序列号(4byte)的内存单元首地址
 * @return: status 值为MI_OK:成功
 * @retval: psnr  得到的序列号放入指定单元
 * @retval: psak  得到的Select acknolege 回复
 *
 *			  sak:
 *            Corresponding to the specification in ISO 14443, this function
 *            is able to handle extended serial numbers. Therefore more than
 *            one select_code is possible.
 *
 *            Select codes:
 *
 *            +----+----+----+----+----+----+----+----+
 *            | b8 | b7 | b6 | b5 | b4 | b3 | b2 | b1 |
 *            +-|--+-|--+-|--+-|--+----+----+----+-|--+
 *              |    |    |    |  |              | |
 *                                |              |
 *              1    0    0    1  | 001..std     | 1..bit frame anticoll
 *                                | 010..double  |
 *                                | 011..triple  |
 *
 *            SAK:
 *
 *            +----+----+----+----+----+----+----+----+
 *            | b8 | b7 | b6 | b5 | b4 | b3 | b2 | b1 |
 *            +-|--+-|--+-|--+-|--+-|--+-|--+-|--+-|--+
 *              |    |    |    |    |    |    |    |
 *                        |              |
 *                RFU     |      RFU     |      RFU
 *
 *                        1              0 .. UID complete, ATS available
 *                        0              0 .. UID complete, ATS not available
 *                        X              1 .. UID not complete
 *
 ****************************************************************
 */
char pcd_cascaded_select(u8 select_code, u8 *psnr,u8 *psak)
{
    u8 i;
    char status;
    u8 snr_check;
    transceive_buffer  *pi;
    pi = &mf_com_data;
    snr_check = 0;
#if (NFC_DEBUG)
    //printf("SELECT:\n");
#endif
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
    set_bit_mask(TxModeReg, BIT7); //使能发送crc
    set_bit_mask(RxModeReg, BIT7); //使能接收crc

    pcd_set_tmo(4);

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length = 7;
    mf_com_data.mf_data[0] = select_code;
    mf_com_data.mf_data[1] = 0x70;
    for (i = 0; i < 4; i++)
    {
        snr_check ^= *(psnr + i);
        mf_com_data.mf_data[i + 2] = *(psnr + i);
    }
    mf_com_data.mf_data[6] = snr_check;

    status = pcd_com_transceive(pi);

    if (status == MI_OK)
    {
        if (mf_com_data.mf_length != 0x8)
        {
            status = MI_BITCOUNTERR;
            return status;
        }
        else
        {
            *psak = mf_com_data.mf_data[0];
#if (NFC_DEBUG)
            printf("SAK:%02x\r\n",*psak);
#endif
        }
    }

    return status;
}

/**
 ****************************************************************
 * @brief pcd_hlta()
 *
 * 功能：命令卡进入休眠状态
 *
 * @param:
 * @return: status 值为MI_OK:成功
 *
 ****************************************************************
 */
char pcd_hlta(void)
{
    char status = MI_OK;
    transceive_buffer  *pi;
    pi = &mf_com_data;

#if (NFC_DEBUG)
    printf("HALT:\n");
#endif
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
    set_bit_mask(TxModeReg, BIT7); //使能发送crc
    clear_bit_mask(RxModeReg, BIT7); //不使能接收crc
    pcd_set_tmo(2); //according to 14443-3 1ms

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_HLTA;
    mf_com_data.mf_data[1] = 0;

    status = pcd_com_transceive(pi);
    if (status)
    {
        if (status==MI_NOTAGERR || status==MI_ACCESSTIMEOUT)
        {
            status = MI_OK;
        }
    }
    return status;
}


/**
 ****************************************************************
 * @brief pcd_rats_a(u8 CID, u8 *ats)
 *
 * 功能：RATS 
 *
 * @param:CID = 终端发送CID   
 * @param:ats:卡片返回的ATS首地址
 * @param:ATSLength:卡片返回ATS的长度
 * @return: status 值为MI_OK:成功
 *
 ****************************************************************/
char pcd_rats_a(u8 CID, u8 *ats,u8 *ATSLength)
{
    char status = MI_OK;
    transceive_buffer *pi;
    unsigned char  * ATS;
    unsigned char ta,tb,tc;
	  unsigned char i;

    pi = &mf_com_data;
    ta = tb = tc  = 0;

    /*initialiszed the PCB*/
    g_pcd_module_info.ucPcdPcb  = 0x02;
    g_pcd_module_info.ucPiccPcb = 0x03;
    g_pcd_module_info.ucCid = CID;

    pcd_delay_sfgi(g_pcd_module_info.uiSfgi);

    pcd_set_tmo(4); //according to 14443-4 4.8ms

    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
    set_bit_mask(TxModeReg, BIT7); //使能发送crc
    set_bit_mask(RxModeReg, BIT7); //使能接收crc

    mf_com_data.mf_command = PCD_TRANSCEIVE;
    mf_com_data.mf_length  = 2;
    mf_com_data.mf_data[0] = PICC_RATS; //Start byte
    mf_com_data.mf_data[1] = (FSDI << 4) + CID; //Parameter
		
#if (NFC_DEBUG) 
		printf("RATS:");		
		for(i=0; i<mf_com_data.mf_length; i++)
		{
				printf("%02X",mf_com_data.mf_data[i]);//RATS
		}
		printf("\r\n"); 
#endif		
		
    status = pcd_com_transceive(pi);
    if ( (status == MI_OK && (pi->mf_length % 8) != 0)
            || (status == MI_COLLERR)
            || (status == MI_INTEGRITY_ERR && pi->mf_length / 8 < 4)
            || (status == MI_INTEGRITY_ERR && pi->mf_length / 8 >= 4 && (pi->mf_length % 8) != 0)
       )
    {
        
        status = MI_NOTAGERR;
    }

    if (MI_OK == status)
    {
        //ATS
        ATS = pi->mf_data;//ATS的内容 包括长度字节
			  *ATSLength = ATS[0];//卡片返回的第一个字节是ATS长度
			  memcpy(ats,ATS,*ATSLength);
        if (pi->mf_length / 8 < 1 || ATS[0] != pi->mf_length / 8)
        {   //at least 1bytes, and	TL = length

            //printf("4\n");
            return PROTOCOL_ERR;//接收到的ATS数据不完整
        }

        if ( (ATS[0] < (2 + ((ATS[1]&BIT4)>>4) + ((ATS[1]&BIT5)>>5) + ((ATS[1]&BIT6)>>6))))
        {   //ERR:TL length
            //return PROTOCOL_ERR;
        }
        else
        {
            if (!(ATS[1]&BIT7)) // bit7 =0  格式字节正确
            {   //T0.7 = 0
                if (ATS[1]&BIT4)//bit4 = 1 TA被传输
                {
                    ta = 1;
                }
                if (ATS[1]&BIT5)//bit5 = 1 TB被传输
                {
                    tb = 1;
                    g_pcd_module_info.uiFwi = (ATS[2+(u8)ta] & 0xF0) >> 4;
                    g_pcd_module_info.uiSfgi = ATS[2+(u8)ta] & 0x0f;

                }
                if (ATS[1]&BIT6)//bit6 = 1 TC被传输
                {
                    tc = 1;
                    g_pcd_module_info.ucCidEn = 0; //CID不支持
                    g_pcd_module_info.ucNadEn = 0; //NAD不支持
                }
                g_pcd_module_info.uiFsc = gausMaxFrameSizeTable[ATS[1] & 0x0F];//PCD支持的最大帧长

                //FSC
                if (pi->mf_length/8 < (2 + (u8)ta + (u8)tb + (u8)tc))
                {   //长度错误
                    return PROTOCOL_ERR;
                }
 #if (NFC_DEBUG)               
								printf("ATS:");		
								for(i=0; i<mf_com_data.mf_length/8; i++)
								{
										printf("%02X",mf_com_data.mf_data[i]);//ATS
								}
								printf("\r\n");
#endif							
                pcd_set_tmo(g_pcd_module_info.uiFwi);

            }
            else
            {
                status = PROTOCOL_ERR;//格式字节错误 本次ATS接收错误
            }
        }
    }

    return status;
}


signed char Send_RATS(APDUPacket *receivePacket)
{
    signed char	error_flag;
    unsigned char i;
    struct transceive_buffer MfComData;
    struct transceive_buffer *pi;
    pi = &MfComData;
    MfComData.mf_command = PCD_TRANSCEIVE;
    MfComData.mf_length = 2;
    MfComData.mf_data[0]=0xE0;
    MfComData.mf_data[1]=0x50;// 5 =>PCD最大接收长度64字节
    pcd_set_tmo(4); //according to 14443-4 4.8ms
    write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
    set_bit_mask(TxModeReg, BIT7); //使能发送crc
    set_bit_mask(RxModeReg, BIT7); //使能接收crc
#if (NFC_DEBUG)
    for(i=0; i<MfComData.mf_length; i++)
    {
        printf("%02X",MfComData.mf_data[i]);//打印RATS
    }
    printf("\r\n");
#endif		
    error_flag = pcd_com_transceive(pi);//发送
    if(error_flag == MI_OK)
    {
        for(i=0; i<MfComData.mf_length/8; i++)
        {
            printf("%02X",MfComData.mf_data[i]);//ATS
        }
        printf("\r\n");

        /*if(pi->MfData[0] > 1)//解析ATS接口字节信息 //TL
        {
        	FSCI=pi->MfData[1] & 0x0F; //低4位是FSCI  //T0
        	if(pi->MfData[1] & TB_MASK)
        	{
        		FWI=(pi->MfData[3] & 0xF0)>>4;//高4位是FWI
        		SFGI=pi->MfData[3] & 0x0F;//低四位是SFGI
        	}
        	if(pi->MfData[1] & TC_MASK)
        	{
        		if(pi->MfData[4] & 0x02)
        			support_CID=1;//支持CID
        		if(pi->MfData[4] & 0x01)
        			support_NAD=1;//支持NAD
        	}
        }	*/
    }
    return error_flag;
}

/**
 ****************************************************************
 * @brief pcd_pps_rate()
 *
 * 功能：向ISO14443-4卡发送COS命令
 *
 *
 ****************************************************************
 */
char pcd_pps_rate(transceive_buffer *pi, u8  *ATS, u8 CID, u8 rate)
{
    u8 DRI, DSI;
    char status = MI_OK;

#if (NFC_DEBUG)
    printf("PPS:\n");
#endif

    DRI = 0;
    DSI = 0;

    if ((ATS[0] > 1) && (ATS[1] & BIT5))
    {   //TA(1) transmited
        if (rate == 1)
        {

        }
        else if (rate == 2)
        {
            printf("212K\n");
            if((ATS[2]&BIT0) && (ATS[2]&BIT4))
            {   // DS=2,DR=2 supported 212kbps
                DRI = 1;
                DSI = 1;
            }
            else
            {
                printf(",Unsupport\n");
                return USER_ERROR;
            }
        }
        else if (rate == 3)
        {
            printf("424K\n");
            if((ATS[2]&BIT1) && (ATS[2]&BIT5))
            {   // DS=4,DR=4 supported 424kbps
                DRI = 2;
                DSI = 2;
            }
            else
            {
                printf(",Unsupport\n");
                return USER_ERROR;
            }
        }
        else if (rate == 4)
        {
            printf("848K\n");
            if((ATS[2]&BIT2) && (ATS[2]&BIT6))
            {   // DS=4,DR=4 supported 424kbps
                DRI = 3;
                DSI = 3;
            }
            else
            {
                printf(",Unsupport\n");
                return USER_ERROR;
            }
        }
        else
        {
            printf("USER:No Rate select\n");
            return USER_ERROR;
        }
        write_reg(BitFramingReg,0x00);	// // Tx last bits = 0, rx align = 0
        set_bit_mask(TxModeReg, BIT7); //使能发送crc
        set_bit_mask(RxModeReg, BIT7); //使能接收crc

        pi->mf_command = PCD_TRANSCEIVE;
        pi->mf_length  = 3;
        pi->mf_data[0] = (0x0D << 4) + CID; //Start byte
        pi->mf_data[1] = 0x01 | BIT4; //PPS0 ;BIT4:PPS1 transmited
        pi->mf_data[2] = (DSI << 2) | DRI; //PPS1
        status = pcd_com_transceive(pi);
        if (status == MI_OK)
        {
            if (pi->mf_length == 8 && pi->mf_data[0] == ((0x0D << 4) + CID))
            {   //PPS ok
                printf("pcd_pps_rate OK\n");
                if (rate == 1)
                {
                    //pps到106k

                }
                else if (rate == 2)
                {
                    //pps到212k
                    pcd_set_rate('2');// 212kbps
                }
                else if (rate == 3)
                {
                    //pps到424k
                    pcd_set_rate('4');// 424kbps
                }
                else if (rate == 4)
                {
                    //pps到848k
                    pcd_set_rate('8');// 848kbps
                }

            }
        }

    }

    return status;
}

