#include "Page.h"
#include "Func.h"

CMENUITEM MenuDelaySet[]={
	{"舵机减速"	,0,0,0,8,0,0,0},
	{"通道1:"	,0,0	, 0,DELAY_MAX,&Model.Delay[0],0,0},
	{"通道2:"	,0,0	, 0,DELAY_MAX,&Model.Delay[1],0,0},
	{"通道3:"	,0,0	, 0,DELAY_MAX,&Model.Delay[2],0,0},
	{"通道4:"	,0,0	, 0,DELAY_MAX,&Model.Delay[3],0,0},
	{"通道5:"	,0,0	, 0,DELAY_MAX,&Model.Delay[4],0,0},
	{"通道6:"	,0,0	, 0,DELAY_MAX,&Model.Delay[5],0,0},
	{"通道7:"	,0,0	, 0,DELAY_MAX,&Model.Delay[6],0,0},
	{"通道8:"	,0,0	, 0,DELAY_MAX,&Model.Delay[7],0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  舵机减速菜单处理过程
//
u32 PageDelaySet(u8 event)
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
		ms.Total=MenuDelaySet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuDelaySet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}