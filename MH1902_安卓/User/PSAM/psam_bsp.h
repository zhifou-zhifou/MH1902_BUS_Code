#ifndef __PSAM_BSP_H
#define __PSAM_BSP_H
#include "mhscpu.h"



extern volatile unsigned char selPsam;
extern volatile unsigned char baud_val;

/***************************************************************************************
    º¯ÊýÁÐ±í
***************************************************************************************/
void psam_init(uint8_t psam);
void psam1_data_set(unsigned char in_or_out);
void psam2_data_set(unsigned char in_or_out);
void psam_power_on(void);
void psam_power_off(void);
void psam_data_set(uint8_t psam, uint8_t in_or_out);
void pasm_rst_write(uint8_t psam, uint8_t enable);
void psam_data_write(uint8_t psam, uint8_t enable);
void tim4_init(void);
void tim_pwm_init(void);
uint8_t psam_data_read(uint8_t psam);

unsigned char iso7816_rcv_byte(uint8_t psam, uint8_t *buf, uint32_t timeover);
void iso7816_init_baud(uint8_t psam, uint32_t baud);
void iso7816_send_byte(uint8_t psam, uint8_t byte);
void iso7816_wait_etu(uint32_t etu);
#endif
