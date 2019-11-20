/**
 ****************************************************************
 * @file mh523.h
 *
 * @brief
 *
 * @author
 *
 *
 ****************************************************************
 */
#ifndef MH523_H
#define MH523_H

#include "define.h"
#include "spi.h"


/*
 * DEFINES Registers Address
 ****************************************************************
 */
// PAGE 0
#define     RFU00                 0x00
#define     CommandReg            0x01
#define     ComIEnReg             0x02
#define     DivIEnReg             0x03
#define     ComIrqReg             0x04
#define     DivIrqReg             0x05
#define     ErrorReg              0x06
#define     Status1Reg            0x07
#define     Status2Reg            0x08
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E
#define     RFU0F                 0x0F
// PAGE 1
#define     RFU10                 0x10
#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxASKReg             0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     RFU1A                 0x1A
#define     RFU1B                 0x1B
#define     MfTxReg	              0x1C
#define     MfRxReg               0x1D
#define     TypeBReg              0x1E
#define     SerialSpeedReg        0x1F
// PAGE 2
#define     RFU20                 0x20
#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24
#define     RFU25                 0x25
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsPReg            0x28
#define     ModGsPReg			0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F
// PAGE 3
#define     RFU30                 0x30
#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39
#define     TestDAC2Reg           0x3A
#define     TestADCReg            0x3B
#define     RFU3C                 0x3C
#define     RFU3D                 0x3D
#define     RFU3E                 0x3E
#define     SpecialReg	  		  0x3F


/*
 * DEFINES Registers bits
 ****************************************************************
 */
#define TxIEn 		BIT6
#define RxIEn 		BIT5
#define IdleIEn		BIT4
#define ErrIEn		BIT1
#define TimerIEn	BIT0
#define TxIRq 		BIT6
#define RxIRq 		BIT5
#define IdleIRq		BIT4
#define ErrIRq		BIT1
#define TimerIRq	BIT0

#define CollErr		BIT3
#define CrcErr		BIT2
#define ParityErr	BIT1
#define ProtocolErr BIT0

#define CollPos		(BIT0|BIT1|BIT2|BIT3|BIT4)

#define RxAlign		(BIT4|BIT5|BIT6)
#define TxLastBits	(BIT0|BIT1|BIT2)
/**
 * PCD命令字
 ****************************************************************
 */
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算
#define PCD_CMD_MASK		      0x0F				       // 命令字掩码


/**
 * Mifare Error Codes
 * Each function returns a status value, which corresponds to
 * the mifare error
 * codes.
 ****************************************************************
 */
#define MI_OK							0
#define MI_CHK_OK						0
#define MI_CRC_ZERO						0

#define MI_CRC_NOTZERO					1

#define MI_NOTAGERR						(-1)
#define MI_CHK_FAILED                   (-1)
#define MI_CRCERR						(-2)
#define MI_CHK_COMPERR					(-2)
#define MI_EMPTY						(-3)
#define MI_AUTHERR						(-4)
#define MI_PARITYERR					(-5)
#define MI_CODEERR						(-6)
#define MI_SERNRERR						(-8)
#define MI_KEYERR						(-9)
#define MI_NOTAUTHERR                   (-10)
#define MI_BITCOUNTERR                  (-11)
#define MI_BYTECOUNTERR					(-12)
#define MI_IDLE							(-13)
#define MI_TRANSERR						(-14)
#define MI_WRITEERR						(-15)
#define MI_INCRERR						(-16)
#define MI_DECRERR						(-17)
#define MI_READERR						(-18)
#define MI_OVFLERR						(-19)
#define MI_POLLING						(-20)
#define MI_FRAMINGERR                   (-21)
#define MI_ACCESSERR                    (-22)
#define MI_UNKNOWN_COMMAND				(-23)
#define MI_COLLERR						(-24)
#define MI_RESETERR						(-25)
#define MI_INITERR						(-25)
#define MI_INTERFACEERR                 (-26)
#define MI_ACCESSTIMEOUT                (-27)
#define MI_NOBITWISEANTICOLL			(-28)
#define MI_QUIT							(-30)
#define MI_INTEGRITY_ERR				(-35) //完整性错误(crc/parity/protocol)
#define MI_RECBUF_OVERFLOW              (-50)
#define MI_SENDBYTENR                   (-51)
#define MI_SENDBUF_OVERFLOW             (-53)
#define MI_BAUDRATE_NOT_SUPPORTED       (-54)
#define MI_SAME_BAUDRATE_REQUIRED       (-55)
#define MI_WRONG_PARAMETER_VALUE        (-60)
#define MI_BREAK						(-99)
#define MI_NY_IMPLEMENTED				(-100)
#define MI_NO_MFRC						(-101)
#define MI_MFRC_NOTAUTH					(-102)
#define MI_WRONG_DES_MODE				(-103)
#define MI_HOST_AUTH_FAILED				(-104)
#define MI_WRONG_LOAD_MODE				(-106)
#define MI_WRONG_DESKEY					(-107)
#define MI_MKLOAD_FAILED				(-108)
#define MI_FIFOERR						(-109)
#define MI_WRONG_ADDR					(-110)
#define MI_DESKEYLOAD_FAILED			(-111)
#define MI_WRONG_SEL_CNT				(-114)
#define MI_WRONG_TEST_MODE				(-117)
#define MI_TEST_FAILED					(-118)
#define MI_TOC_ERROR					(-119)
#define MI_COMM_ABORT					(-120)
#define MI_INVALID_BASE					(-121)
#define MI_MFRC_RESET					(-122)
#define MI_WRONG_VALUE					(-123)
#define MI_VALERR						(-124)
#define MI_COM_ERR                      (-125)
#define PROTOCOL_ERR					(-126)

///用户使用错误
#define USER_ERROR						(-127)
#define MAX_TRX_BUF_SIZE	            255

typedef struct transceive_buffer {
    u8 mf_command;
    u16 mf_length;
    u8 mf_data[MAX_TRX_BUF_SIZE];
} transceive_buffer;


typedef struct
{
    unsigned char CardTypebuf[2];               // 卡片类型
    unsigned char UIDbuf[4];               	    // 卡片序列号
    unsigned char CardCapacity;                 // 卡片容量
    unsigned char coll_position;                // 冲突位置
    unsigned char SAK;                          // 卡片SAK
    unsigned char KeyAbuf[6];                   // 卡片密码A
    unsigned char KeyBbuf[6];                   // 卡片密码B
    unsigned char IncrementVal[4];              // 电子钱包充值值
    unsigned char DecrementVal[4];              // 电子钱包扣款值
    unsigned char Block[16];                    //一个块数据缓存区
    unsigned char Sendbuf[64][16];               // 发送数据
    unsigned char Rcvbuf[64][16];                // 接收数据
    unsigned char ATS[64];
    unsigned char ATSLength;
} MH523TypeDef;

extern MH523TypeDef MH523;//初始化一个卡片缓存区


#define FSDI 5 //Frame Size for proximity coupling Device, in EMV test. 身份证必须FSDI = 8

#if(INT_USE_CHECK_REG)
//中断使用查询寄存器
#define INT_PIN (read_reg(0x07) & 0x10)
#else
#define INT_PIN  GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13)
#endif

extern transceive_buffer  mf_com_data;
extern u8  g_led1_on_time;
extern u8  g_led2_on_time;

void write_reg(uint8_t addr, uint8_t val);
uint8_t read_reg(uint8_t addr);
void set_bit_mask(u8 reg, u8 mask);
void clear_bit_mask(u8 reg,u8 mask);
u32 ntohl(u32 lval);
u32 htonl(u32 lval);

void pcd_init(void);
void pcd_reset(void);
char pcd_config(u8 tag_type);

void write_reg(u8 addr, u8 val);
u8 read_reg(u8 addr);
signed char pcd_com_transceive(struct transceive_buffer *pi);

void set_bit_mask(u8 reg, u8 mask);
void clear_bit_mask(u8 reg,u8 mask);
void pcd_set_tmo(u8 fwi);
void pcd_set_rate(u8 rate);
void pcd_antenna_on(void);
void pcd_antenna_off(void);

#if 0
void page45_lock(void);
void page4_unlock(void);
void page5_unlock(void);
#endif

void pcd_lpcd_start(void);
void pcd_lpcd_end(void);
u8 pcd_lpcd_check(void);
void pcd_delay_sfgi(u8 sfgi);
void pcd_lpcd_config_start(u8 delta, u32 t_inactivity_ms, u8 skip_times, u8 t_detect_us);

#endif
