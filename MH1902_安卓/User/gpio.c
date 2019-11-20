/* Include ------------------------------------------------------------------*/
#include "mhscpu.h"
#include "gpio.h"
#include "delay.h"
/* Private typedef ----------------------------------------------------------*/
/* Private define -----------------------------------------------------------*/

/* Private macro ------------------------------------------------------------*/
#define NFC_LED_PIN       GPIO_Pin_0     /* LED PB0 刷卡成功信号 */
#define BEEP_PIN          GPIO_Pin_3

/* Private variables --------------------------------------------------------*/
static GPIO_TypeDef* LED_GPIOx = GPIOB;

#define BEEP_GPIOx GPIOB
#define	NFC_PD_GPIOX	  GPIOB
#define	NFC_PD_GPIO_PIN	  GPIO_Pin_0

/* Ptivate function prototypes ----------------------------------------------*/

/******************************************************************************
* Function Name  : gpio_config
* Description    : config gpio for led and nrstpd
* Input          : NONE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void gpio_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    /// LED (PB0)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    //BEEP PA5  ======>蜂鸣器
	  
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD_PU;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
	  GPIOA->PUE &= ~GPIO_Pin_5;		//失能上拉电阻 蜂鸣器不响

    /// NRSTPD (PB0)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    /// NFC INT (PB13)
    /*GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOB, &GPIO_InitStruct);*/

    /// NFC CSN (PB3)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Remap = GPIO_Remap_1;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/******************************************************************************
* Function Name  : led_success_on
* Description    : len on
* Input          : NONE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void beep(void)
{
	  SYSCTRL->PHER_CTRL &= ~BIT(20);
	  GPIOA->PUE |= GPIO_Pin_5;		//Pue enable
    delayms(50);   
	  GPIOA->PUE &= ~GPIO_Pin_5;		//Pue disable
    delayms(10);
	  SYSCTRL->PHER_CTRL |= BIT(20);
}
/******************************************************************************
* Function Name  : led_success_on
* Description    : len on
* Input          : NONE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void led_success_on(void)
{
    GPIO_ResetBits(LED_GPIOx, NFC_LED_PIN);
}

/******************************************************************************
* Function Name  : led_success_off
* Description    : len off
* Input          : NONE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void led_success_off(void)
{
    GPIO_SetBits(LED_GPIOx, NFC_LED_PIN);
}

/******************************************************************************
* Function Name  : pcd_poweron
* Description    : pcd power on
* Input          : NONE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void pcd_poweron(void)
{
    int i;
    GPIO_SetBits(NFC_PD_GPIOX, NFC_PD_GPIO_PIN);
    for (i = 0; i < 2000; i++);
}

/******************************************************************************
* Function Name  : pcd_powerdown
* Description    : pcd power down
* Input          : NONE
* Output         : NONE
* Return         : NONE
******************************************************************************/
void pcd_powerdown(void)
{
    GPIO_ResetBits(NFC_PD_GPIOX, NFC_PD_GPIO_PIN);
}


/********************** (C) COPYRIGHT 2014 Megahuntmicro ********************/
