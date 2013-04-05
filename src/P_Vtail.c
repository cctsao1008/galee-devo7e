#include "Page.h"
#include "Func.h"

CMENUITEM MenuVtailSet[]={
	{" V 尾 混控"	,0,0,0,5,0,0,0},
	{"混控开关:"	,0,PageOptionOnOff,0,1,&Model.Vtail.Enable,0,0},
	{"升降\2升降:"	,0,0, -MAX_VTL,+MAX_VTL,&Model.Vtail.E2E,0,0},
	{"方向\2升降:"	,0,0, -MAX_VTL,+MAX_VTL,&Model.Vtail.R2E,0,0},
	{"升降\2方向:"	,0,0, -MAX_VTL,+MAX_VTL,&Model.Vtail.E2R,0,0},
	{"方向\2方向:"	,0,0, -MAX_VTL,+MAX_VTL,&Model.Vtail.R2R,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageVtailSet(u8 event)
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
		ms.Total=MenuVtailSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	//根据使能开关确定菜单数
	if(Model.Vtail.Enable)	ms.Total=MenuVtailSet[0].Max;
	else					ms.Total=1;
	
	//菜单数变化重绘
	if(ms.Param!=ms.Total)
	{
		LcdClear(0);
		ms.Param=ms.Total;
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuVtailSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		//V尾混控和三角翼混控互斥
		if(Model.Vtail.Enable) Model.Delta.Enable=0;
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}