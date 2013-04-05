#include "Page.h"
#include "Func.h"

CMENUITEM MenuMain[]={
	{"GALEE DEVO-7E",0,0,0,6,0,0,0},
	{"模型参数调整"	,PageAeroSet,0,0,0,0,0,0},
	{"通道监视器"	,PageMonitor,0,0,0,0,0,0},
	{"模型高级设置"	,PageMenuMod,0,0,0,0,0,0},
	{"遥控器设置"	,PageMenuSys,0,0,0,0,0,0},
	{"USB连接   "	,PageUsb,0,0,0,0,0,0},
	{"关于...   "	,PageAbout,0,0,0,0,0,0},	
};
///////////////////////////////////////////////////////////////////////////////////////
//
//  主界面绘制和按键处理
//
u32 PageMenuMain(u8 event)
{
	static MENUSTAT ms;
	
	if(event==PV_INIT)
	{
		LcdClear(0);
		if(!ms.Init)
		{
			ms.iFocus=ms.iStart=0;
			ms.Init=1;
		}
		ms.Total=MenuMain[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}	
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuMain,&ms);
		
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{
		//如果有子菜单或子功能，进入
		if(MenuMain[ms.iFocus+1].SubItem==PageAeroSet)//模型设置
		{
			BeepMusic(MusicEnter);
			if(Model.Type==MT_AERO)	PageEnter(PageAeroSet,PV_INIT);
			else					PageEnter(PageHeliSet,PV_INIT);
		}
		else if(MenuMain[ms.iFocus+1].SubItem)
		{
			BeepMusic(MusicEnter);
			PageEnter((PAGEPROC)MenuMain[ms.iFocus+1].SubItem,PV_INIT);
		}
		
	}
	
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_INIT);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}