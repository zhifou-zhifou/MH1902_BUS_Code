//#include "Packet.h"
//#include "iso14443a.h"
//#include "iso14443_4.h"
//#include "mh523.h"
//#include "mifare.h"
//#include "uart.h"
//#include "stdio.h"

//unsigned char UID[5];//卡号（含BCC）
//unsigned char snrSize;//卡片容量
//unsigned char setCID;
//unsigned char setPCB ;

///**********************************************************************
//函数名称：M1_AnalyzeReceivePacket
//函数功能：M1卡接收数据包分析
//输入参数：
//		receivePacket:接收数据包的首地址
//		sendPacket:待设置发送包首地址
//输出参数:
//		无
//***********************************************************************/
//void M1_AnalyzeReceivePacket(Packet *receivePacket, Packet *sendPacket)
//{
//    signed char status;
//    unsigned char i,Section,Block,VerifiMode;
//    unsigned char key[12];
//    unsigned int n;
//    transceive_buffer  *pi;
//    pi = &mf_com_data;

//    switch(receivePacket->instruction)
//    {
//    case COM_FINDCARD:	//M1寻卡
//        sendPacket->length = 0x03;
//        sendPacket->instruction = COM_FINDCARD;
//        n = 3;
//        do
//        {
//            n--;
//            status = pcd_request(PICC_REQALL, MH523.CardTypebuf);//寻卡
//        } while((status == MI_NOTAGERR) && (n > 0));

//        if (status == MI_OK)
//        {
//            sendPacket->p	= 'Y';//寻卡成功
//        }
//        else if(status == MI_NOTAGERR)
//        {
//            sendPacket->p	= 'E';//无卡
//        }
//        else
//        {
//            sendPacket->p	= 'N';//寻卡失败
//        }
//        break;
//    case COM_GETUID:	//M1获取卡号
//        sendPacket->instruction = COM_GETUID;
//        n = 3;
//        do
//        {
//            n--;
//            status = pcd_request(PICC_REQALL, MH523.CardTypebuf);
//        } while((status == MI_NOTAGERR) && (n > 0));

//        if(status == MI_NOTAGERR)
//        {
//            sendPacket->length	= 0x03;
//            sendPacket->p		= 'E';
//            break;
//        }
//        if(status == MI_OK)
//        {
//            //防冲突
//            status=pcd_cascaded_anticoll(PICC_ANTICOLL1, MH523.coll_position, MH523.UIDbuf);
//            sendPacket->length	= 0x07;
//            if(status == MI_OK)
//            {
//                sendPacket->p		= 'Y';
//                memcpy(sendPacket->cardUID,MH523.UIDbuf,4);//拷贝读到的卡号发送上位机
//                pcd_cascaded_select(PICC_ANTICOLL1, MH523.UIDbuf,&MH523.SAK);
//            }
//            else
//            {
//                sendPacket->p		= 'N';
//                memset(sendPacket->cardUID,0,4);//读卡号错误 上传全0
//            }
//            break;
//        }
//        else
//        {
//            sendPacket->p		= 'N';
//            memset(sendPacket->cardUID,0,4);//读UID错误 UID清0
//            break;
//        }
//    case COM_VERIFYKEYA:
//    case COM_VERIFYKEYB:	//密码验证
//        sendPacket->length	= 0x04;
//        sendPacket->sectorNumber = receivePacket->sectorNumber;
//        if(receivePacket->instruction == COM_VERIFYKEYA)
//        {
//            sendPacket->instruction = COM_VERIFYKEYA;
//            VerifiMode = PICC_AUTHENT1A;//验证模式 = KeyA
//        }
//        else if(receivePacket->instruction == COM_VERIFYKEYB)
//        {
//            sendPacket->instruction = COM_VERIFYKEYB;
//            VerifiMode = PICC_AUTHENT1B;//验证模式 = KeyB
//        }
//        memcpy(key,receivePacket->key,6); //拷贝协议包中的密钥 到 key数组中
//        Section = receivePacket->sectorNumber * 4;// 扇区号*4
//        //密码验证  (验证模式 + 绝对块号 + UID + 密码)
//        status =pcd_auth_state(VerifiMode, Section, MH523.UIDbuf,key);
//        if(status == MI_OK)
//        {
//            sendPacket->p	= 'Y';
//            break;
//        }
//        else if(status == MI_AUTHERR)
//        {
//            sendPacket->p	= '3';//密码错误
//            break;
//        }
//        else if(status == MI_ACCESSTIMEOUT)//超时
//        {
//            n = 3;
//            do
//            {
//                n--;
//                status = pcd_request(PICC_REQALL, MH523.CardTypebuf);

//            } while((status == MI_NOTAGERR) && (n > 0));

//            if(status == MI_NOTAGERR)
//            {
//                sendPacket->p	= 'E';
//                break;
//            }
//            else
//            {
//                sendPacket->p	= 'N';//（0x4E） 寻卡不成功
//                break;
//            }
//        }
//        else
//        {
//            sendPacket->p	= 'N';
//            break;
//        }
//    case COM_READBLOCK:
//    case COM_WRITEBLOCK:	//读写块
//        sendPacket->sectorNumber= receivePacket->sectorNumber;
//        sendPacket->blockNumber	= receivePacket->blockNumber;

//        if(receivePacket->sectorNumber <= 31)
//            Block = receivePacket->sectorNumber*4 + receivePacket->blockNumber;//绝对块号
//        else
//            Block = receivePacket->sectorNumber*16 + 128 + receivePacket->blockNumber;

//        if(receivePacket->instruction == COM_READBLOCK)//收到读块指令
//        {
//            sendPacket->instruction	= COM_READBLOCK;
//            status = pcd_read(Block,MH523.Block);//读取一块
//        }
//        else if(receivePacket->instruction == COM_WRITEBLOCK)//收到写块指令
//        {
//            sendPacket->instruction	= COM_WRITEBLOCK;
//            status = pcd_write(Block,receivePacket->blockData);//写入一块
//        }

//        if(status == MI_OK)
//        {
//            sendPacket->length	= 0x15;
//            sendPacket->p		= 'Y';
//            if(receivePacket->instruction == COM_READBLOCK)//读取一块成功
//            {
//                memcpy(sendPacket->blockData,MH523.Block,16);// 拷贝读到的数据 发送到上位机
//            }
//            else if(receivePacket->instruction == COM_WRITEBLOCK)//写入一块成功
//            {
//                memcpy(sendPacket->blockData,MH523.Block,16);
//            }
//            break;
//        }
//        else if(status == MI_ACCESSTIMEOUT)//读写卡超时
//        {
//            sendPacket->length	= 0x05;

//            n = 3;
//            do
//            {
//                n--;
//                status = pcd_request(PICC_REQALL, MH523.CardTypebuf);

//            } while((status == MI_NOTAGERR) && (n > 0));

//            if(status == MI_NOTAGERR)
//            {
//                sendPacket->p	= 'E';//（0x45） 卡机内无卡
//                break;
//            }
//            else
//            {
//                sendPacket->p	= '4';//读写数据错
//                break;
//            }
//        }
//        else
//        {
//            sendPacket->p	= 'N';//寻卡失败
//            break;
//        }
//    case COM_MODIFYKEY:	//修改密码
//        sendPacket->length		= 0x04;
//        sendPacket->instruction	= COM_MODIFYKEY;
//        sendPacket->sectorNumber= receivePacket->sectorNumber;

//        if(receivePacket->sectorNumber <= 31)
//            mf_com_data.mf_data[1] = receivePacket->sectorNumber*4 + 3;
//        else
//            mf_com_data.mf_data[1] = receivePacket->sectorNumber*16 + 15;

//        for(i=0; i<6; i++)
//        {
//            mf_com_data.mf_data[i+2] = receivePacket->key[i];
//        }
//        mf_com_data.mf_data[8]	= 0xFF;
//        mf_com_data.mf_data[9]	= 0x07;
//        mf_com_data.mf_data[10]	= 0x80;
//        mf_com_data.mf_data[11]	= 0x69;// 8~11 为 控制字节 不允许改变

//        for(i=0; i<6; i++)
//        {
//            mf_com_data.mf_data[i+12] = 0xFF;// KEYB 固定为6个0xFF
//        }
//        status 	= pcd_write(mf_com_data.mf_data[1],&mf_com_data.mf_data[2]);
//        if(status == MI_OK)
//        {
//            sendPacket->p		= 'Y';
//            break;
//        }
//        else if(status == MI_ACCESSTIMEOUT)
//        {
//            n = 3;
//            do
//            {
//                n--;
//                status=pcd_request(PICC_REQALL, MH523.CardTypebuf);

//            } while((status == MI_NOTAGERR) && (n > 0));

//            if(status == MI_NOTAGERR)
//            {
//                sendPacket->p	= 'E';
//                break;
//            }
//            else
//            {
//                sendPacket->p	= 'N';
//                break;
//            }
//        }
//        else
//        {
//            sendPacket->p	= 'N';
//            break;
//        }
//    case COM_INCREMENT:
//    case COM_DECREMENT:	//增减值
//        sendPacket->length		= 0x05;
//        sendPacket->sectorNumber= receivePacket->sectorNumber;
//        sendPacket->blockNumber	= receivePacket->blockNumber;

//        if(receivePacket->sectorNumber <= 31)
//        {
//            // set.MfData[1] = receivePacket->sectorNumber*4 + receivePacket->blockNumber;
//        }
//        else
//        {
//            //  set.MfData[1] = receivePacket->sectorNumber*16 + 128 + receivePacket->blockNumber;
//        }
//        for(i=0; i<4; i++)
//        {
//            //   set.MfData[i+2] = receivePacket->blockValue[i];
//        }
//        if(receivePacket->instruction == COM_INCREMENT)
//        {
//            sendPacket->instruction = COM_INCREMENT;
//            // status = Increment(&set);//块加值
//            // status=PcdValue(PICC_INCREMENT,set.MfData[1],&set.MfData[2]);//充值
//        }
//        else if(receivePacket->instruction == COM_DECREMENT)
//        {
//            sendPacket->instruction = COM_DECREMENT;
//            // status = Decrement(&set);//块键值
//            // status=PcdValue(PICC_DECREMENT,set.MfData[1],&set.MfData[2]);//扣款
//        }

//        if(status == MI_OK)
//        {
//            sendPacket->p		= 'Y';
//            break;
//        }
//        else if(status == MI_ACCESSTIMEOUT)
//        {
//            n = 5;
//            do
//            {
//                n--;
//                status=pcd_request(PICC_REQALL, MH523.CardTypebuf);

//            } while((status == MI_NOTAGERR) && (n > 0));

//            if(status == MI_NOTAGERR)
//            {
//                sendPacket->p	= 'E';
//                break;
//            }
//            else
//            {
//                sendPacket->p	= 'N';
//                break;
//            }
//        }
//        else
//        {
//            sendPacket->p	='4';//块格式不对 不是值格式
//            break;
//        }

//    }
//}

//void CPU_Contact_AnalyzeReceivePacket(Packet *receivePacket, Packet *sendPacket)
//{
//    char status;
//    unsigned int i;
//    unsigned int tx_len,rx_len;
//    Block sendBlock, receiveBlock;

//    switch(receivePacket->instruction)
//    {
//    case CPU_APDUSEND_T0:
//    case CPU_APDUSEND_T1:
//        if(receivePacket->instruction == CPU_APDUSEND_T0)
//            sendPacket->instruction = CPU_APDUSEND_T0;
//        else if(receivePacket->instruction == CPU_APDUSEND_T1)
//            sendPacket->instruction = CPU_APDUSEND_T1;
//        tx_len = receivePacket->APDULength;//需要发送APDU长度
//        status = ISO14443_4_HalfDuplexExchange(&g_pcd_module_info, receivePacket->APDU, tx_len, receivePacket->APDU, &rx_len);
//        if(status == MI_OK)
//        {
//            sendPacket->length		= rx_len+5;
//            sendPacket->APDULength	= rx_len;
//            for(i=0; i<sendPacket->APDULength; i++)
//            {
//                sendPacket->APDU[i]	= receivePacket->APDU[i];
//            }
//            sendPacket->p	= 'Y';
//            break;
//        }
//        else
//        {
//            sendPacket->length	= 0x03;

//            if(status == ISO14443_4_ERR_PROTOCOL)
//            {
//                sendPacket->p	= 'E';
//                break;
//            }
//            else
//            {
//                sendPacket->p	= 'N';
//                break;
//            }
//        }
//    case CPU_DESELECT:
//        sendPacket->instruction = CPU_DESELECT;
//        sendPacket->length		= 3;

//        //Deselect(&sendBlock);
//        //status = SendReceiveBlock(&sendBlock, &receiveBlock);

//        if(status == MI_OK)
//            sendPacket->p		= 'Y';
//        else if(status == MI_COM_ERR)
//        {
//            sendPacket->p	= 'E';
//            break;
//        }
//        else
//        {
//            sendPacket->p	= 'N';
//            break;
//        }

//        break;
//    }
//}

//void CPU_Proximity_AnalyzeReceivePacket(Packet *receivePacket, Packet *sendPacket)
//{
//    signed char status;
//    unsigned char i;
//    // Transceive set;
//    transceive_buffer  *pi;
//    pi = &mf_com_data;

//    switch(receivePacket->instruction)
//    {
//    case PROXIMITY_CPU_SENDRATS: //0x30 RATS
//        sendPacket->instruction = PROXIMITY_CPU_SENDRATS;//0x30
//        status = pcd_rats_a(0, MH523.ATS,&MH523.ATSLength);	//发送RATS
//        if(status == MI_OK)
//        {
//            for(i=0; i<MH523.ATSLength; i++)
//            {
//                sendPacket->ATS[i]	= MH523.ATS[i];
//            }
//            sendPacket->length		= MH523.ATSLength+3;
//            sendPacket->ATSLength	= MH523.ATSLength-2;
//            sendPacket->p			= 'Y';
//            break;
//        }

//        sendPacket->length	= 0x03;

//        if(status == MI_NOTAGERR)
//        {
//            sendPacket->p	= 'E';
//            break;
//        }
//        else
//        {
//            sendPacket->p	= 'N';
//            break;
//        }
//    case PROXIMITY_CPU_SENDPPS:
//        //SendPPS();//0x31
//        break;
//    }
//}

///********************************************
//函数名称：SendPacket
//函数功能：向上位机发送数据包
//输入参数：
//		sendPacket:发送数据包首地址
//输出参数:
//		无
//*********************************************/
//void SendPacket(Packet *sendPacket)
//{
//    unsigned char temp, bcc=0;
//    unsigned int i;

//    if(sendPacket->type == TYPE_M1)
//    {
//        switch(sendPacket->instruction)
//        {
//        case COM_FINDCARD:
//            sendPacket->data[0] = sendPacket->p;
//            break;
//        case COM_GETUID:
//            sendPacket->data[0] = sendPacket->p;
//            for(i=0; i<4; i++)
//            {
//                sendPacket->data[i+1] = sendPacket->cardUID[i];
//            }
//            break;
//        case COM_VERIFYKEYA:
//        case COM_VERIFYKEYB:
//        case COM_MODIFYKEY:
//            sendPacket->data[0] = sendPacket->sectorNumber;
//            sendPacket->data[1] = sendPacket->p;
//            break;
//        case COM_READBLOCK:
//        case COM_WRITEBLOCK:
//            sendPacket->data[0] = sendPacket->sectorNumber;
//            sendPacket->data[1] = sendPacket->blockNumber;
//            sendPacket->data[2] = sendPacket->p;
//            if(sendPacket->p == 'Y')
//            {
//                for(i=0; i<16; i++)
//                    sendPacket->data[i+3] = sendPacket->blockData[i];
//            }
//            break;
//        case COM_INCREMENT:
//        case COM_DECREMENT:
//            sendPacket->data[0] = sendPacket->sectorNumber;
//            sendPacket->data[1] = sendPacket->blockNumber;
//            sendPacket->data[2] = sendPacket->p;
//            break;
//        default:
//            break;
//        }
//    }
//    else if(sendPacket->type == TYPE_CPU_CONTACT)
//    {
//        switch(sendPacket->instruction)
//        {
//        case CONTACT_CPU_RESETCOLD:
//        case CONTACT_CPU_RESETHOT:
//        case CPU_APDUSEND_T0:
//        case CPU_APDUSEND_T1:
//            sendPacket->data[0] = sendPacket->p;
//            if(sendPacket->p == 'Y')
//            {
//                sendPacket->data[1] = sendPacket->APDULength/256;
//                sendPacket->data[2] = sendPacket->APDULength%256;
//                for(i=0; i<sendPacket->APDULength; i++)
//                    sendPacket->data[i+3] = sendPacket->APDU[i];
//            }
//            break;
//        case CPU_DESELECT:
//            sendPacket->data[0] = sendPacket->p;
//            break;
//        }
//    }
//    else if(sendPacket->type == TYPE_CPU_PROXI)
//    {
//        switch(sendPacket->instruction)
//        {
//        case PROXIMITY_CPU_SENDRATS:
//            sendPacket->data[0] = sendPacket->p;
//            if(sendPacket->p == 'Y')
//            {
//                for(i=0; i<sendPacket->ATSLength+2; i++)
//                    sendPacket->data[i+1] = sendPacket->ATS[i];
//            }
//            break;
//        }
//    }

//    bcc ^= 0x02;
//    UART0_SendByte(0x02);//02

//    temp = sendPacket->length>>8;

//    bcc ^= temp;
//    UART0_SendByte(temp);//00

//    temp = sendPacket->length;
//    bcc ^= temp;
//    UART0_SendByte(temp);//length

//    bcc ^= sendPacket->type;
//    bcc ^= sendPacket->instruction;

//    UART0_SendByte(sendPacket->type);//type
//    UART0_SendByte(sendPacket->instruction);//command

//    for(i=0; i<(sendPacket->length-2); i++)
//    {
//        bcc ^= sendPacket->data[i];
//        UART0_SendByte(sendPacket->data[i]);
//    }
//    bcc ^= 0x03;
//    UART0_SendByte(0x03);
//    UART0_SendByte(bcc);
//}
