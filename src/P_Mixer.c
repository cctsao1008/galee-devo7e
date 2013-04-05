#include "Page.h"
#include "Func.h"

char MenuMixerDeltaStat[6];
char MenuMixerVtailStat[6];
char MenuMixerFlpAilStat[6];

CMENUITEM MenuMixerSet[]={
	{"混控设置"	,0,0,0,5,0,0,0},
	{"三角翼混控",PageDeltaSet,0,0,0,0,0,MenuMixerDeltaStat},
	{" V 尾 混控",PageVtailSet,0,0,0,0,0,MenuMixerVtailStat},
	{"襟副翼混控",PageFlpAilSet,0,0,0,0,0,MenuMixerFlpAilStat},
	{"油门\2平衡混控"  ,PageThrMixSet,0,0,0,0,0,0},
	{"转向\2升降混控"  ,PageYawMixSet,0,0,0,0,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  混控菜单处理过程
//
u32 PageMixerSet(u8 event)
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
		ms.Total=MenuMixerSet[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//设置各混控状态
	strcpy(MenuMixerVtailStat,PageOptionOnOff[Model.Vtail.Enable]);
	strcpy(MenuMixerDeltaStat,PageOptionOnOff[Model.Delta.Enable]);
	strcpy(MenuMixerFlpAilStat,PageOptionOnOff[Model.FlpAil.Enable]);
	
	//菜单通用处理过程
	PageMenuProc(MenuMixerSet,&ms);
		
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{
		//如果有子菜单或子功能，进入
		if(MenuMixerSet[ms.iFocus+1].SubItem)
		{
			BeepMusic(MusicEnter);			
			PageEnter((PAGEPROC)MenuMixerSet[ms.iFocus+1].SubItem,PV_INIT);
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