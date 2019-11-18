#ifndef LED_H
#define LED_H

#include "mh523.h"
#include "mhscpu.h"
#include "delay.h"

#define LEDn                                    4

#define LED1_PIN                                GPIO_Pin_1
#define LED1_GPIO_PORT                          GPIOB

#define LED2_PIN                                GPIO_Pin_4
#define LED2_GPIO_PORT                          GPIOD

#define LED3_PIN                                GPIO_Pin_5
#define LED3_GPIO_PORT                          GPIOD

#define BGLED_PIN                                GPIO_Pin_4
#define BGLED_GPIO_PORT                          GPIOA

typedef enum 
{
  LED1 = 0,
  LED2 = 1,
  LED3 = 2,
  BGLED =3,
}Led_TypeDef;

void BSP_LED_Init();
void LED_Control(Led_TypeDef LED,unsigned char mode);
void LED_TEST();


#endif

