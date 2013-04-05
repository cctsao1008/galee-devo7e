#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  键盘扫描初始化
//
//这个键盘扫描阵列图可能是错的
//       B.5         B.6         B.7           B.8
//C.6  Rudder TL   Rudder TR   Elevator TU   Elevator TD        
//C.7  NC          Ent         R+            L-
//C.8  Throttle TU Throttle TD Aileron TR    Aileron TL
//C.9  NC          DN-         UP+           Ext  
//
//Comment:
//TL:  Trim Left   TR: Trim Right
//TU:  Trim Up     TD: Trim down
//
//OP:
// 红PB5 蓝PB8 白PB6 黄PB7 棕PC6

void KeyInit(void)
{
	/* Enable AFIO */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_AFIOEN);
	
	/* Remap GPIO_Remap_SWJ_JTAGDisable */
	AFIO_MAPR |= AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON;
	
	/* Enable GPIOB & GPIOE */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
	
	// PortB 5 6 7 8 开漏输出
	gpio_set_mode(KEY_COL_PORT, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_OPENDRAIN, GPIO5 | GPIO6 | GPIO7 | GPIO8);
	gpio_set(KEY_COL_PORT, GPIO5 | GPIO6| GPIO7 | GPIO8);
	
	// PortC 6 7 8 9 上拉输入
	gpio_set_mode(KEY_ROW_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO6 | GPIO7 | GPIO8 | GPIO9);
	gpio_set(KEY_ROW_PORT, GPIO6 | GPIO7 | GPIO8 | GPIO9);
	
	//PortC 10 11上拉输入 HOLD FMOD
	gpio_set_mode(KEY_SW_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO10 | GPIO11);
	gpio_set(KEY_SW_PORT, GPIO10 | GPIO11);
	
	KeyFlush();
	KeyStat=KeyScanOnce();//为开机检测按键取得按键状态
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  键盘扫描一次
//
volatile u16 KeySwExt;//扩展按键
u16 KeyScanOnce(void)
{
    u16 result=0;
    u16 col;
    
    gpio_set(KEY_COL_PORT, KEY_COL_PORT_MASK);
    for(col = 1<<5; col <= 1<<8; col<<=1)
    {
        gpio_clear(KEY_COL_PORT, col);
        result<<=4;
        result|= 0x0f&(gpio_port_read(KEY_ROW_PORT)>>6);
        gpio_set(KEY_COL_PORT, col);
    }

    //如果没有任何键按下，就可以进行OP开关扫描
    if(result==KEY_MASK)
    {    	
    	//PC6 开漏输出并拉低
		gpio_set_mode(KEY_ROW_PORT,GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_OPENDRAIN, GPIO6);
		gpio_clear(KEY_ROW_PORT, GPIO6);
		
		KeySwExt=(~(gpio_port_read(KEY_COL_PORT)>>5))&0xf;
		
		//还原PC6
		gpio_set_mode(KEY_ROW_PORT, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN, GPIO6);
		gpio_set(KEY_ROW_PORT, GPIO6);
	}
	
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  键盘扫描中断服务程序(sys_tick_handler里1ms调用1次)
//
volatile u16 KeyStat,KeyDown,KeyHold,KeyBuf;
volatile u16 KeyPressCnt;
volatile u16 SwStat,SwBuf;
void KeyScanHandler(void)
{
	static u32 KeyScanDivider;
	u16 k;
	u16 s,sw;
		
	//按键扫描分频，25ms扫描一次
	if(SysTimerClk-KeyScanDivider<25)	return;	
	KeyScanDivider=SysTimerClk;
	
	k=KeyScanOnce();		
	
	//键盘消抖
	if(k==KeyBuf)	
	{
		KeyStat=k;
		k&=KEY_MASK;
		if(k!=KEY_MASK)
		{
			if(KeyPressCnt==0)
			{
				if(TxSys.KeyBeep)	BeepShort(1000+TxSys.KeyTone*30,30+TxSys.KeyBeep*6,20);				
				LightStartTmr=SysTimerClk;//背光关闭计数器重置
				AutoOffReset();
				KeyDown&=KeyStat;
			}
			else
			{
				//连按处理
				if(KeyPressCnt>=KEY_CONT_TRIG)
				{
					u16 kd=KeyDown;					
					//连按的赋值要用&,以免将还未处理的"禁连按键"消去
					KeyHold&=KeyBuf;
					KeyDown&=KeyBuf |KEY_EXT|KEY_ENT;//***不允许连按的键在此处处理
					KeyPressCnt=KEY_CONT_TRIG-KEY_CONT_FREQ;
					
					//按键有变化，发声
					if(KeyDown!=kd)
					{
						if(TxSys.KeyBeep)	BeepShort(1000+TxSys.KeyTone*30,30+TxSys.KeyBeep*6,20);
						LightStartTmr=SysTimerClk;//背光关闭计数器重置
						AutoOffReset();
					}
				}
			}
			KeyPressCnt++;
		}
		else
		{
			KeyHold|=KeyBuf;		//按键一松开,KEYHOLD就要为0
			KeyPressCnt=0;
		}
	}
	else
	{
		KeyBuf=k;
	}
	
	//读取开关	
    s=gpio_port_read(KEY_SW_PORT);
    sw=0;
    if(s&(1L<<10))	sw|=SW_FMOD;
    if(s&(1L<<11))	sw|=SW_HOLD;
    if(KeySwExt&1)	sw|=SW_FLAP0;
    if(KeySwExt&2)	sw|=SW_FLAP1;
    if(KeySwExt&4)	sw|=SW_GEAR0;
    if(KeySwExt&8)	sw|=SW_GEAR1;
	if(sw==SwBuf)	SwStat=sw;
	else			SwBuf=sw;	
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  按键检测
//
u8 KeyTstDown(u16 mask)
{
	if((KeyDown&mask)==0)
	{
		KeyDown |= mask;
		return 1;
	}
	return 0;
}
void KeyClearDown(u16 mask)
{
	KeyDown |= mask;	
}

u8 KeyTstStat(u16 mask)
{
	return !(KeyStat&mask);
}

u8 KeyTstHold(u16 mask)
{
	if((KeyHold&mask)==0)
	{
		KeyHold |= mask;
		return 1;
	}
	return 0;
}

void KeyFlush(void)
{
	KeyDown=KeyStat=KeyHold=KeyBuf=KEY_MASK;
	KeyPressCnt=0;
}

void KeyStopCnt(void)
{
	KeyPressCnt=1;
}