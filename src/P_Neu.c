#include "Page.h"
#include "Func.h"

CMENUITEM MenuNeuSet[]={
	{"中点调节"	,0,0,0,8,0,0,0},
	{"通道1:"	,0,0	, -100,+100,&Model.Neu[0],0,0},
	{"通道2:"	,0,0	, -100,+100,&Model.Neu[1],0,0},
	{"通道3:"	,0,0	, -100,+100,&Model.Neu[2],0,0},
	{"通道4:"	,0,0	, -100,+100,&Model.Neu[3],0,0},
	{"通道5:"	,0,0	, -100,+100,&Model.Neu[4],0,0},
	{"通道6:"	,0,0	, -100,+100,&Model.Neu[5],0,0},
	{"通道7:"	,0,0	, -100,+100,&Model.Neu[6],0,0},
	{"通道8:"	,0,0	, -100,+100,&Model.Neu[7],0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageNeuSet(u8 event)
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
		ms.Total=MenuNeuSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuNeuSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}