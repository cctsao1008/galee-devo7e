#include "Page.h"
#include "Func.h"

CMENUITEM MenuSwashSet[]={
	{"斜盘设置"	,0,0,0,4,0,0,0},
	{"斜盘类型:"	,0,PageOptionSwash,0,3,&Model.SwashType,0,0},
	{"副翼混控比:",0,0, -100,+100,&Model.Swash[0],0,0},
	{"升降混控比:",0,0, -100,+100,&Model.Swash[1],0,0},
	{"螺距混控比:",0,0, -100,+100,&Model.Swash[2],0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  陀螺仪菜单处理
//
u32 PageSwashSet(u8 event)
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
		ms.Total=MenuSwashSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuSwashSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}