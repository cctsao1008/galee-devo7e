#include "Page.h"
#include "Func.h"

CMENUITEM MenuHeliSet[]={
	{"直升机参数调节"	,0,0,0,11,0,0,0},
	{"正反设置"	,PageNorRevSet		, 0,0,0,0,0,0},
	{"行程设置"	,PageEpaSet			, 0,0,0,0,0,0},
	{"中点调节"	,PageNeuSet			, 0,0,0,0,0,0},
	{"陀螺仪"	,PageGyroSet		, 0,0,0,0,0,0},
	{"摇杆曲线"	,PageCurveSet		, 0,1,9,0,0,0},
	{"油门曲线"	,PageCurveSet		, 0,2,2,0,0,0},
	{"螺距曲线"	,PageCurveSet		, 0,3,3,0,0,0},
	{"油门锁定:",0,0,-50,50,&Model.ThrHold,0,0},
	{"斜盘混控"	,PageSwashSet		, 0,0,0,0,0,0},
	{"辅助通道" ,PageAuxChSet,0,0,0,0,0,0},
	{"舵机减速"	,PageDelaySet		, 0,0,0,0,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageHeliSet(u8 event)
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
		ms.Total=MenuHeliSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuHeliSet,&ms);
		
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{
		//如果有子菜单或子功能，进入
		if(MenuHeliSet[ms.iFocus+1].SubItem)
		{
			if(MenuHeliSet[ms.iFocus+1].SubItem==PageCurveSet)
			{
				PageCurveCnt	=MenuHeliSet[ms.iFocus+1].Max;
				if(MenuHeliSet[ms.iFocus+1].Min==1)
				{
					PageCurveTitle	=PageStkCurveTitle;
					PageCurveValue	=&Model.StkCurve[0][0][0];
				}
				else if(MenuHeliSet[ms.iFocus+1].Min==2)
				{
					PageCurveTitle	=PageThrCurveTitle;
					PageCurveValue	=&Model.ThrCurve[0][0];
				}
				else
				{
					PageCurveTitle	=PagePitCurveTitle;
					PageCurveValue	=&Model.PitCurve[0][0];
				}
			}
			BeepMusic(MusicEnter);			
			PageEnter((PAGEPROC)MenuHeliSet[ms.iFocus+1].SubItem,PV_INIT);
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