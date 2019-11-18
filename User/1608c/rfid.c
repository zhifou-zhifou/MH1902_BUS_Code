/**
 ****************************************************************
 * @file rfid.c
 *
 * @brief
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
#include "uart.h"
#include "timer.h"
#include "mh523.h"
#include "iso14443a.h"
#include "iso14443b.h"
#include "mifare.h"
#include "iso14443_4.h"
#include "rfid.h"

tag_info XDATA g_tag_info;

statistics_t XDATA g_statistics;
bool XDATA g_statistic_refreshed;

u32 XDATA g_cos_loop_times = 0;
u8 g_cos_loop = FALSE;
u8 XDATA g_loop_buf[60];//不支持太大的cos指令测试

void com_halt_a(void);
void com_mf1_read(u8 *pcmd);
void com_mf1_write(u8 *pcmd);
void com_mf0_read(u8 *pcmd);
void com_mf0_write(u8 *pcmd);
void com_halt_b(u8 *pcmd);
void com_sr176_read(u8 *pcmd);
void com_sr176_write(u8 *pcmd);
void com_select_sr(void);
void com_protect_sr176(u8 *pcmd);
void com_complect_sr(void);
void com_get_idcard_num(void);
void auto_write_read_after_reqa(void);
void com_exchange(u8 *pcmd);
void com_multi_exchange_test(u8 *pcmd);
void com_typea_rats(u8 *pcmd);

void make_packet(u8 cmd, u8 *buf, u8 len)
{
    u8 sum = 0;

    //sum += HEAD;
    //uart_SendChar(HEAD);
    //sum += len + 4;
    //uart_SendChar(len + 4);
//sum += cmd;
//uart_SendChar(cmd);

    while (len--)
    {
        //uart_SendChar(*buf);
        sum += *buf++;
    }
    //uart_SendChar(sum);
}


void rfid_init(void)
{
    memset(&g_tag_info, 0, sizeof(g_tag_info));
    memset(&g_statistics, 0, sizeof(g_statistics));
}
void rfid_operation(u8 *pcmd)
{
    switch(pcmd[0])
    {
    case COM_PKT_CMD_INIT_TYPEA:
        pcd_config('A');
        break;
    case COM_PKT_CMD_INIT_TYPEB:
        pcd_config('B');
        break;
        //Type A
    case COM_PKT_CMD_REQA:
        pcd_set_rate('1');
        com_reqa(pcmd);
        break;
    case COM_PKT_CMD_TYPEA_HALT:
        com_halt_a();
        break;
    case COM_PKT_CMD_TYPEA_MF1_READ:
        com_mf1_read(pcmd);
        break;
    case COM_PKT_CMD_TYPEA_MF1_WRITE:
        com_mf1_write(pcmd);
        break;
    case COM_PKT_CMD_TYPEA_MF1_WALLET_INIT:
        break;
    case COM_PKT_CMD_TYPEA_MF1_WALLET_READ:
        break;
    case COM_PKT_CMD_TYPEA_MF1_WALLET_INCREMENT:
        break;
    case COM_PKT_CMD_TYPEA_MF1_WALLET_DECREMENT:
        break;
    case COM_PKT_CMD_TYPEA_MF0_READ:
        com_mf0_read(pcmd);
        break;
    case COM_PKT_CMD_TYPEA_MF0_WRITE:
        com_mf0_write(pcmd);
        break;
    case COM_PKT_CMD_TYPEA_RATS:
        com_typea_rats(pcmd);
        break;
        //Type B
    case COM_PKT_CMD_REQB:
        pcd_set_rate('1');
        com_reqb(pcmd);
        break;
    case COM_PKT_CMD_TYPEB_HALT:
        com_halt_b(pcmd);
        break;
    case COM_PKT_CMD_SELECT_SR:
        com_select_sr();
        break;
    case COM_PKT_CMD_TYPEB_SR176_READ:
        com_sr176_read(pcmd);
        break;
    case COM_PKT_CMD_TYPEB_SR176_WRITE:
        com_sr176_write(pcmd);
        break;
    case COM_PKT_CMD_PROTECT_SR176:
        com_protect_sr176(pcmd);
        break;
    case COM_PKT_CMD_COMPLECT_SR176:
        com_complect_sr();
        break;
    case COM_PKT_CMD_TYPEB_UID:
        com_get_idcard_num();
        break;
    case COM_PKT_CMD_EXCHANGE:
        com_exchange(pcmd);
        break;
    case COM_PKT_CMD_MULTI_EXCHANGE_TEST:
        com_multi_exchange_test(pcmd);
        break;
    case COM_PKT_CMD_DESELECT:
        iso14443_4_deselect(0);
        break;
    case COM_PKT_CMD_STATISTICS:
        memset(&g_statistics, 0, sizeof(g_statistics));
        break;
    default:
        //#if (NFC_DEBUG)
        //printf("cmd err\n");
        //#endif
        break;
    }

}
char com_reqa(u8 *pcmd)
{
    char status;
    u8 sak;
    u8 XDATA buf[10];

    g_statistics.reqa_cnt++;
    g_statistic_refreshed=TRUE;

    pcd_default_info();


    status = pcd_request(pcmd[1], g_tag_info.tag_type_bytes);
    //一次防冲突及选卡
    if (status == MI_OK)
    {
        g_tag_info.uid_length = UID_4;
        // make_packet(COM_PKT_CMD_CARD_TYPE, g_tag_info.tag_type_bytes, sizeof(g_tag_info.tag_type_bytes));

        status = pcd_cascaded_anticoll(PICC_ANTICOLL1, 0, &g_tag_info.serial_num[0]);
        if (status == MI_OK)
        {
            status = pcd_cascaded_select(PICC_ANTICOLL1, &g_tag_info.serial_num[0], &sak);
        }
    }
    //二次防冲突及选卡
    if(status == MI_OK && (sak & BIT2))
    {
        g_tag_info.uid_length = UID_7;
        status = pcd_cascaded_anticoll(PICC_ANTICOLL2, 0, &g_tag_info.serial_num[4]);
        if(status == MI_OK)
        {
            status = pcd_cascaded_select(PICC_ANTICOLL2, &g_tag_info.serial_num[4], &sak);
        }
    }
    //回复uid
    if (status == MI_OK)
    {
        buf[0] = g_tag_info.uid_length;
        memcpy(buf+1, (g_tag_info.uid_length == UID_4 ? &g_tag_info.serial_num[0]:&g_tag_info.serial_num[1]), g_tag_info.uid_length);
        //  make_packet(COM_PKT_CMD_REQA, buf, g_tag_info.uid_length + 1);
    }

    if(status == MI_OK)
    {
    }
    else
    {
        g_statistics.reqa_fail++;
#if(NFC_DEBUG)
        printf("reqa_fail\n");
#endif
    }

    return status;
}

void com_halt_a(void)
{
    u8 status;

    status = pcd_hlta();
    //  make_packet(COM_PKT_CMD_TYPEA_HALT, &status, sizeof(status));
}

void com_mf1_read(u8 *pcmd)
{
    u8 status;
    u8 block;
    u8 is_a;
    u8 XDATA buf[17];

    block = pcmd[1];
    is_a = pcmd[2];

    status = pcd_auth_state((is_a == TRUE ? PICC_AUTHENT1A : PICC_AUTHENT1B), block, (g_tag_info.uid_length == UID_4 ? &g_tag_info.serial_num[0] : &g_tag_info.serial_num[4]), &pcmd[3]);

    if (status == MI_OK)
    {
        status = pcd_read(block, buf + 1);
    }
    if (status == MI_OK)
    {
        buf[0] = block;
        //    make_packet(COM_PKT_CMD_TYPEA_MF1_READ, buf, sizeof(buf));
    }
}

void com_mf1_write(u8 *pcmd)
{
    u8 status;
    u8 block;
    u8 is_a;

    block = pcmd[1];
    is_a = pcmd[2];

    status = pcd_auth_state((is_a == TRUE ? PICC_AUTHENT1A : PICC_AUTHENT1B), block, (g_tag_info.uid_length == UID_4 ? &g_tag_info.serial_num[0] : &g_tag_info.serial_num[4]), &pcmd[3]);

    if (status == MI_OK)
    {
        status = pcd_write(block, &pcmd[9]);
    }
    make_packet(COM_PKT_CMD_TYPEA_MF1_WRITE, &status, sizeof(status));
}

void com_mf0_read(u8 *pcmd)
{
    u8 status;
    u8 block;
    u8 XDATA buf[17];

    block = pcmd[1];

    status = pcd_read(block, buf + 1);

    if (status == MI_OK)
    {
        buf[0] = block;
        make_packet(COM_PKT_CMD_TYPEA_MF0_READ, buf, sizeof(buf));
    }
}

void com_mf0_write(u8 *pcmd)
{
    u8 status;
    u8 block;

    block = pcmd[1];

    status = pcd_write_ultralight(block, &pcmd[2]);

    make_packet(COM_PKT_CMD_TYPEA_MF0_WRITE, &status, sizeof(status));
}

char com_reqb(u8 *pcmd)
{
    char XDATA status;
    u8 XDATA i;
    u8 XDATA cnt;
    u8 XDATA ATQB[16];
    u8 XDATA req_code;

    g_statistics.reqb_cnt++;
    g_statistic_refreshed=TRUE;

    req_code = pcmd[1];

    cnt = 3;//应用中 可以使用轮询N次
    while(cnt--)
    {
        status = pcd_request_b(req_code, 0, 0, ATQB);

        if(status == MI_COLLERR)	// 有冲突，超过一张卡
        {
            if((status = pcd_request_b(req_code, 0, 2, ATQB)) != MI_OK)
            {
                for (i = 1; i < 4; i++)
                {
                    if((status = pcd_slot_marker(i, ATQB)) == MI_OK)
                    {
                        break;
                    }
                }
                if (status == MI_OK)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else if (status == MI_OK)
        {
            break;
        }
    }

    if (status == MI_OK)
    {
        //typeB 106默认速率
        status = pcd_attri_b(&ATQB[1], 0, ATQB[10]&0x0f, PICC_CID, ATQB);

        if (status == MI_OK)
        {
            ATQB[0] = 0x50;//恢复默认值
            make_packet(COM_PKT_CMD_REQB, ATQB, 12);
        }
    }

    if(status == MI_OK)
    {
    }
    else
    {
        g_statistics.reqb_fail++;
#if(NFC_DEBUG)
        printf("reqb_fail\n");
#endif
    }

    return status;
}

void com_halt_b(u8 *pcmd)
{
    u8 XDATA status;
    pcmd = pcmd;

    status = iso14443_4_deselect(PICC_CID); //deselect all card
    //status = pcd_halt_b(pcmd + 1);// 4 bytes uid

    make_packet(COM_PKT_CMD_TYPEB_HALT, &status, sizeof(status));
}
void com_get_idcard_num(void)
{
    u8 XDATA id[10];
    char XDATA status;

    status = get_idcard_num(id);
    if (status == MI_OK)
    {
        make_packet(COM_PKT_CMD_TYPEB_UID, id, 10);
    }
    else
    {
        g_statistics.r_typeB_id_fail++;
        make_packet(COM_PKT_CMD_STATISTICS, (u8*)&g_statistics, sizeof(g_statistics));
    }
}
void com_select_sr(void)
{
    char XDATA status;
    u8 chip_id;

    status = select_sr(&chip_id);
    if (status == MI_OK)
    {
        make_packet(COM_PKT_CMD_SELECT_SR, &chip_id, sizeof(chip_id));
    }
}
void com_sr176_read(u8 *pcmd)
{
    char XDATA status;
    u8 XDATA buf[3];

    status = read_sr176(pcmd[1], buf + 1);

    if (status == MI_OK)
    {
        buf[0] = pcmd[1];
        make_packet(COM_PKT_CMD_TYPEB_SR176_READ, buf, sizeof(buf));
    }
}
void com_sr176_write(u8 *pcmd)
{
    u8 XDATA status;

    status = write_sr176(pcmd[1], &pcmd[2]);

    if (status == MI_OK)
    {
        make_packet(COM_PKT_CMD_TYPEB_SR176_WRITE, &status, sizeof(status));
    }
}

void com_protect_sr176(u8 *pcmd)
{
    u8 XDATA status;
    u8 XDATA block;

    block = pcmd[1];
    status = protect_sr176(block);
    if (status == MI_OK)
    {
        make_packet(COM_PKT_CMD_PROTECT_SR176, &status, sizeof(status));
    }
}
void com_complect_sr(void)
{
    u8 XDATA status;
    status = completion_sr();

    make_packet(COM_PKT_CMD_COMPLECT_SR176, &status, sizeof(status));
}
void statistic_print(void)
{
    statistics_t statistics_tmp;

    if (g_statistic_refreshed == TRUE)
    {
        mdelay(20);
        memcpy(&statistics_tmp, &g_statistics, sizeof(g_statistics));

        statistics_tmp.reqa_cnt = htonl(statistics_tmp.reqa_cnt);
        statistics_tmp.reqa_fail = htonl(statistics_tmp.reqa_fail);
        statistics_tmp.reqb_cnt = htonl(statistics_tmp.reqb_cnt);
        statistics_tmp.reqb_fail = htonl(statistics_tmp.reqb_fail);
        statistics_tmp.write_fail = htonl(statistics_tmp.write_fail);
        statistics_tmp.read_fail = htonl(statistics_tmp.read_fail);
        statistics_tmp.check_fail = htonl(statistics_tmp.check_fail);
        statistics_tmp.atqa_fail = htonl(statistics_tmp.atqa_fail);
        statistics_tmp.cos_fail = htonl(statistics_tmp.cos_fail);
        statistics_tmp.atqa_cnt = htonl(statistics_tmp.atqa_cnt);
        statistics_tmp.cos_cnt = htonl(statistics_tmp.cos_cnt);
        statistics_tmp.lpcd_cnt = htonl(statistics_tmp.lpcd_cnt);
        statistics_tmp.lpcd_fail = htonl(statistics_tmp.lpcd_fail);
        statistics_tmp.r_typeB_id_fail = htonl(statistics_tmp.r_typeB_id_fail);

        make_packet(COM_PKT_CMD_STATISTICS, (u8*)&statistics_tmp, sizeof(statistics_tmp));
        g_statistic_refreshed = FALSE;
    }
}
void com_typea_rats(u8 * pcmd)
{
    char status;
    u8 CID = 0;
    u8 ats[3];
    u8 rate;
    transceive_buffer *pi;


    pi = &mf_com_data;

    // status = pcd_rats_a(CID, ats,);
    if(status == MI_OK)
    {
        if (pcmd[1] != 0)
        {   //需要做pps指令
            rate = pcmd[1];
            status = pcd_pps_rate(pi, ats, CID, rate);
        }
    }

    if (status != MI_OK)
    {
        g_statistics.atqa_fail++;
    }
    g_statistic_refreshed = TRUE;

}
void com_exchange(u8 * pcmd)
{
    u8  loop_buf[256];
    char status;

    u16  tx_len;
    u32  rx_len;

    g_statistics.cos_cnt++;

    tx_len = 5;//APDU长度
    memcpy(loop_buf, pcmd , tx_len);//从pcmd[2] 开始是APDU命令
    status = ISO14443_4_HalfDuplexExchange(&g_pcd_module_info, loop_buf, tx_len, loop_buf, &rx_len);
    if (status == MI_OK && rx_len < sizeof(loop_buf))
    {
        memmove(loop_buf+1, loop_buf, rx_len);
        loop_buf[0]=rx_len;
        make_packet(COM_PKT_CMD_EXCHANGE, (u8*)&loop_buf, rx_len + 1);
    }
    if (status != MI_OK)
    {
        g_statistics.cos_fail++;
    }
    g_statistic_refreshed = TRUE;
}

unsigned char APDU_Exchange(APDUPacket *receivePacket)
{
    char status=MI_COM_ERR;
    unsigned char i;
    unsigned char   tx_len;//发送APDU的长度
    unsigned int    rx_len;//接收的APDU长度
    tx_len = receivePacket->APDULength;//需要发送APDU长度
    status = ISO14443_4_HalfDuplexExchange(&g_pcd_module_info, receivePacket->APDU, tx_len, receivePacket->APDU, &rx_len);
    if(status==MI_OK)
    {
        receivePacket->APDULength = rx_len;//接收的APDU长度
        for(i=0; i<receivePacket->APDULength; i++)
        {
            printf("%02X",receivePacket->APDU[i]);//打印
        }
        printf("\r\n");
    }
    return status;

}


void com_multi_exchange_test(u8 *pcmd)
{
    g_cos_loop = TRUE;
    if(pcmd[1 + sizeof(u32)] + 1 <= sizeof(g_loop_buf))
    {
        g_cos_loop_times = *(u32*)&pcmd[1];
        g_loop_buf[0]= pcmd[sizeof(u32)];
        memcpy(g_loop_buf + 1, pcmd + 1 + sizeof(u32), pcmd[1 + sizeof(u32)] + 1);
    }
    else
    {
        printf("multi cos data > g_loop_buf\n");
    }
}

