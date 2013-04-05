#include "Page.h"
#include "Func.h"
#include "Tx.h"

char PageMenuModelCur[10]="00号模型";
s16 PageMenuModelReset=0;
CMENUITEM MenuMod[]={
	{"模型设置"	,0,0,0,12,0,0,0},
	{"选择:"	,PageModelSel			, 0,0,0,0,0,PageMenuModelCur},
	{"名称:"	,PageEditName			, 0,0,0,0,0,Model.Name},
	{"类型:"	,0,PageOptionModType	, 0,1,&Model.Type,0,0},
	{"图标:"	,PageIconSel			, 0,0,0,0,0,Model.Icon},
	{"射频设置" ,PageMenuRf,0,0,0,0,0,0},
	{"通道映射"	,PageMenuChMap,0,0,0,0,0,0},
	{"定时器1"	,PageTimerSet1,0,0,0,0,0,0},
	{"定时器2"	,PageTimerSet2,0,0,0,0,0,0},
	{"定时器3"	,PageTimerSet3,0,0,0,0,0,0},
	{"PPM 输入" ,PagePpmIn,0,0,0,0,0,0},
	{"开关选择" ,PageSwDef,0,0,0,0,0,0},
	{"重置模型" ,0,PageOptionOkExe,0,2,&PageMenuModelReset,0,0},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型基本配置菜单处理过程
//
void PageMenuModMarkNo(void)
{
	PageMenuModelCur[0]=TxSys.ModelNo/10+'0';
	PageMenuModelCur[1]=TxSys.ModelNo%10+'0';
}

u32 PageMenuMod(u8 event)
{
	static MENUSTAT ms;
	
	PageMenuModMarkNo();
		
	if(event==PV_INIT)
	{
		LcdClear(0);
		if(!ms.Init)
		{
			ms.iFocus=ms.iStart=0;
			ms.Init=1;
		}
		ms.Total=MenuMod[0].Max;
		ms.DrawMask=PD_ALL;	
		return 1;
	}
		
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuMod,&ms);
		
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{			
		//如果有子菜单或子功能，进入
		if(MenuMod[ms.iFocus+1].SubItem)
		{
			BeepMusic(MusicEnter);			
			PageEnter((PAGEPROC)MenuMod[ms.iFocus+1].SubItem,PV_INIT);
		}		
		
		//复位功能
		if(MenuMod[ms.iFocus+1].pValue==&PageMenuModelReset)
		{
			if(PageMenuModelReset==1)
			{
				PageMenuModelReset=0;
				memcpy(&Model,&ModelDef,sizeof(Model));
				Model.Name[5]=TxSys.ModelNo/10+'0';
				Model.Name[6]=TxSys.ModelNo%10+'0';
				Model.Name[7]=0;
				PageAlert("模型已重置!",1000,PV_INIT);
			}
			if(PageMenuModelReset==2)
			{
				PageMenuModelReset=0;
				ModelFormat();
				PageAlert("所有模型已重置!",1000,PV_INIT);
			}
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