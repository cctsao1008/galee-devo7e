#include "Page.h"
#include "Func.h"

CMENUITEM MenuRevSet[]={
	{"正反设置"	,0,0,0,8,0,0,0},
	{"通道1:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[0],0,0},
	{"通道2:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[1],0,0},
	{"通道3:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[2],0,0},
	{"通道4:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[3],0,0},
	{"通道5:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[4],0,0},
	{"通道6:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[5],0,0},
	{"通道7:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[6],0,0},
	{"通道8:"	,0,PageOptionNorRev	, 0,1,&Model.Rev[7],0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型参数调节菜单处理过程
//
u32 PageNorRevSet(u8 event)
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
		ms.Total=MenuRevSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuRevSet,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}