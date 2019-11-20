/**
 ****************************************************************
 * @file rc523.c
 *
 * @brief  rc523 driver.
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
#include "mh523.h"
#include "uart.h"
#include "gpio.h"
#include "timer.h"
#include "iso14443_4.h"

#define WATER_LEVEL	     16
#define FIFO_SIZE	       64  //FIFO大小 = 64字节
#define FSD              256 //Frame Size for proximity coupling Device


#define	READ_REG_CTRL	 0x80  // MH523寄存器读写控制位
#define	TP_FWT_302us	 2048
#define TP_dFWT	       192
#define MAX_RX_REQ_WAIT_MS	     5000 // 命令等待超时时间100ms

static u32  g_polling_cnt = 0;
transceive_buffer mf_com_data;//APDU发送Buff

/**
 ****************************************************************
 * @brief write_reg()
 *
 * 写芯片的寄存器
 *
 * @param:  addr 寄存器地址 
 * @param:  val  要写入的值
 ****************************************************************
 */
void write_reg(uint8_t addr, uint8_t val)
{
    uint8_t c;
    //最低位空闲，有效数据域为bit1~bit6
    addr <<= 1;
    //地址最高位为1代表读，为0代表写；
    c = addr & ~(READ_REG_CTRL);
    SPI_CS_LOW();
    spi_write_byte(c);
    spi_write_byte(val);
    SPI_CS_HIGH();
}

/**
 ****************************************************************
 * @brief read_reg()
 *
 * 读芯片的寄存器
 *
 * @param: addr 寄存器地址
 * @return: 读到的寄存器值
 ****************************************************************
 */
uint8_t read_reg(uint8_t addr)
{
    uint8_t c;
    //最低位空闲，有效数据域为bit1~bit6
    addr <<= 1;
    //地址最高位为1代表读，为0代表写；
    c = addr | READ_REG_CTRL;
    SPI_CS_LOW();
    spi_write_byte(c);
    c = spi_read_byte();
    SPI_CS_HIGH();
    return c;
}
/**
 ****************************************************************
 * @brief set_bit_mask()
 *
 * 将寄存器的某些bit位值1
 *
 * @param: reg 寄存器地址
 * @param: mask 需要置位的bit位
 ****************************************************************
 */
void set_bit_mask(u8 reg, u8 mask)
{
    char  tmp;
    tmp = read_reg(reg);
    write_reg(reg, tmp | mask);  // set bit mask
}

/**
 ****************************************************************
 * @brief clear_bit_mask()
 *
 * 将寄存器的某些bit位清0
 *
 * @param: reg 寄存器地址
 * @param: mask 需要清0的bit位
 ****************************************************************
 */
void clear_bit_mask(u8 reg,u8 mask)
{
    char  tmp;

    tmp = read_reg(reg);
    write_reg(reg, tmp & ~mask);  // clear bit mask
}

u32 htonl(u32 lval)
{
    return (((lval >> 24)&0x000000ff) + ((lval >> 8) & 0x0000ff00) + ((lval << 8) & 0x00ff0000) + ((lval << 24) & 0xff000000));
}

u32 ntohl(u32 lval)
{
    return (((lval >> 24)&0x000000ff) + ((lval >> 8) & 0x0000ff00) + ((lval << 8) & 0x00ff0000) + ((lval << 24) & 0xff000000));
}

/**
 ****************************************************************
 * @brief pcd_init()
 *
 * 初始化芯片 包括硬件复位(RST引脚)和软件复位
 *
 * @param: NONE
 * @return:NONE
 * @retval:
 ****************************************************************
 */
void pcd_init(void)
{
    pcd_poweron();//硬件复位
    mdelay(5);
    pcd_reset() ;//芯片软件复位
    mdelay(5);
    pcd_config('A');
}


/**
 ****************************************************************
 * @brief pcd_config()
 *
 * 配置芯片的A/B模式
 *
 * @param: type = A：ISO14443-A  B:ISO14443-B
 * @return:
 * @retval:
 ****************************************************************
 */
char pcd_config(u8 type)
{
    pcd_antenna_off();//关闭天线
    mdelay(7);

    if ('A' == type)
    {
#if (NFC_DEBUG)
        printf("config type A\n");
#endif
        clear_bit_mask(Status2Reg, BIT3);
        clear_bit_mask(ComIEnReg, BIT7); // 高电平
        write_reg(ModeReg,0x3D);	// 11 // CRC seed:6363
        write_reg(RxSelReg, 0x86);//RxWait
        write_reg(RFCfgReg, 0x58); //
        write_reg(TxASKReg, 0x40);//15  //typeA
        write_reg(TxModeReg, 0x00);//12 //Tx Framing A
        write_reg(RxModeReg, 0x00);//13 //Rx framing A
        write_reg(0x0C, 0x10);	//^_^

        //兼容配置
        {
            u8 backup, adc;
            backup = read_reg(0x37);
            write_reg(0x37, 0x00);
            adc = read_reg(0x37);

            if (adc == 0x11)
            {
                write_reg(0x26, 0x48);
                write_reg(0x37, 0x5E);
                write_reg(0x17, 0x88);
                write_reg(0x38, 0x6B);
                write_reg(0x3A, 0x23);
                write_reg(0x29, 0x12);//0x0F); //调制指数
                write_reg(0x3b, 0x25);
            }
            else if (adc == 0x12)
            {
                // 以下寄存器必须按顺序配置
                write_reg(0x37, 0x5E);
                write_reg(0x26, 0x48);
                write_reg(0x17, 0x88);
                write_reg(0x29, 0x12);//0x0F); //调制指数
                write_reg(0x35, 0xED);
                write_reg(0x3b, 0xA5);
                write_reg(0x37, 0xAE);
                write_reg(0x3b, 0x72);

            }
            write_reg(0x37, backup);
        }

    }
    else if ('B' == type)
    {
#if(NFC_DEBUG)
        printf("config type B\n");
#endif
        write_reg(Status2Reg, 0x00);	//清MFCrypto1On
        clear_bit_mask(ComIEnReg, BIT7);// 高电平触发中断
        write_reg(ModeReg, 0x3F);	// CRC seed:FFFF
        write_reg(RxSelReg, 0x88);	//RxWait
        //Tx
        write_reg(GsNReg, 0xF8);	//调制系数
        write_reg(CWGsPReg, 0x3F);	//
        write_reg(ModGsPReg, 0x12);	//调制指数
        write_reg(AutoTestReg, 0x00);
        write_reg(TxASKReg, 0x00);	// typeB
        write_reg(TypeBReg, 0x13);
        write_reg(TxModeReg, 0x83);	//Tx Framing B
        write_reg(RxModeReg, 0x83);	//Rx framing B
        write_reg(BitFramingReg, 0x00);	//TxLastBits=0

        //兼容配置
        {
            u8 backup, adc;
            backup = read_reg(0x37);
            write_reg(0x37, 0x00);
            adc = read_reg(0x37);

            if (adc == 0x11)
            {
                write_reg(0x37, 0x5E);
                write_reg(0x17, 0x88);
                write_reg(0x3A, 0x23);
                write_reg(0x38, 0x6B);
                write_reg(0x29, 0x12);//0x0F); //调制指数
                write_reg(0x3b, 0x25);
            }
            else if (adc == 0x12)
            {
                write_reg(0x37, 0x5E);
                write_reg(0x26, 0x48);
                write_reg(0x17, 0x88);
                write_reg(0x29, 0x12);
                write_reg(0x35, 0xED);
                write_reg(0x3b, 0xA5);
                write_reg(0x37, 0xAE);
                write_reg(0x3b, 0x72);
            }
            write_reg(0x37, backup);
        }
    }
    else
    {
        return USER_ERROR;
    }

    pcd_antenna_on();//打开天线
    mdelay(2);

    return MI_OK;
}

/**
 ****************************************************************
 * @brief pcd_com_transceive()
 *
 * 通过芯片和ISO14443卡通讯
 *
 * @param: pi->mf_command = 芯片命令字
 * @param: pi->mf_length  = 发送的数据长度
 * @param: pi->mf_data[]  = 发送数据
 * @return: status 值为MI_OK:成功
 * @retval: pi->mf_length  = 接收的数据BIT长度
 * @retval: pi->mf_data[]  = 接收数据
 ****************************************************************
 */
char pcd_com_transceive(struct transceive_buffer *pi)
{
    u8  recebyte;
    char  status;
    u8  irq_en;
    u8  wait_for;
    u8  last_bits;
    u8  j;
    u8  val;
    u8  err;
    tick  start_tick;
    u8 irq_inv;
    u16  len_rest;
    u8  len;

    len = 0;
    len_rest = 0;
    err = 0;
    recebyte = 0;
    irq_en = 0;
    wait_for = 0;

    switch (pi->mf_command)
    {
    case PCD_IDLE:
        irq_en   = 0x00;
        wait_for = 0x00;
        break;
    case PCD_AUTHENT:
        irq_en = IdleIEn | TimerIEn;
        wait_for = IdleIRq;
        break;
    case PCD_RECEIVE:
        irq_en   = RxIEn | IdleIEn;
        wait_for = RxIRq;
        recebyte=1;
        break;
    case PCD_TRANSMIT:
        irq_en   = TxIEn | IdleIEn;
        wait_for = TxIRq;
        break;
    case PCD_TRANSCEIVE:
        irq_en = RxIEn | IdleIEn | TimerIEn | TxIEn;
        wait_for = RxIRq;
        recebyte=1;
        break;
    default:
        pi->mf_command = MI_UNKNOWN_COMMAND;
        break;
    }

    if (pi->mf_command != MI_UNKNOWN_COMMAND
            && (((pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT) && pi->mf_length > 0)
                || (pi->mf_command != PCD_TRANSCEIVE && pi->mf_command != PCD_TRANSMIT))
       )
    {
        write_reg(CommandReg, PCD_IDLE);

        irq_inv = read_reg(ComIEnReg) & BIT7;
        write_reg(ComIEnReg, irq_inv |irq_en | BIT0);//使能Timer 定时器中断
        write_reg(ComIrqReg, 0x7F); //Clear INT
        write_reg(DivIrqReg, 0x7F); //Clear INT
        //Flush Fifo
        set_bit_mask(FIFOLevelReg, BIT7);
        if (pi->mf_command == PCD_TRANSCEIVE || pi->mf_command == PCD_TRANSMIT || pi->mf_command == PCD_AUTHENT)
        {
#if (NFC_DEBUG)
            printf(" PCD_tx:");
            for (j = 0; j < pi->mf_length; j++)
            {
                printf("%02x ", (u16)pi->mf_data[j]);
            }
            printf("\n");
#endif

            len_rest = pi->mf_length;
            if (len_rest >= FIFO_SIZE)
            {
                len = FIFO_SIZE;
            } else
            {
                len = len_rest;
            }

            for (j = 0; j < len; j++)
            {
                write_reg(FIFODataReg, pi->mf_data[j]);
            }
            len_rest -= len;//Rest bytes
            if (len_rest != 0)
            {
                write_reg(ComIrqReg, BIT2); // clear LoAlertIRq
                set_bit_mask(ComIEnReg, BIT2);// enable LoAlertIRq
            }

            write_reg(CommandReg, pi->mf_command);
            if (pi->mf_command == PCD_TRANSCEIVE)
            {
                set_bit_mask(BitFramingReg,0x80);
            }

            while (len_rest != 0)
            {
                while(INT_PIN == 0);//Wait LoAlertIRq
                if (len_rest > (FIFO_SIZE - WATER_LEVEL))
                {
                    len = FIFO_SIZE - WATER_LEVEL;
                }
                else
                {
                    len = len_rest;
                }
                for (j = 0; j < len; j++)
                {
                    write_reg(FIFODataReg, pi->mf_data[pi->mf_length - len_rest + j]);
                }

                write_reg(ComIrqReg, BIT2);//在write fifo之后，再清除中断标记才可以

                //printf("\n8 comirq=%02bx,ien=%02bx,INT= %bd \n", read_reg(ComIrqReg), read_reg(ComIEnReg), (u8)INT_PIN);
                len_rest -= len;//Rest bytes
                if (len_rest == 0)
                {
                    clear_bit_mask(ComIEnReg, BIT2);// disable LoAlertIRq
                    //printf("\n9 comirq=%02bx,ien=%02bx,INT= %bd \n", read_reg(ComIrqReg), read_reg(ComIEnReg), (u8)INT_PIN);
                }
            }

#if (0)
            start_tick = get_tick();
#endif
            //Wait TxIRq
            while (INT_PIN == 0)
            {
#if (0)
                //检测芯片是否没有TxIRq中断
                if (is_timeout(start_tick, 500))
                {
                    printf("CHIP_has_no TxIRq interrupt > %d ms, current reg:\n", (u16)500);
                    mdelay(500);
                    for(j = 0; j <= 0x3F; j++)
                    {
                        printf("%02x=%02x\n", j, read_reg(j));
                    }
                    g_polling_cnt=0;

                    return -1;
                }
#endif
            }
            val = read_reg(ComIrqReg);
            if (val & TxIRq)
            {
                write_reg(ComIrqReg, TxIRq);
            }
        }
        if (PCD_RECEIVE == pi->mf_command)
        {
            set_bit_mask(ControlReg, BIT6);// TStartNow
        }

        len_rest = 0; // bytes received
        write_reg(ComIrqReg, BIT3); // clear HoAlertIRq
        set_bit_mask(ComIEnReg, BIT3); // enable HoAlertIRq

#if (0)
        start_tick = get_tick();
#endif
        //等待命令执行完成
        while(INT_PIN == 0)
        {
#if (0)
            //检测芯片是否没有timeout中断
            if (is_timeout(start_tick, MAX_RX_REQ_WAIT_MS))
            {
                printf("CHIP_has_no_timeout_interrupt > %d ms, current reg:\n", (u16)MAX_RX_REQ_WAIT_MS);
                mdelay(500);
                for(j = 0; j <= 0x3F; j++)
                {
                    printf("%02x=%02x\n", j, read_reg(j));
                }
                g_polling_cnt=0;

                return -1;
            }
#endif
        }

        while(1)
        {
#if (0)
            start_tick = get_tick();
#endif
            while(0 == INT_PIN)
            {
#if (0)
                //检测芯片是否没有RxIrq中断
                if (is_timeout(start_tick, 500))
                {
                    printf("CHIP_has_no RxIrq interrupt > %d ms, current reg:\n", (u16)500);
                    mdelay(500);
                    for(j = 0; j <= 0x3F; j++)
                    {
                        printf("%02x=%02x\n", j, read_reg(j));
                    }
                    g_polling_cnt=0;

                    return -1;
                }
#endif
            }
            val = read_reg(ComIrqReg);
            if ((val & BIT3) && !(val & BIT5))
            {
                if (len_rest + FIFO_SIZE - WATER_LEVEL > 255)
                {
#if (0)
                    printf("AF RX_LEN > 255B\n");
#endif
                    break;
                }
                for (j = 0; j <FIFO_SIZE - WATER_LEVEL; j++)
                {
                    pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
                }
                write_reg(ComIrqReg, BIT3);//在read fifo之后，再清除中断标记才可以
                len_rest += FIFO_SIZE - WATER_LEVEL;
            }
            else
            {
                clear_bit_mask(ComIEnReg, BIT3);//disable HoAlertIRq
                break;
            }
        }


        val = read_reg(ComIrqReg);
#if (0)
        printf(" INT:fflvl=%d,rxlst=%02x ,ien=%02x,cirq=%02x\n", (u16)read_reg(FIFOLevelReg),read_reg(ControlReg)&0x07,read_reg(ComIEnReg), val);//XU
#endif
        write_reg(ComIrqReg, val);// 清中断
        if (val & BIT0)
        {   //发生超时
            status = MI_NOTAGERR;
#if (NFC_DEBUG)
            printf(" Time_out!!\n");
#endif
        }
        else
        {
            err = read_reg(ErrorReg);

            status = MI_COM_ERR;
            if ((val & wait_for) && (val & irq_en))
            {
                if (!(val & ErrIRq))
                {   //指令执行正确
                    status = MI_OK;

                    if (recebyte)
                    {
                        val = 0x7F & read_reg(FIFOLevelReg);
                        last_bits = read_reg(ControlReg) & 0x07;
                        if (len_rest + val > MAX_TRX_BUF_SIZE)
                        {   //长度过长超出缓存
                            status = MI_COM_ERR;
#if (0)
                            printf("RX_LEN > 255B\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //防止spi读错后 val-1成为负值
                            {
                                pi->mf_length = (val-1)*8 + last_bits;
                            }
                            else
                            {
                                pi->mf_length = val*8;
                            }
                            pi->mf_length += len_rest*8;

#if (0)
                            printf(" RX:len=%02x,dat:", (u16)pi->mf_length);
#endif
                            if (val == 0)
                            {
                                val = 1;
                            }
                            for (j = 0; j < val; j++)
                            {
                                pi->mf_data[len_rest + j] = read_reg(FIFODataReg);
                            }

#if (0)
                            for (j = 0; j < pi->mf_length/8 + !!(pi->mf_length%8); j++)
                            {                              
                                printf("%02X ", (u16)pi->mf_data[j]);                              
                            }                          
                            printf("\n");
#endif
                        }
                    }
                }
                else if ((err & CollErr) && (!(read_reg(CollReg) & BIT5)))
                {   //a bit-collision is detected
                    status = MI_COLLERR;
                    if (recebyte)
                    {
                        val = 0x7F & read_reg(FIFOLevelReg);
                        last_bits = read_reg(ControlReg) & 0x07;
                        if (len_rest + val > MAX_TRX_BUF_SIZE)
                        {   //长度过长超出缓存
#if (0)
                            printf("COLL RX_LEN > 255B\n");
#endif
                        }
                        else
                        {
                            if (last_bits && val) //防止spi读错后 val-1成为负值
                            {
                                pi->mf_length = (val-1)*8 + last_bits;
                            }
                            else
                            {
                                pi->mf_length = val*8;
                            }
                            pi->mf_length += len_rest*8;
#if (0)
                            printf(" RX: pi_cmd=%02x,pi_len=%02x,pi_dat:", (u16)pi->mf_command, (u16)pi->mf_length);
#endif
                            if (val == 0)
                            {
                                val = 1;
                            }
                            for (j = 0; j < val; j++)
                            {
                                pi->mf_data[len_rest + j +1] = read_reg(FIFODataReg);
                            }
#if (0)
                            for (j = 0; j < pi->mf_length/8 + !!(pi->mf_length%8); j++)
                            {
                                printf("%02X ", (u16)pi->mf_data[j+1]);
                            }
                            printf("\n");
#endif
                        }
                    }
                    pi->mf_data[0] = (read_reg(CollReg) & CollPos);
                    if (pi->mf_data[0] == 0)
                    {
                        pi->mf_data[0] = 32;
                    }
#if (0)
                    printf("\n COLL_DET pos=%02x\n", (u16)pi->mf_data[0]);
#endif
                    pi->mf_data[0]--;// 与之前版本有点映射区别，为了不改变上层代码，这里直接减一；

                }
                else if ((err & CollErr) && (read_reg(CollReg) & BIT5))
                {
                    //printf("COLL_DET,but CollPosNotValid=1\n");
                }
                //else if (err & (CrcErr | ParityErr | ProtocolErr))
                else if (err & (ProtocolErr))
                {
#if (0)
                    printf("protocol err=%02x\n", err);
#endif
                    status = MI_FRAMINGERR;
                }
                else if ((err & (CrcErr | ParityErr)) && !(err &ProtocolErr) )
                {
                    //EMV  parity err EMV 307.2.3.4
                    val = 0x7F & read_reg(FIFOLevelReg);
                    last_bits = read_reg(ControlReg) & 0x07;
                    if (len_rest + val > MAX_TRX_BUF_SIZE)
                    {   //长度过长超出缓存
                        status = MI_COM_ERR;
#if (0)
                        printf("RX_LEN > 255B\n");
#endif
                    }
                    else
                    {
                        if (last_bits && val)
                        {
                            pi->mf_length = (val-1)*8 + last_bits;
                        }
                        else
                        {
                            pi->mf_length = val*8;
                        }
                        pi->mf_length += len_rest*8;
                    }
#if (0)
                    printf("crc-parity err=%02x\n", err);
                    printf("l=%d\n", pi->mf_length );
#endif



                    status = MI_INTEGRITY_ERR;
                }
                else
                {
#if (NFC_DEBUG)
                    printf("unknown ErrorReg=%02x\n", err);
#endif
                    status = MI_INTEGRITY_ERR;
                }
            }
            else
            {
                status = MI_COM_ERR;
#if (NFC_DEBUG)
                printf(" MI_COM_ERR\n");
#endif
            }
        }

        set_bit_mask(ControlReg, BIT7);// TStopNow =1,必要的；
        write_reg(ComIrqReg, 0x7F);// 清中断0
        write_reg(DivIrqReg, 0x7F);// 清中断1
        clear_bit_mask(ComIEnReg, 0x7F);//清中断使能,最高位是控制位
        clear_bit_mask(DivIEnReg, 0x7F);//清中断使能,最高位是控制位
        write_reg(CommandReg, PCD_IDLE);
    }
    else
    {
        status = USER_ERROR;
#if (NFC_DEBUG)
        printf("USER_ERROR\n");
#endif
    }
#if (0)
    printf(" pcd_com: sta=%d,err=%02x\n", status, err);
#endif
    return status;
}

void pcd_reset(void) //软复位数字芯片
{
#if(NFC_DEBUG)
    printf("pcd_reset\n");
#endif
    write_reg(CommandReg, PCD_RESETPHASE); 
}

void pcd_antenna_on(void)//开启天线
{
    write_reg(TxControlReg, read_reg(TxControlReg) | 0x03); //Tx1RFEn=1 Tx2RFEn=1
}

void pcd_antenna_off(void)//关闭天线
{
    write_reg(TxControlReg, read_reg(TxControlReg) & (~0x03));
}
/////////////////////////////////////////////////////////////////////
//设置PCD定时器
//input:fwi=0~15
/////////////////////////////////////////////////////////////////////
void pcd_set_tmo(u8 fwi)
{
    write_reg(TPrescalerReg, (TP_FWT_302us) & 0xFF);
    write_reg(TModeReg, BIT7 | (((TP_FWT_302us)>>8) & 0xFF));

    write_reg(TReloadRegL, (1 << fwi)  & 0xFF);
    write_reg(TReloadRegH, ((1 << fwi)  & 0xFF00) >> 8);
}



void pcd_delay_sfgi(u8 sfgi)
{
    //SFGT = (SFGT+dSFGT) = [(256 x 16/fc) x 2^SFGI] + [384/fc x 2^SFGI]
    //dSFGT =  384 x 2^FWI / fc
    write_reg(TPrescalerReg, (TP_FWT_302us + TP_dFWT) & 0xFF);
    write_reg(TModeReg, BIT7 | (((TP_FWT_302us + TP_dFWT)>>8) & 0xFF));

    if (sfgi > 14 || sfgi < 1)
    {   //FDTA,PCD,MIN = 6078 * 1 / fc
        sfgi = 1;
    }

    write_reg(TReloadRegL, (1 << sfgi) & 0xFF);
    write_reg(TReloadRegH, ((1 << sfgi) >> 8) & 0xFF);

    write_reg(ComIrqReg, 0x7F);//清除中断
    write_reg(ComIEnReg, BIT0);
    clear_bit_mask(TModeReg,BIT7);// clear TAuto
    set_bit_mask(ControlReg,BIT6);// set TStartNow

    while(!INT_PIN);// wait new INT
    //set_bit_mask(TModeReg,BIT7);// recover TAuto
    pcd_set_tmo(g_pcd_module_info.uiFwi); //recover timeout set

}


void pcd_lpcd_config_start(u8 delta, u32 t_inactivity_ms, u8 skip_times, u8 t_detect_us)
{
    u8 XDATA WUPeriod;
    u8 XDATA SwingsCnt;
#if (NFC_DEBUG)
    printf("pcd_lpcd_config_start\n");
#endif
    WUPeriod = t_inactivity_ms * 32.768 / 256  + 0.5;
    SwingsCnt = t_detect_us * 27.12 / 2 / 16 + 0.5;

    write_reg(0x01,0x0F); //先复位寄存器再进行lpcd

    write_reg(0x14, 0x8B);	// Tx2CW = 1 ，continue载波发射打开
    write_reg(0x37, 0x00);//恢复版本号
    write_reg(0x37, 0x5e);	// 打开私有寄存器保护开关
    write_reg(0x3c, 0x30 | delta);	//设置Delta[3:0]的值, 开启32k
    write_reg(0x3d, WUPeriod);	//设置休眠时间
    write_reg(0x3e, 0x80 | ((skip_times & 0x07) << 4) | (SwingsCnt & 0x0F));	//开启LPCD_en设置,跳过探测次数，探测时间
    write_reg(0x37, 0x00);	// 关闭私有寄存器保护开关
    write_reg(0x03, 0x20);	//打开卡探测中断使能
    write_reg(0x01, 0x10);	//PCD soft powerdown

    //具体应用相关，本示例工程配置为高电平为有中断
    clear_bit_mask(0x02, BIT7);
}

/*
	lpcd功能开始函数
*/
void pcd_lpcd_start(void)
{
#if (NFC_DEBUG)
    printf("pcd_lpcd_start\n");
#endif

    write_reg(0x01,0x0F); //先复位寄存器再进行lpcd

    write_reg(0x37, 0x00);//恢复版本号
    if (read_reg(0x37) == 0x10)
    {
        write_reg(0x01, 0x00);	// idle
    }
    write_reg(0x14, 0x8B);	// Tx2CW = 1 ，continue载波发射打开

    write_reg(0x37, 0x5e);	// 打开私有寄存器保护开关

    //write_reg(0x3c, 0x30);	//设置Delta[3:0]的值, 开启32k //0 不能使用
    //write_reg(0x3c, 0x31);	//设置Delta[3:0]的值, 开启32k
    //write_reg(0x3c, 0x32);	//设置Delta[3:0]的值, 开启32k
    //write_reg(0x3c, 0x33);	//设置Delta[3:0]的值, 开启32k
    //write_reg(0x3c, 0x34);	//设置Delta[3:0]的值, 开启32k
    //write_reg(0x3c, 0x35);	//设置Delta[3:0]的值, 开启32k XU
    write_reg(0x3c, 0x37);	//设置Delta[3:0]的值, 开启32k XU
    //write_reg(0x3c, 0x3A);	//设置Delta[3:0]的值, 开启32k XU
    //write_reg(0x3c, 0x3F);	//设置Delta[3:0]的值, 开启32k XU

    write_reg(0x3d, 0x0d);	//设置休眠时间
    write_reg(0x3e, 0x95);	//设置连续探测次数，开启LPCD_en
    write_reg(0x37, 0x00);	// 关闭私有寄存器保护开关
    write_reg(0x03, 0x20);	//打开卡探测中断使能
    write_reg(0x01, 0x10);	//PCD soft powerdown

    //具体应用相关，配置为高电平为有中断
    clear_bit_mask(0x02, BIT7);
}

void pcd_lpcd_end(void)
{
#if (NFC_DEBUG)
    printf("pcd_lpcd_end\n");
#endif
    write_reg(0x01,0x0F); //先复位寄存器再进行lpcd
}

u8 pcd_lpcd_check(void)
{
    if (INT_PIN && (read_reg(DivIrqReg) & BIT5)) //TagDetIrq
    {
        write_reg(DivIrqReg, BIT5); //清除卡检测到中断
        pcd_lpcd_end();
        return TRUE;
    }
    return FALSE;
}

#if 0
void page45_lock(void)
{
    write_reg(VersionReg, 0);
}

//打开芯片的page4私有寄存器的写保护
void page4_unlock(void)
{
    write_reg(VersionReg, 0x5E);
}
//打开芯片的page5私有寄存器的写保护
void page5_unlock(void);
{
    write_reg(VersionReg, 0xAE);
}
#endif

void pcd_set_rate(u8 rate)
{
    u8 val,rxwait;
    switch(rate)
    {
    case '1':
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        write_reg(ModWidthReg, 0x26);//Miller Pulse Length

        write_reg(RxSelReg, 0x88);

        break;

    case '2':
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(TxModeReg, BIT4);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(RxModeReg, BIT4);
        write_reg(ModWidthReg, 0x12);//Miller Pulse Length
        //rxwait相对于106基本速率需增加相应倍数
        val = read_reg(RxSelReg);
        rxwait = ((val & 0x3F)*2);
        if (rxwait > 0x3F)
        {
            rxwait = 0x3F;
        }
        write_reg(RxSelReg,(rxwait | (val & 0xC0)));

        break;

    case '4':
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(TxModeReg, BIT5);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        set_bit_mask(RxModeReg, BIT5);
        write_reg(ModWidthReg, 0x0A);//Miller Pulse Length
        //rxwait相对于106基本速率需增加相应倍数
        val = read_reg(RxSelReg);
        rxwait = ((val & 0x3F)*4);
        if (rxwait > 0x3F)
        {
            rxwait = 0x3F;
        }
        write_reg(RxSelReg,(rxwait | (val & 0xC0)));

        break;


    default:
        clear_bit_mask(TxModeReg, BIT4 | BIT5 | BIT6);
        clear_bit_mask(RxModeReg, BIT4 | BIT5 | BIT6);
        write_reg(ModWidthReg, 0x26);//Miller Pulse Length

        break;
    }

    {   //不同速率选择不同带宽
        u8 adc;
        write_reg(0x37, 0x00);
        adc = read_reg(0x37);

        if (adc == 0x12)
        {
            write_reg(0x37, 0x5E);
            if (rate == '8' || rate == '4')//848k,424k
            {
                write_reg(0x3B, 0x25);
            }
            else if (rate == '2' || rate == '1')// 212k, 106k
            {
                write_reg(0x3B, 0xE5);
            }
            write_reg(0x37, 0x00);
        }
    }
}

#if 0
/**
 ****************************************************************
 * @brief calculate_crc()
 *
 * 利用MH523计算CRC
 *
 * @param: pin   = 需计算CRC数据首地址
 * @param: len   = 需要计算CRC数据的长度
 * @return: pout = CRC结果首地址
 ****************************************************************/
void calculate_crc(u8 *pin, u8 len, u8 *pout)
{
    u8 i, n;

    clear_bit_mask(DivIrqReg, 0x04);
    write_reg(CommandReg, PCD_IDLE);
    set_bit_mask(FIFOLevelReg, 0x80);

    for (i = 0; i < len; i++)
    {
        write_reg(FIFODataReg, *(pin + i));
    }
    write_reg(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = read_reg(DivIrqReg);
        i--;
    } while((i!=0) && !(n&0x04));

#if (NFC_DEBUG)
    printf("crc:i=%02bx,n=%02bx\n", i, n);
#endif
    pout[0] = read_reg(CRCResultRegL);
    pout[1] = read_reg(CRCResultRegM);
    clear_bit_mask(DivIrqReg, 0x04);
}
#endif

