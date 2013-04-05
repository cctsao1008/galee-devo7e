#include "Page.h"
#include "Func.h"

CMENUITEM MenuEpaSet[]={
	{"行程设置"	,0,0,0,16,0,0,0},
	{"通道1 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[0][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[0][1],0,0},
	{"通道2 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[1][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[1][1],0,0},
	{"通道3 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[2][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[2][1],0,0},
	{"通道4 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[3][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[3][1],0,0},
	{"通道5 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[4][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[4][1],0,0},
	{"通道6 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[5][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[5][1],0,0},
	{"通道7 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[6][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[6][1],0,0},
	{"通道8 低:"	,0,0	, 0,+MAX_EPA,&Model.Epa[7][0],0,0},
	{"      高:"	,0,0	, 0,+MAX_EPA,&Model.Epa[7][1],0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageEpaSet(u8 event)
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
		ms.Total=MenuEpaSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuEpaSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}