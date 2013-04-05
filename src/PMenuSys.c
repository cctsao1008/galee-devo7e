#include "Page.h"
#include "Func.h"

s16 PageMenuSysReset=0;
s16 PageMenuSysAutoOff=0;
char PageMenuSysVol[6];
CMENUITEM MenuSys[]={
	{"遥控器设置",0,0,0,15,0,0,0},
	{"摇杆校准"	,PageStkCal,0,0,0,0,0,0},
	{"摇杆模式:"	,0,PageOptionStkType	, 0, 3,&TxSys.StkType,0,0},
	{"摇杆死区:"	,0,0					, 0,50,&TxSys.StkDa,0,0},
	{"振动提示:"	,0,PageOptionOnOff		, 0, 1,&TxSys.Vibrator,0,0},
	{"声音提示:"	,0,PageOptionOnOff		, 0, 1,&TxSys.Music,0,0},
	{"按键音量:"	,0,0					, 0,10,&TxSys.KeyBeep,0,0},
	{"按键音调:"	,0,0					, 0,30,&TxSys.KeyTone,0,0},
	{"背光亮度:"	,0,0					, 0,10,&TxSys.Light,0,0},
	{"对 比 度:"	,0,0					, 0,10,&TxSys.Contrast,0,0},
	{"背光关闭:"	,0,PageOptionLightOff	, 0, 6,&TxSys.LightOff,0,0},
	{"电池类型:"	,0,PageOptionBatType	, 0, 3,&TxSys.BatType,0,0},
	{"报警电压:"	,0,0					,33,120,&TxSys.BatWarn,0,PageMenuSysVol},
	{"空闲警告:"	,0,PageOptionFreeWarn	, 0, 5,&TxSys.FreeWarn,0,0},
	{"自动关机:"	,0,PageOptionAutoOff	, 0, 3,&PageMenuSysAutoOff,0,0},
	{"恢复默认值"	,0,PageOptionOkExe		, 0, 1,&PageMenuSysReset,0,0},
};
///////////////////////////////////////////////////////////////////////////////////////
//
//  主界面绘制和按键处理
//
u32 PageMenuSys(u8 event)
{
	static MENUSTAT ms;
		
	//初始化界面参数
	if(event==PV_INIT)
	{
		LcdClear(0);
		if(!ms.Init)
		{
			ms.iFocus=ms.iStart=0;
			ms.Init=1;
		}
		ms.Total=MenuSys[0].Max;
		ms.DrawMask=PD_ALL;	
		PageMenuSysAutoOff=TxSys.AutoOff;
		return 1;
	}
	
	//将电压转为字符串
	PageMenuSysVol[0]=TxSys.BatWarn>=100?('0'+TxSys.BatWarn/100):(' ');
	PageMenuSysVol[1]=TxSys.BatWarn>=10?('0'+TxSys.BatWarn/10%10):(' ');
	PageMenuSysVol[2]='.';	
	PageMenuSysVol[3]=TxSys.BatWarn%10+'0';	
	PageMenuSysVol[4]='V';
	PageMenuSysVol[5]=0;
		
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuSys,&ms);
		
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{
		//如果有子菜单或子功能，进入
		if(MenuSys[ms.iFocus+1].SubItem)
		{
			BeepMusic(MusicEnter);			
			PageEnter((PAGEPROC)MenuSys[ms.iFocus+1].SubItem,PV_INIT);
		}
		//复位功能
		if(MenuSys[ms.iFocus+1].pValue==&PageMenuSysReset)
		{
			if(PageMenuSysReset==1)
			{
				PageMenuSysReset=0;
				memcpy(&TxSys,&TxSysDef,sizeof(TxSys));
				PageAlert("遥控器已恢复默认值!",1000,PV_REDRAW);
			}
		}
		
	}
	
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_INIT);
	}
	
	//自动关机处理
	if(PageMenuSysAutoOff!=TxSys.AutoOff)
	{
		TxSys.AutoOff=PageMenuSysAutoOff;
		AutoOffReset();
	}	
	
	return 0;
}