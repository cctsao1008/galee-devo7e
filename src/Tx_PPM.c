#include "Tx.h"

#define PPM_PLUS	400	//PPM 脉冲400us
#define PPM_MID		1500
#define PPM_FRAME	20000

//PPM中断程序
u8 TxPpmPin,TxPpmIdx;
static u16 TxPpmCallback(void)
{
	static u16 ppmtime;
	
	//输出高电平
	if(TxPpmPin)	
	{
		PPM_H();
		TxPpmPin=0;
		return PPM_PLUS;
	}
	//输出低电平
	else
	{
		u16 t;
		PPM_L();
		TxPpmPin=1;		
		
		if(TxPpmIdx>=TX_CH_NUM)	//输出完最后一个脉冲
		{
			t=PPM_FRAME-PPM_PLUS*TX_CH_NUM-ppmtime;	//计算帧间隔时间
			ppmtime=0;								//清空通道累加器
			TxPpmIdx=0;								//回归CH1
		}			
		else					//PPM脉冲序列
		{
			t=PPM_MID-PPM_PLUS+TxChValue[TxPpmIdx];	//计算本通道时间		
			ppmtime+=t;								//累加本通道时间
			TxPpmIdx++;								//CH步进
		}
		
		return t;
	}
}
	
u32 TxPpmOpen(void)
{
	//将PPM管脚设置为推挽
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);    
    gpio_set_mode(PPM_PORT, GPIO_MODE_OUTPUT_50_MHZ,GPIO_CNF_OUTPUT_PUSHPULL, PPM_OUT);
	PPM_L();
    
    //50ms 后开始输出PPM信号
    SysTimerStart(50000, TxPpmCallback);
    
    TxPpmPin=0;
    TxPpmIdx=TX_CH_NUM+1;

	return 1;
}

void TxPpmClose(void)
{
	//将PPM管脚设置为高阻态
    gpio_set_mode(PPM_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, PPM_OUT);
}

u32 TxPpmBind(void)
{
	return 0;//返回对码界面显示秒数
}

