#include "Page.h"
#include "Func.h"

CMENUITEM MenuYawMixSet[]={
	{"转向\2升降混控"	,0,0,0,4,0,0,0},
	{"副翼-左:"	,0,0, -MAX_YWM,+MAX_YWM,&Model.YawMix.Al2E,0,0},
	{"    -右:"	,0,0, -MAX_YWM,+MAX_YWM,&Model.YawMix.Ar2E,0,0},
	{"方向-左:"	,0,0, -MAX_YWM,+MAX_YWM,&Model.YawMix.Rl2E,0,0},
	{"    -右:"	,0,0, -MAX_YWM,+MAX_YWM,&Model.YawMix.Rr2E,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageYawMixSet(u8 event)
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
		ms.Total=MenuYawMixSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}	
	
	//菜单通用处理过程
	PageMenuProc(MenuYawMixSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}