#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  电源控制
//
void PowerInit(void)
{
    SCB_VTOR = VECTOR_TABLE_LOCATION;
    SCB_SCR  &= ~SCB_SCR_SLEEPONEXIT; //sleep immediate on WFI
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    /* Enable GPIOA so we can manage the power switch */
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);   

    /* Pin 2 controls power-down */
    gpio_set_mode(PWR_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, PWR_CTL_PIN);

    /* Enable GPIOA.2 to keep from shutting down */
    gpio_set(PWR_PORT, PWR_CTL_PIN);

    /* When Pin 3 goes high, the user turned off the Tx */
    gpio_set_mode(PWR_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, PWR_SW_PIN);
    
    //////////////////////////////
    //液晶背光初始化
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
    
    //设置为输入管脚以关闭背光
    gpio_set_mode(GPIOB, GPIO_MODE_INPUT,GPIO_CNF_INPUT_FLOAT, GPIO1);

    //设置PWM
    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_TIM3EN);
    timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_period(TIM3, 0x2CF);
    timer_set_prescaler(TIM3, 0);
    timer_generate_event(TIM3, TIM_EGR_UG);
    timer_set_oc_mode(TIM3, TIM_OC4, TIM_OCM_PWM1);
    timer_enable_oc_preload(TIM3, TIM_OC4);

    timer_set_oc_polarity_high(TIM3, TIM_OC4);
    timer_enable_oc_output(TIM3, TIM_OC4);

    timer_enable_preload(TIM3);      
    
    //////////////////////////////
    //振动器初始化
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
    gpio_set_mode(VIBRATOR_PORT, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, VIBRATOR_PIN);
    gpio_clear(VIBRATOR_PORT, VIBRATOR_PIN);
}

void PowerDown(void)
{
    PowerLight(0);
    rcc_set_sysclk_source(RCC_CFGR_SW_SYSCLKSEL_HSICLK);
    rcc_wait_for_osc_ready(HSI);
    gpio_clear(PWR_PORT, PWR_CTL_PIN);
    while(1);
}

int PowerSw(void)
{
    if(gpio_get(PWR_PORT, PWR_SW_PIN)) 
    {
        return 1;
    }
    return 0;
}

void PowerSleep(void)
{
    asm("wfi");
}

void PowerLight(u8 brightness)
{
    timer_disable_counter(TIM3);
    if (brightness == 0) 		//关闭背光
    {
        gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO1);
    }
    else if(brightness > 99)		//背光全开
    {
        gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
        gpio_set(GPIOB, GPIO1);
    }
    else						//中间亮度
    {
        gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO1);
        u32 duty_cycle = 36 * brightness / 10;  // 720 is too bright,
        timer_set_oc_value(TIM3, TIM_OC4, duty_cycle);
        timer_enable_counter(TIM3);
    }
}

void PowerVibrate(u8 onoff)
{
	if(onoff)	gpio_set(VIBRATOR_PORT, VIBRATOR_PIN);
	else 	    gpio_clear(VIBRATOR_PORT, VIBRATOR_PIN);
}

