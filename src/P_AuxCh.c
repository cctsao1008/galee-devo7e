#include "Page.h"
#include "Func.h"

CMENUITEM MenuAeroAuxChSet[]={
	{"辅助通道设置"	,0,0,0,12,0,0,0},
	{"起落架-低:",0,0,-TRV_AUX,TRV_AUX,&Model.Gear[0],0,0},
	{"      -中:",0,0,-TRV_AUX,TRV_AUX,&Model.Gear[1],0,0},
	{"      -高:",0,0,-TRV_AUX,TRV_AUX,&Model.Gear[2],0,0},
	{"襟 翼 -低:",0,0,-TRV_AUX,TRV_AUX,&Model.Flap[0],0,0},
	{"      -中:",0,0,-TRV_AUX,TRV_AUX,&Model.Flap[1],0,0},
	{"      -高:",0,0,-TRV_AUX,TRV_AUX,&Model.Flap[2],0,0},
	{"辅助1 -低:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[0][0],0,0},
	{"      -中:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[0][1],0,0},
	{"      -高:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[0][2],0,0},
	{"辅助2 -低:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[1][0],0,0},
	{"      -中:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[1][1],0,0},
	{"      -高:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[1][2],0,0},
};

CMENUITEM MenuHeliAuxChSet[]={
	{"辅助通道设置"	,0,0,0,6,0,0,0},
	{"辅助1 -低:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[0][0],0,0},
	{"      -中:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[0][1],0,0},
	{"      -高:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[0][2],0,0},
	{"辅助2 -低:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[1][0],0,0},
	{"      -中:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[1][1],0,0},
	{"      -高:",0,0,-TRV_AUX,TRV_AUX,&Model.Aux[1][2],0,0},
};
///////////////////////////////////////////////////////////////////////////////////////
//
//  辅助通道数调节菜单处理过程
//
CMENUITEM *MenuAux;
u32 PageAuxChSet(u8 event)
{
	static MENUSTAT ms;
			
	if(event==PV_INIT)
	{
		LcdClear(0);
		//模型类型变化要重新初始化
		if(ms.Param!=Model.Type)
		{
			ms.Init=0;
			ms.Param=Model.Type;
		}
		
		if(!ms.Init)
		{
			ms.iFocus=ms.iStart=0;
			ms.Init=1;
		}
		MenuAux=(Model.Type==MT_AERO)?MenuAeroAuxChSet:MenuHeliAuxChSet;
		ms.Total=MenuAux[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuAux,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}