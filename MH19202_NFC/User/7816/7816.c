#include "mhscpu.h"
#include "iso7816_3.h"
#include "emv_errno.h"
#include "emv_hard.h"
#include <stdio.h>
#include "7816.h"

uint8_t atr_buf[64];

static ST_APDU_REQ apdu_req;
static ST_APDU_RSP apdu_rsp;

extern struct emv_core emv_devs[];

void ISO7816_Bspinit(void)
{
   //Enable clock SCI0, SCI1, SCI2, UART0, and GPIO.
    //SYSCTRL->CG_CTRL = U32_BIT6 | U32_BIT7 | U32_BIT8 | U32_BIT10 | U32_BIT21;
    SYSCTRL->CG_CTRL1 = ~0UL;//Enable clock SCI0, SCI1, SCI2, UART0, and GPIO.

    //SCI0->SCI_IIR = 0x80;
    //Reset SCI0, SCI1, SCI2, UART0, and GPIO.
    SYSCTRL->SOFT_RST1 = ~BIT(20);//GPIO 模块软复位信号

    GPIO_ALT_GROUP[0] = 0;//PA_ALT = 0
    //GPIO_ALT_GROUP[1] = 0;//PB_ALT = 0
    GPIO_ALT_GROUP[2] = 0;//PC_ALT = 0;
    //card detect
    SYSCTRL->PHER_CTRL |= BIT(16);//SCI0卡检测信号有效电平选择
                                  //0: 高有效；1: 低有效
    //card power on
    //SYSCTRL->PHER_CTRL &= ~(BIT(20) | BIT(21) | BIT(22));
    SYSCTRL->PHER_CTRL |= BIT(20);//SCI0 VCCEN信号有效电平选择
                                  //0: 高有效；1: 低有效

    //SYSCTRL->PHER_CTRL |= BIT(18) | BIT(19) | BIT(20) | BIT(21) | BIT(22) | BIT(23);
    //SYSCTRL->PHER_CTRL &= ~(BIT(21) | BIT(22) | BIT(23));
    //SYSCTRL->PHER_CTRL = ( BIT21 | BIT22 | BIT23);
    //SYSCTRL->PHER_CTRL = 0;
    //GPIO->PC_ALT &= ~(PIN_ALT_MASK(10) | PIN_ALT_MASK(11) | PIN_ALT_MASK(12) | PIN_ALT_MASK(13) | PIN_ALT_MASK(14));
    //GPIO->PC_ALT |= PIN_ALT(10, 0x01) | PIN_ALT(11, 0x01) | PIN_ALT(12, 0x01) | PIN_ALT(13, 0x01) | PIN_ALT(14, 0x01);

    //GPIO->PB_ALT &= ~(PIN_ALT_MASK(0) | PIN_ALT_MASK(1) | PIN_ALT_MASK(2) | PIN_ALT_MASK(3) | PIN_ALT_MASK(4));
    //GPIO->PB_ALT |= PIN_ALT(0, 0x03) | PIN_ALT(1, 0x03) | PIN_ALT(2, 0x03) | PIN_ALT(3, 0x03) | PIN_ALT(4, 0x03);
    
    //SYSCTRL->FREQ_SEL = 0x38;
		GPIO_PinRemapConfig(GPIOA, GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_14, GPIO_Remap_0);
		SYSCTRL->SCI_GLF = SYSCTRL->SCI_GLF & ~BIT(29) | BIT(28);//模拟卡供电电压选择 5V
		SCI_ConfigEMV(0x01, 3000000);
}


int32_t tst_SCIWarmReset(uint32_t u32Slot)
{
    //uint8_t au8Out_ATR[64];
	  unsigned char i;
    int32_t s32Slot, state;
    struct emv_core *pdev;
    struct emv_atr  su_atr;

    memset(atr_buf, 0, sizeof(atr_buf));
    memset(&apdu_req, 0, sizeof(apdu_req));
    memset(&apdu_rsp, 0, sizeof(apdu_rsp));


    printf("Test WarmReset!\n");

    s32Slot = u32Slot;
    s32Slot = select_slot( s32Slot );
    pdev = &( emv_devs[ s32Slot ] );
    pdev->terminal_ch = s32Slot;

    //SCI_ConfigEMV(0x07, 3000000UL);
    state = iso7816_device_init();
    if (state)
    {
			  printf("iso7816_device_init state=%d\r\n",state);
        while (1);
    }

    /**
     * detect user card whether is in slot or not?
     */

    pdev->terminal_vcc = 5000;

    pdev->terminal_pps = 0;

    pdev->terminal_fi    = 372;
    pdev->terminal_implict_fi = 372;

    pdev->terminal_di = 1;
    pdev->terminal_implict_di = 1;

    pdev->terminal_spec = 0;

    pdev->terminal_ifs  = 254;
    pdev->terminal_pcb  = 0x00;
    pdev->terminal_ipcb = 0x00;
    pdev->emv_card_pcb  = 0x00;
    pdev->emv_card_ipcb = 0x00;
    pdev->terminal_igt  = 16;
    pdev->terminal_mode = 0; /* asynchronize card */


    while (0 != (state = emv_hard_power_pump( pdev )));
    emv_hard_cold_reset( pdev );
    if( 0 == ( state = emv_atr_analyser( pdev, &su_atr, atr_buf )  ) )
    {
        state = emv_atr_parse( pdev, &su_atr );
        state = 1;
        if( 0 != state )
        {
            memset(atr_buf, 0, sizeof(atr_buf));
            emv_hard_warm_reset( pdev );/*warm reset.*/

            if( 0 == ( state = emv_atr_analyser( pdev, &su_atr, atr_buf ) ) )
            {
                state = emv_atr_parse( pdev, &su_atr );
            }
        }
    }
    else
    {
        memset(atr_buf, 0, sizeof(atr_buf));
        emv_hard_warm_reset( pdev );/*warm reset.*/

        if( 0 == ( state = emv_atr_analyser( pdev, &su_atr, atr_buf ) ) )
        {
            state = emv_atr_parse( pdev, &su_atr );
        }
    }
    if(state == 0)
		{
			
			printf("ATR：");
			for(i=0;i<=atr_buf[0];i++)
			{
				printf("%02X ",atr_buf[i]);
			}
			printf("\r\n");

			memcpy(apdu_req.cmd, "\x00\xa4\x04\x00", 4 );
			apdu_req.le = 256;
			apdu_req.lc = 14;
			memcpy( apdu_req.data_in, "2PAY.SYS.DDF01", 14 );
			state = iso7816_exchange(s32Slot, 1, &apdu_req, &apdu_rsp);
			if (state)
			{
					printf("recv response1 %d\n", state);
					while (1);
			}
			printf("PPSE：");
			for(i=0;i<=apdu_rsp.len_out;i++)
			{
				printf("%02X ",apdu_rsp.data_out[i]);
			}
			printf("\r\n");
			
			
			printf("WarmReset test finished, reset!\n");//热复位结束
			return 0;
	  }
		printf("WarmReset ERROR, no ATR return!\n");//热复位结束
}

int32_t tst_SCIColdReset(uint32_t u32Slot)
{
	  int32_t s32Slot, state;
	  uint32_t slot = 0;
	  unsigned int i;
    state = iso7816_device_init();
    if (state)
    {
        printf("device init error %d\n", state);
        while (1);
    }
		
    while(ICC_ERR_NOCARD == (state = iso7816_init(slot, 0, atr_buf )));

    if (state)
    {
        printf("recv ATR error %d\n", state);
        while (1);
    }

		printf("ATR：");
    for(i=0;i<=atr_buf[0];i++)
		{
		  printf("%02X ",atr_buf[i]);
		}
		printf("\r\n");
		
    //printf("recv ATR!\n");
    memcpy(apdu_req.cmd, "\x00\xa4\x04\x00", 4);
    apdu_req.le = 256;
    apdu_req.lc = 14;
    memcpy( apdu_req.data_in, "2PAY.SYS.DDF01", 14);

    state = iso7816_exchange(slot, 1, &apdu_req, &apdu_rsp);

    if (state)
    {
        printf("recv response1 %d\n", state);
        while (1);
    }

    memcpy( apdu_req.cmd, "\xf0\xd4\x00\x00", 4 );
    apdu_req.lc = 32;
    apdu_req.le = 0;
    memcpy( apdu_req.data_in, "2PAY.SYS.DDF01", 14 );

    memcpy( apdu_req.cmd, "\xf0\xd1\x00\x00", 4 );
    apdu_req.lc = 0;
    apdu_req.le = 0;
    apdu_req.cmd[2] = 0;

    state = iso7816_exchange(slot, 1, &apdu_req, &apdu_rsp);

    if (state)
    {
        printf("recv response2 %d\n", state);
        while (1);
    }
		printf("ATR：");
    for(i=0;i<apdu_rsp.len_out;i++)
		{
		  printf("%X\r\n",apdu_rsp. data_out[i]);
		}
		printf("\r\n");
    printf("ColdReset test finished, reset!\n");
    return 0;
}	



