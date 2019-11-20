#ifndef __PSAM_H
#define __PSAM_H
#include "mhscpu.h"
#include "psam_bsp.h"
#include "emv_core.h"

extern uint8_t psam_atr_tab[];
extern unsigned char psam_atr_len;

extern const uint32_t f_tab[16];
extern const uint32_t d_tab[8];

#define TICK_1US                    (5376*8928U)
#define etu_time                     93U    //CLK = 4M  4M/372=10752bps  etu = 1/10752=93us
/***************************************************************************************
   iso7816 ATR协议相关格式
***************************************************************************************/
#define T0_PROTOCOL                 0x00
#define DIRECT                      0x3B
#define INDIRECT                    0x3F
#define SETUP_LENGTH                20
#define HIST_LENGTH                 20
#define LCmax                       256
#define ISO7816_RCV_TIMEOUT         1500  // iso7816接收超时时间

/***************************************************************************************
   卡ADPU 相关操作码
***************************************************************************************/
#define SC_CLA_GSM11                0xA0
#define SC_SELECT_FILE              0xA4
#define SC_GET_RESPONCE             0xC0
#define SC_STATUS                   0xF2
#define SC_UPDATE_BINARY            0xD6
#define SC_READ_BINARY              0xB0
#define SC_WRITE_BINARY             0xD0
#define SC_UPDATE_RECORD            0xDC
#define SC_READ_RECORD              0xB2
#define SC_CREATE_FILE              0xE0
#define SC_VERIFY                   0x20
#define SC_CHANGE                   0x24
#define SC_DISABLE                  0x26
#define SC_ENABLE                   0x28
#define SC_UNBLOCK                  0x2C
#define SC_EXTERNAL_AUTH            0x82
#define SC_GET_CHALLENGE            0x84
#define SC_GET_A2R                  0x00

/***************************************************************************************
    卡返回状态
***************************************************************************************/
#define SC_EF_SELECTED              0x9F
#define SC_DF_SELECTED              0x9F
#define SC_OP_TERMINATED            0x9000

#define SC_Voltage_5V               0
#define SC_Voltage_3V               1
/***************************************************************************************
   相关结构体定义
***************************************************************************************/
// ATR
typedef struct
{
    uint8_t TS;
    uint8_t T0;
    uint8_t T[SETUP_LENGTH];
    uint8_t H[HIST_LENGTH];
    uint8_t Tlength;
    uint8_t Hlength;
} ISO7816_ATR;

// ADPU-Header
typedef struct
{
    uint8_t CLA; // 命令类别
    uint8_t INS; // 指令代码
    uint8_t P1;  // 附加参数 1
    uint8_t P2;  // 附加参数 2
} ISO7816_Header;

// ADPU BODY
typedef struct
{
    uint8_t LC;
    uint8_t Data[LCmax];
    uint8_t LE;
} ISO7816_Body;

/// ADPU
typedef struct
{
    ISO7816_Header Header;
    ISO7816_Body Body;
} ISO7816_ADPU_Commands;

// SC response
typedef struct
{
    uint8_t Data[LCmax];
    uint8_t SW1;
    uint8_t SW2;
} ISO7816_ADPU_Responce;

/***************************************************************************************
    函数列表
***************************************************************************************/
uint8_t psam_reset(uint8_t psam);
uint8_t iso7816_decode_atr(uint8_t *atr_tab, ISO7816_ATR *atr);
uint8_t atr_init_psam_iso7816(uint8_t psam);
void psam_iso7816_send_ADPU(uint8_t psam, ISO7816_ADPU_Commands *SC_ADPU, ISO7816_ADPU_Responce *SC_ResponceStatus);

void psam_cmd_deal(void);
void iso7816_send_ADPU(uint8_t psam, uint8_t *datain, uint8_t *dataout,unsigned short datain_len,unsigned short *dataout_len);
void iso7816_analy_atr(uint8_t *atr_tab, struct emv_atr *atr);
#endif
