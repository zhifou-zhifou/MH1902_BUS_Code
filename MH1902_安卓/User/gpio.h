/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : gpio.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 08/31/2016
 * Description          : GPIO wrapper.
 *****************************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__


#ifdef __cplusplus
extern "C" {
#endif



    /* Include ------------------------------------------------------------------*/
#include "mhscpu.h"

    /* Exported types -----------------------------------------------------------*/
    /* Exported constants -------------------------------------------------------*/
    /* Exported macro -----------------------------------------------------------*/
    /* Exported variables -------------------------------------------------------*/
    /* Exported functions -------------------------------------------------------*/
    void gpio_config(void);

    void beep(void);
    void led_success_on(void);
    void led_success_off(void);

    void pcd_poweron(void);
    void pcd_powerdown(void);


#ifdef __cplusplus
}
#endif

#endif	/* __GPIO_H__ */


/********************** (C) COPYRIGHT 2014 Megahuntmicro ********************/
