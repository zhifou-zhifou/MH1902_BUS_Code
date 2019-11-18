#ifndef BSP7816_H
#define BSP7816_H
#include "mhscpu.h"
#include "iso7816_3.h"

extern uint8_t atr_buf[64];

extern  ST_APDU_REQ apdu_req;
extern  ST_APDU_RSP apdu_rsp;
extern struct emv_core emv_devs[];

void ISO7816_Bspinit(void);
int32_t tst_SCIWarmReset(uint32_t u32Slot);
int32_t tst_SCIColdReset(uint32_t u32Slot);

#endif


