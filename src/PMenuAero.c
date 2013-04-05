#include "Page.h"
#include "Func.h"

CMENUITEM MenuAeroSet[]={
	{"固定翼参数调节"	,0,0,0,9,0,0,0},
	{"正反设置"	,PageNorRevSet		, 0,0,0,0,0,0},
	{"行程设置"	,PageEpaSet			, 0,0,0,0,0,0},
	{"中点调节"	,PageNeuSet			, 0,0,0,0,0,0},
	{"摇杆曲线"	,PageCurveSet		, 0,1,9,0,0,0},
	{"油门曲线"	,PageCurveSet		, 0,2,1,0,0,0},
	{"油门锁定:",0,0,-50,50,&Model.ThrHold,0,0},
	{"辅助通道",PageAuxChSet,0,0,0,0,0,0},
	{"混控设置",PageMixerSet,0,0,0,0,0,0},
	{"舵机减速"	,PageDelaySet			, 0,0,0,0,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageAeroSet(u8 event)
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
		ms.Total=MenuAeroSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuAeroSet,&ms);
		
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{
		//如果有子菜单或子功能，进入
		if(MenuAeroSet[ms.iFocus+1].SubItem)
		{
			if(MenuAeroSet[ms.iFocus+1].SubItem==PageCurveSet)
			{
				PageCurveCnt	=MenuAeroSet[ms.iFocus+1].Max;
				if(MenuAeroSet[ms.iFocus+1].Min==1)
				{
					PageCurveTitle	=PageStkCurveTitle;
					PageCurveValue	=&Model.StkCurve[0][0][0];
				}
				else
				{
					PageCurveTitle	=PageThrCurveTitle;
					PageCurveValue	=&Model.ThrCurve[0][0];
				}
			}
			BeepMusic(MusicEnter);			
			PageEnter((PAGEPROC)MenuAeroSet[ms.iFocus+1].SubItem,PV_INIT);
		}				
	}	
	
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}