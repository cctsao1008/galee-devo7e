#include "Func.h"

void BeepInit(void)
{
    rcc_peripheral_enable_clock(&RCC_APB1ENR, BEEP_RCC_APB1ENR_TIMEN);
    gpio_set_mode(BEEP_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, BEEP_PIN);

    timer_set_mode(BEEP_TIM, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    
    /* Period */
    timer_set_period(BEEP_TIM, 65535);

    /* Prescaler */
    timer_set_prescaler(BEEP_TIM, 5);
    timer_generate_event(BEEP_TIM, TIM_EGR_UG);

    /* ---- */
    /* Output compare 1 mode and preload */
    timer_set_oc_mode(BEEP_TIM, BEEP_TIM_OC, TIM_OCM_PWM1);
    timer_enable_oc_preload(BEEP_TIM, BEEP_TIM_OC);

    /* Polarity and state */
    timer_set_oc_polarity_low(BEEP_TIM, BEEP_TIM_OC);
    timer_enable_oc_output(BEEP_TIM, BEEP_TIM_OC);

    /* Capture compare value */
    timer_set_oc_value(BEEP_TIM, BEEP_TIM_OC, 0x8000);
    /* ---- */
    /* ARR reload enable */
    timer_enable_preload(BEEP_TIM);
}

void Beep(u16 frequency, u8 volume)
{
    if (volume == 0) {
        //We need to keep the timer running (for the vibration motor, but also in case there is a pause in the music)
        //But don't want the buzzer running
        timer_disable_oc_output(BEEP_TIM, BEEP_TIM_OC);
    } else {
        timer_enable_oc_output(BEEP_TIM, BEEP_TIM_OC);
    }
    /* volume is between 0 and 100 */
    /* period = 14400000 / frequency */
    /* A Period of 65535 gives a ~ 220Hz tone */
    /* The Devo buzzer reaches max-volume with a pw ~ 100us.  That is max volume */
    /* use quadratic to approximate exponential volume control */
    u32 period = 14400000 / frequency;
    /* Taylor series: x + x^2/2 + x^3/6 + x^4/24 */
    u32 duty_cycle = volume == 100 ? (period >> 1) : (u32)volume * volume * volume * 12 / 10000;
    timer_set_period(BEEP_TIM, period);
    timer_set_oc_value(BEEP_TIM, BEEP_TIM_OC, duty_cycle);
}

//////////////////////////////////////////////
//
// 音乐处理函数
//
u8 BeepMusicEnable,BeepVibrator;
const u8 *pBeepMusic=NULL;
static u8 BeepCnt,BeepDivider,BeepShortCnt=0;
const u16 BeepMusicTable[]={
	//   1    2    3 -  4    5    6    7 -
	-1, 349, 392, 440, 466, 523, 578, 659, -1,-1,
	-1, 698, 784, 880, 932,1046,1175,1318, -1,-1,
	-1,1397,1568,1760,1865,2960,3170,3280,
};

void BeepMusic(const u8 *music)
{	
	BeepShortCnt=0;//取消短鸣叫
	BeepCnt=BeepDivider=0;
	pBeepMusic=music;
	if(BeepMusicEnable) 
	{
		Beep(BeepMusicTable[pBeepMusic[1]],pBeepMusic[2]);
		timer_enable_counter(BEEP_TIM);
	}
	if(BeepVibrator)	PowerVibrate(pBeepMusic[3]);
}

void BeepShort(u16 frequency, u8 volume,u8 tm)
{
	BeepCnt=BeepDivider=0;
	BeepShortCnt=tm;
	Beep(frequency,volume);
	timer_enable_counter(BEEP_TIM);
}

void BeepHandler(void)
{	
	static u32 clk1ms;
	
	//比较系统CLICK
	if(SysTimerClk==clk1ms)	return;
	clk1ms=SysTimerClk;
	
	//短时鸣响计数器
	if(BeepShortCnt)
	{
		BeepShortCnt--;
		if(BeepShortCnt==0)
		{
			Beep(1000,0);//解决很微小的嗡嗡声
			timer_disable_counter(BEEP_TIM);
		}
		return;
	}
	
	//蜂鸣器分频，50ms为一个周期
	if(BeepDivider++<50)	return;	
	BeepDivider=0;	

	//如果没到末尾	
	if(pBeepMusic && pBeepMusic[0])
	{
		if(BeepCnt++>=pBeepMusic[0])
		{
			pBeepMusic+=4;
			BeepCnt=0;
			if(BeepMusicEnable) Beep(BeepMusicTable[pBeepMusic[1]],pBeepMusic[2]);
			if(BeepVibrator)	PowerVibrate(pBeepMusic[3]);
		}
	}
	else
	{	
		//到末尾停止声响
		pBeepMusic=NULL;
		timer_disable_counter(BEEP_TIM);
		PowerVibrate(0);
	}
}

//////////////////////////////////////////////
//
// 音乐列表
//
//格式：4字节一单元，依次是：
//	延续:1个数字代表50ms
//	频率:1 2 3...11 12 13...21 22 23..代表低音中音高音的哆来咪
//	音量:(0~100)
//  震动：0|1
//最后必须以4个延续为0结尾
const u8 MusicStartup[]={4,11,80,0, 4,13,90,0, 4,15,90,0, 2,21,100,1, 2,21,80,1, 2,21,60,0, 2,21,40,1, 2,21,20,1, 2,21,10,0, 0,0,0,0};
const u8 MusicTrimZero[]={1,15,80,0, 1,16,80,0, 2,15,80,0, 0,0,0,0};
const u8 MusicTrimMax[]={1,5,80,0, 1,12,0,0, 2,5,60,0, 0,0,0,0};
const u8 MusicEnter[]={1,21,80,0, 2,23,60,0, 0,0,0,0};
const u8 MusicBatLow[]={1,15,80,1, 1,13,60,1, 1,11,60,0, 1,11,0,0, 1,11,60,1, 1,11,0,0, 1,11,60,1, 0,0,0,0};
const u8 MusicTimeout[]={1,11,80,1, 1,13,70,1, 1,15,60,0, 1,13,50,1, 1,11,40,1,  0,0,0,0};
const u8 MusicStkStop[]={8,21,90,1, 4,13,0,1, 4,21,90,0, 2,13,0,1, 4,21,90,1,  0,0,0,0};
const u8 MusicTimeRun[]={4,21,90,1, 4,13,0,0, 4,21,90,1, 4,13,0,0, 4,21,90,1, 4,13,0,0,  0,0,0,0};