#include "Page.h"
#include "Func.h"

CMENUITEM MenuSwDef[]={
	{"选择开关"	,0,0,0,7,0,0,0},
	{"油门锁定:",0,PageOptionSwDef		, 0,4,&Model.SwDef[SWD_HOLD],0,0},
	{"特技飞行:",0,PageOptionSwDef		, 0,4,&Model.SwDef[SWD_IDLE],0,0},
	{"大小舵:",0,PageOptionSwDef		, 0,4,&Model.SwDef[SWD_DR],0,0},
	{"起落架:",0,PageOptionSwDef		, 0,4,&Model.SwDef[SWD_GEAR],0,0},
	{"襟  翼:",0,PageOptionSwDef		, 0,4,&Model.SwDef[SWD_FLAP],0,0},
	{"辅助通道1:",0,PageOptionSwDef		, 0,4,&Model.SwDef[SWD_AUX1],0,0},
	{"辅助通道2:",0,PageOptionSwDef		, 0,4,&Model.SwDef[SWD_AUX2],0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型开关选择处理过程
//
u32 PageSwDef(u8 event)
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
		ms.Total=MenuSwDef[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuSwDef,&ms);
		
	//回车键处理 返回键处理
	if(KeyTstDown(KEY_ENT) || KeyTstDown(KEY_EXT))
	{
		PageReturn(PV_INIT);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}