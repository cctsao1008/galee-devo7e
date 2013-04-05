#include "Macros.h"
#include "Func.h"
#include "Page.h"
#include "Tx.h"

//开机USB连接的图标
const u8 USBLogo[]= {
    0x00, 0x77,//width:  119
    0x00, 0x35,//height: 53
    0x00, 0xb3,//size:   179
    0x47, 0x85, 0x70, 0x89, 0x6d, 0x8b, 0x6c, 0x8b, 0x6b, 0x8d, 0x5b, 0x9c, 0x59, 0x9e, 0x58, 0x9f,
    0x57, 0xa0, 0x56, 0x86, 0x0f, 0x8b, 0x56, 0x86, 0x10, 0x8b, 0x55, 0x86, 0x12, 0x89, 0x55, 0x86,
    0x14, 0x87, 0x55, 0x86, 0x70, 0x86, 0x70, 0x86, 0x4f, 0x88, 0x19, 0x86, 0x4e, 0x8c, 0x16, 0x86,
    0x4e, 0x8f, 0x13, 0x86, 0x4e, 0x91, 0x11, 0x86, 0x4e, 0x92, 0x11, 0x85, 0x41, 0x82, 0x0c, 0x93,
    0x0f, 0x85, 0x42, 0x84, 0x09, 0x94, 0x0e, 0x86, 0x42, 0x86, 0x07, 0x95, 0x0c, 0x86, 0x43, 0x87,
    0x06, 0x95, 0x09, 0x88, 0x44, 0x89, 0x03, 0xf6, 0x01, 0xff, 0xff, 0xe5, 0x03, 0x95, 0x1b, 0x87,
    0x33, 0x88, 0x05, 0x95, 0x1d, 0x86, 0x32, 0x87, 0x06, 0x94, 0x1f, 0x86, 0x31, 0x85, 0x09, 0x93,
    0x20, 0x85, 0x31, 0x83, 0x0b, 0x92, 0x22, 0x85, 0x30, 0x81, 0x0e, 0x90, 0x23, 0x86, 0x3f, 0x8e,
    0x25, 0x86, 0x40, 0x8b, 0x27, 0x86, 0x41, 0x86, 0x2b, 0x86, 0x72, 0x86, 0x72, 0x86, 0x72, 0x86,
    0x12, 0x8d, 0x53, 0x86, 0x11, 0x8d, 0x54, 0x86, 0x10, 0x8d, 0x55, 0x86, 0x0f, 0x8d, 0x56, 0x87,
    0x0d, 0x8d, 0x57, 0xa0, 0x58, 0x9f, 0x5a, 0x9d, 0x5c, 0x9b, 0x6a, 0x8d, 0x6a, 0x8d, 0x6a, 0x8d,
    0x6a, 0x8d, 0x16,
};

//遥控器参数
TXSYS	TxSys;
const TXSYS TxSysDef={
	TXCFGMARK,
	0,	//u8 StkCalied
	1,	//s16 ModelNo;
	0,	//s16 StkType; 
	{	//s16 StkCali[4][3];
		{1700,2048,1700},
		{1700,2048,1700},
		{1700,2048,1700},
		{1700,2048,1700},		
	},
	0,	//s16 StkDa;
	1,	//s16 Vibrator;
	1,	//s16 Music;
	5,	//s16 KeyBeep;
	20,	//s16 KeyTone;
	6,	//s16 Light;
	3,	//s16 Contrast;
	3,	//s16 LightOff;
	0,	//s16 BatTYpe;
	45,	//s16 BatWarn;
	0,	//s16 AutoOff;
	1,	//s16 FreeWarn;
};

///////////////////////////////////////////////////////////////////////////////////////
//
// 自动关机计数器复位
//
void AutoOffReset(void)
{
	AutoOffTime=SysTimerClk+TxSys.AutoOff*10*60*1000;
	ChStopTime=SysTimerClk+TxSys.FreeWarn*60000;//初始化
}

///////////////////////////////////////////////////////////////////////////////////////
//
// 系统事件
//
u32 AutoOffTime;
u32 ChStopTime;
void SysProc(void)
{
	static s16 AllChValue;
	static u8 InShutdown=0;
	
	SysTimerWatchDogRst();	
	
	//处理各系统事件
	AdcHandler();	
	StickHandler();
	BeepHandler();
	KeyScanHandler();
	BatteryHandler();				
		
	//信号混控（建议在界面之前，以避免飞行模式闪烁）
	TxMixer();
		
	//定时器处理
	TimerProc();
	
	//微调按钮处理
	TrimProc();

	//检测摇杆是否移动（自动关机）
	if(AllChValue!=TxValueSum)
	{
		AllChValue=TxValueSum;
		AutoOffReset();
	}
	
	//摇杆不动每分钟提醒一次
	if(TxSys.FreeWarn)
	{
		if(SysTimerClk>=ChStopTime && SysTimerClk>60000)
		{
			ChStopTime=SysTimerClk+TxSys.FreeWarn*60000;//
			BeepMusic(MusicStkStop);
			LightStartTmr=SysTimerClk;//背光关闭计数器重置
		}
	}
	
	//如果已经在关机处理过程中，不再处理，以免SysProc重入
	if(!InShutdown)	
	{
		//电源键处理|自动关机(1分钟后才能自动关机)
		if(PowerSw() || (TxSys.AutoOff && SysTimerClk>=AutoOffTime && SysTimerClk>60000))
		{
			InShutdown=1;
			TxClose();
			
			//保存数据并关机	
			ModelSave(TxSys.ModelNo);
			SaveCfg();	
			PowerDown();
		}	
	}	
}

///////////////////////////////////////////////////////////////////////////////////////
//
// 主函数
//
u32 LightStartTmr;
int main() 
{
	s16 txlight,txcontrast;
	int i;
	
    PowerInit();
	AdcInit();
    SysTimerInit();
    SPIFlash_Init(); 				//This must come before LCD_Init() for 7e (LCD share SPI with FLASH!)
    SPI_FlashBlockWriteEnable(1);	//Enable writing to all banks of SPIFlash
    TxSpiInit();
    LcdInit();
	KeyInit();
	BeepInit();
	PpmInInit();
				
	//如果文件系统错误，进入USB联机模式
	if(!FS_Mount() || KeyTstStat(KEY_ENT))
	{
		//打开U盘，等待关机
		PowerLight(10);
		LcdDrawLogo(USBLogo);	
		USB_Enable(1);	
		KeyFlush();
		while(!KeyTstDown(KEY_EXT))
		{
			if(PowerSw())	PowerDown();
			KeyScanHandler();
			SysTimerWatchDogRst();
		}
		KeyFlush();
		USB_Disable(1);
		FS_Mount();
		SysTimerClk=0;//清空系统计时器
	}
	
	//加载配置
	LoadCfg();	
	
	//绘制开机画面，屏幕渐亮
	BeepMusicEnable=TxSys.Music;
	BeepVibrator=TxSys.Vibrator;
	BeepMusic(MusicStartup);
	LcdDrawBmpFile(0,0,"res/logo.bmp");	
	LcdDrawMiniEng(40,59,__DATE__);
	LcdDrawStop();
	for(i=0;i<8000;i++)
	{
		PowerLight(i/100);
		usleep(100);
		SysProc();
	}	

	//提示摇杆
	if(TxSys.StkCalied!=CALMARK)	PageAlertModel("摇杆未校准!",0);
	
	//加载模型，不开发射
	ModelLoad(TxSys.ModelNo,0);
	
	//安全检测
	char *msg=0;
	do{
		PageAlertModel(msg,100);	//检测摇杆和开关
		msg=0;
		if(FlyMode==FM_IDL)		msg="请关闭特技模式!";
		else if(TxSys.StkCalied==CALMARK && StickValue[2]>THRSAFE)	msg="油门摇杆未复位!";	//直接检测摇杆值
	}while(msg);
	
	//重新加载模型，打开发射
	ModelLoad(TxSys.ModelNo,1);		
	
	//预设初值
	txlight=txcontrast=-1;
	
	//主循环
	KeyFlush();
	PageSet(PageMain,PV_INIT);
	
	while(1)
	{
		SysProc();		
		
		PageStack[PageStackIdx](PV_RUN);		//界面处理过程
	
		//调节屏幕亮度和对比度
		if(TxSys.LightOff && SysTimerClk> (LightStartTmr+(u32)TxSys.LightOff*10000))
		{
			PowerLight(txlight=0);
		}
		else
		{
			if(txlight!=TxSys.Light)
			{
				txlight=TxSys.Light;
				PowerLight(TxSys.Light*8);
			}
		}
		if(txcontrast!=TxSys.Contrast)
		{
			txcontrast=TxSys.Contrast;
			LcdContrast(TxSys.Contrast);
		}
		BeepMusicEnable=TxSys.Music;
		BeepVibrator=TxSys.Vibrator;
		PpmInCtl(Model.PpmIn);
	}
}