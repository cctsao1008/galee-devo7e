#include "Page.h"
#include "Func.h"

CMENUITEM *MenuChMap;
CMENUITEM MenuChMapA[]={
	{"固定翼通道映射"	,0,0,0,8,0,0,0},
	{"通道1:",0,PageOptionChSelA		, 0,16,&Model.ChMap[0],0,0},
	{"通道2:",0,PageOptionChSelA		, 0,16,&Model.ChMap[1],0,0},
	{"通道3:",0,PageOptionChSelA		, 0,16,&Model.ChMap[2],0,0},
	{"通道4:",0,PageOptionChSelA		, 0,16,&Model.ChMap[3],0,0},
	{"通道5:",0,PageOptionChSelA		, 0,16,&Model.ChMap[4],0,0},
	{"通道6:",0,PageOptionChSelA		, 0,16,&Model.ChMap[5],0,0},
	{"通道7:",0,PageOptionChSelA		, 0,16,&Model.ChMap[6],0,0},
	{"通道8:",0,PageOptionChSelA		, 0,16,&Model.ChMap[7],0,0},
};

CMENUITEM MenuChMapH[]={
	{"直升机通道映射"	,0,0,0,8,0,0,0},
	{"通道1:",0,PageOptionChSelH		, 0,16,&Model.ChMap[0],0,0},
	{"通道2:",0,PageOptionChSelH		, 0,16,&Model.ChMap[1],0,0},
	{"通道3:",0,PageOptionChSelH		, 0,16,&Model.ChMap[2],0,0},
	{"通道4:",0,PageOptionChSelH		, 0,16,&Model.ChMap[3],0,0},
	{"通道5:",0,PageOptionChSelH		, 0,16,&Model.ChMap[4],0,0},
	{"通道6:",0,PageOptionChSelH		, 0,16,&Model.ChMap[5],0,0},
	{"通道7:",0,PageOptionChSelH		, 0,16,&Model.ChMap[6],0,0},
	{"通道8:",0,PageOptionChSelH		, 0,16,&Model.ChMap[7],0,0},
};
///////////////////////////////////////////////////////////////////////////////////////
//
//  模型通道映射处理过程
//
u32 PageMenuChMap(u8 event)
{
	static MENUSTAT ms;
			
	if(event==PV_INIT)
	{
		if(Model.Type==MT_AERO)	MenuChMap=MenuChMapA;
		else					MenuChMap=MenuChMapH;
		
		LcdClear(0);
		if(!ms.Init)
		{
			ms.iFocus=ms.iStart=0;
			ms.Init=1;
		}
		ms.Total=MenuChMap[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuChMap,&ms);
		
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{
		//如果有子菜单或子功能，进入
		if(MenuChMap[ms.iFocus+1].SubItem)
		{
			BeepMusic(MusicEnter);			
			PageEnter((PAGEPROC)MenuChMap[ms.iFocus+1].SubItem,PV_INIT);
		}		
	}
	
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_INIT);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}