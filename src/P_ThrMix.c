#include "Page.h"
#include "Func.h"

CMENUITEM MenuThrMixSet[]={
	{"油门\2平衡混控"	,0,0,0,3,0,0,0},
	{"抗反扭-副翼:"	,0,0	, -MAX_THM,+MAX_THM,&Model.ThrMix[0],0,0},
	{"抗偏航-方向:"	,0,0	, -MAX_THM,+MAX_THM,&Model.ThrMix[1],0,0},
	{"抗起伏-升降:"	,0,0	, -MAX_THM,+MAX_THM,&Model.ThrMix[2],0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  油门混控菜单处理过程
//
u32 PageThrMixSet(u8 event)
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
		ms.Param=0xff;
		ms.Total=MenuThrMixSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
		
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuThrMixSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}