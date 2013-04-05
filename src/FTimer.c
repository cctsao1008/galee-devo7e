#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  系统定时器处理
//

u16 (*SysTimerCallBackFunc)(void);
void SysTimerInit(void)
{
    /* 72MHz / 8 => 9000000 counts per second */
    systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8);

    /* 9000000/9000 = 1000 overflows per second - every 1ms one interrupt */
    systick_set_reload(9000);
    nvic_set_priority(NVIC_SYSTICK_IRQ, 0x0); //Highest priority

    systick_interrupt_enable();

    /* Start counting. */
    systick_counter_enable();

    /* Setup timer for Transmitter */
    SysTimerCallBackFunc = NULL;
    
    /* Enable TIM4 clock. */
    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_TIM4EN);

    /* Enable TIM2 interrupt. */
    nvic_enable_irq(NVIC_TIM4_IRQ);
    nvic_set_priority(NVIC_TIM4_IRQ, 16); //High priority

    timer_disable_counter(TIM4);
    /* Reset TIM4 peripheral. */
    timer_reset(TIM4);

    /* Timer global mode:
     * - No divider
     * - Alignment edge
     * - Direction up
     */
    timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

    /* timer updates each microsecond */
    timer_set_prescaler(TIM4, 72 - 1);
    timer_set_period(TIM4, 65535);

    /* Disable preload. */
    timer_disable_preload(TIM4);

    /* Continous mode. */
    timer_continuous_mode(TIM4);

    /* Disable outputs. */
    timer_disable_oc_output(TIM4, TIM_OC1);
    timer_disable_oc_output(TIM4, TIM_OC2);
    timer_disable_oc_output(TIM4, TIM_OC3);
    timer_disable_oc_output(TIM4, TIM_OC4);

    /* Enable CCP1 */
    timer_disable_oc_clear(TIM4, TIM_OC1);
    timer_disable_oc_preload(TIM4, TIM_OC1);
    timer_set_oc_slow_mode(TIM4, TIM_OC1);
    timer_set_oc_mode(TIM4, TIM_OC1, TIM_OCM_FROZEN);

    /* Disable CCP1 interrupt. */
    timer_disable_irq(TIM4, TIM_DIER_CC1IE);
    timer_enable_counter(TIM4);
}

void SysTimerStart(u16 us, u16 (*cb)(void))
{
    if(! cb)       return;
    SysTimerCallBackFunc = cb;
    /* Counter enable. */
    u16 t = timer_get_counter(TIM4);
    /* Set the capture compare value for OC1. */
    timer_set_oc_value(TIM4, TIM_OC1, us + t);

    timer_clear_flag(TIM4, TIM_SR_CC1IF);
    timer_enable_irq(TIM4, TIM_DIER_CC1IE);
}

void SysTimerStop(void) 
{
    timer_disable_irq(TIM4, TIM_DIER_CC1IE);
    SysTimerCallBackFunc = NULL;
}

void SysTimerWatchDogStart(void)
{
    iwdg_set_period_ms(3000);
    iwdg_start();
}

void SysTimerWatchDogRst(void)
{
    iwdg_reset();
}

void tim4_isr()
{
    if(SysTimerCallBackFunc)
    {
        u16 us = SysTimerCallBackFunc();
        timer_clear_flag(TIM4, TIM_SR_CC1IF);
        if (us)
        {
            timer_set_oc_value(TIM4, TIM_OC1, us + TIM_CCR1(TIM4));
            return;
        }
    }
    SysTimerStop();
}

void usleep(u32 x)
{
    (void)x;
    asm ("mov r1, #24;"
         "mul r0, r0, r1;"
         "b _delaycmp;"
         "_delayloop:"
         "subs r0, r0, #1;"
         "_delaycmp:;"
         "cmp r0, #0;"
         " bne _delayloop;");
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  系统1ms定时器处理
//
volatile u32 SysTimerClk=0;
void (*SysTimerClkCallBack)(void)=NULL;
void sys_tick_handler(void)
{
	SysTimerClk++;
	if(SysTimerClkCallBack)
	{
		SysTimerCallBackFunc();
	}
}

void SysTimerSetCallback(void (*cb)(void))
{
	SysTimerClkCallBack=cb;
}
