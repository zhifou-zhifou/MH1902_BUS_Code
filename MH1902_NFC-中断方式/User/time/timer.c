#include "mhscpu.h"
#include "mhscpu_timer.h"
#include "timer.h"

tick g_current_tick = 0;

/*****************************************************************************
 * Name		: init_timer0
 * Function	: initial the timer 0, it will be token interrupt interval 1ms.
 * ---------------------------------------------------------------------------
 * Input Parameters:None
 * Output Parameters:None
 * Return Value:
 * ---------------------------------------------------------------------------
 * Description:
 * 		Must init the timer 0 before calling get_tick(), mdelay(), is_timeout() etc.
 *****************************************************************************/
void init_timer0(void)
{
    TIM_InitTypeDef d;
    NVIC_InitTypeDef nvic;

    g_current_tick = 0;
    d.TIMx = TIM_1;
    d.TIM_Period = SYSCTRL->PCLK_1MS_VAL;
    TIM_DeInit(TIMM0);
    TIM_Init(TIMM0, &d);
    TIM_ITConfig(TIMM0, TIM_1, ENABLE);
    TIM_Cmd(TIMM0, TIM_1, ENABLE);

    nvic.NVIC_IRQChannel = TIM0_1_IRQn;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&nvic);

}

void TIM0_1_IRQHandler()
{
    g_current_tick++;
    TIM_ClearITPendingBit(TIMM0, TIM_1);
}

/*****************************************************************************
 * Name		: get_tick
 * Function	: Get current tick.
 * ---------------------------------------------------------------------------
 * Input Parameters:None
 * Output Parameters:None
 * Return Value:
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
tick get_tick(void)
{
    return g_current_tick;
}

/*****************************************************************************
 * Name		: mdelay
 * Function	: Millisecond delay.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *			msec		Milliseconds of delay
 * Output Parameters:None
 * Return Value:none
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
void mdelay(tick msec)
{
    tick old_tick;

    old_tick = get_tick();
    while (get_diff_tick(get_tick(), old_tick) < msec)
    {
    }
}

/*****************************************************************************
 * Name		: is_timeout
 * Function	: Determine whether the timeout that millisecond.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *			start_time	start time
 *			interval		time interval
 * Output Parameters:None
 * Return Value:
 *			TRUE		timeout
 *			FALSE		It is not timeout
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
int is_timeout(tick start_time, tick interval)
{
    return (get_diff_tick(get_tick(), start_time) >= interval);
}

/*****************************************************************************
 * Name		: get_diff_tick
 * Function	: Get the time interval (unit is 1ms) for two tick.
 * ---------------------------------------------------------------------------
 * Input Parameters:
 *			cur_tick		lastest tick
 *			prior_tick		prior tick
 * Output Parameters:None
 * Return Value:
 *			Return the ticks of two tick difference.
 * ---------------------------------------------------------------------------
 * Description:
 *
 *****************************************************************************/
tick get_diff_tick(tick cur_tick, tick prior_tick)
{
    if (cur_tick < prior_tick) {
        return (cur_tick + (~prior_tick));
    }
    else {
        return (cur_tick - prior_tick);
    }
}
