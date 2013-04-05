#include "Page.h"
#include "Func.h"

CMENUITEM MenuFlpAilSet[]={
	{"襟副翼混控"	,0,0,0,5,0,0,0},
	{"混控开关:"	,0,PageOptionOnOff,0,1,&Model.FlpAil.Enable,0,0},
	{"副翼\2副翼:"	,0,0, -MAX_FAM,+MAX_FAM,&Model.FlpAil.A2A,0,0},
	{"副翼\2襟翼:"	,0,0, -MAX_FAM,+MAX_FAM,&Model.FlpAil.A2F,0,0},
	{"襟翼\2副翼:"	,0,0, -MAX_FAM,+MAX_FAM,&Model.FlpAil.F2A,0,0},
	{"襟翼\2襟翼:"	,0,0, -MAX_FAM,+MAX_FAM,&Model.FlpAil.F2F,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageFlpAilSet(u8 event)
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
		ms.Total=MenuFlpAilSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	//根据使能开关确定菜单数
	if(Model.FlpAil.Enable)	ms.Total=MenuFlpAilSet[0].Max;
	else					ms.Total=1;
	
	//菜单数变化重绘
	if(ms.Param!=ms.Total)
	{
		LcdClear(0);
		ms.Param=ms.Total;
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuFlpAilSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		//襟副翼混控和三角翼混控互斥
		if(Model.FlpAil.Enable) Model.Delta.Enable=0;
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}