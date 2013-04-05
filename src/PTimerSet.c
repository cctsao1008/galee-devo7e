#include "Page.h"
#include "Func.h"

s16 PageTimerSetH,PageTimerSetS;

CMENUITEM MenuTimer1[]={
	{"定时器1设置",0,0,0,5,0,0,0},
	{"类型:"	,0,PageOptionTmrType	, 0, 3,&Model.Timer[0].Type,0,0},
	{"控制开关:",0,PageOptionTmrSw		, 0, 4,&Model.Timer[0].Sw,0,0},
	{"油门位置:",0,0,-100,100,&Model.Timer[0].Thr,0,0},
	{"时间(分):",0,0,1,99,&PageTimerSetH,0,0},
	{"    (秒):",0,0,0,59,&PageTimerSetS,0,0},
};

CMENUITEM MenuTimer2[]={
	{"定时器2设置",0,0,0,4,0,0,0},
	{"类型:"	,0,PageOptionTmrType	, 0, 3,&Model.Timer[1].Type,0,0},
	{"控制开关:",0,PageOptionTmrSw		, 0, 4,&Model.Timer[1].Sw,0,0},
	{"油门位置:",0,0,-100,100,&Model.Timer[1].Thr,0,0},
	{"时间(分):",0,0,1,99,&PageTimerSetH,0,0},
	{"    (秒):",0,0,0,59,&PageTimerSetS,0,0},
};
CMENUITEM MenuTimer3[]={
	{"定时器3设置",0,0,0,5,0,0,0},
	{"类型:"	,0,PageOptionTmrType	, 0, 3,&Model.Timer[2].Type,0,0},
	{"控制开关:",0,PageOptionTmrSw		, 0, 4,&Model.Timer[2].Sw,0,0},
	{"油门位置:",0,0,-100,100,&Model.Timer[2].Thr,0,0},
	{"时间(分):",0,0,1,99,&PageTimerSetH,0,0},
	{"    (秒):",0,0,0,59,&PageTimerSetS,0,0},
};
///////////////////////////////////////////////////////////////////////////////////////
//
//  定时器设置处理过程
//

u32 PageTimerSet1(u8 event)
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
		ms.Total=MenuTimer1[0].Max;
		ms.DrawMask=PD_ALL;	
		PageTimerSetH=Model.Timer[0].Time/60;
		PageTimerSetS=Model.Timer[0].Time%60;
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuTimer1,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT) || KeyTstDown(KEY_ENT))
	{
		TxTimer[0].Reset=1;
		PageReturn(PV_INIT);
		Model.Timer[0].Time=PageTimerSetH*60+PageTimerSetS;
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}

u32 PageTimerSet2(u8 event)
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
		ms.Total=MenuTimer2[0].Max;
		ms.DrawMask=PD_ALL;	
		PageTimerSetH=Model.Timer[1].Time/60;
		PageTimerSetS=Model.Timer[1].Time%60;
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuTimer2,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT) || KeyTstDown(KEY_ENT))
	{
		TxTimer[1].Reset=1;
		PageReturn(PV_INIT);
		Model.Timer[1].Time=PageTimerSetH*60+PageTimerSetS;
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}

u32 PageTimerSet3(u8 event)
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
		ms.Total=MenuTimer3[0].Max;
		ms.DrawMask=PD_ALL;	
		PageTimerSetH=Model.Timer[2].Time/60;
		PageTimerSetS=Model.Timer[2].Time%60;
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuTimer3,&ms);
		
	//返回键处理
	if(KeyTstDown(KEY_EXT) || KeyTstDown(KEY_ENT))
	{
		TxTimer[2].Reset=1;
		PageReturn(PV_INIT);
		Model.Timer[2].Time=PageTimerSetH*60+PageTimerSetS;
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}