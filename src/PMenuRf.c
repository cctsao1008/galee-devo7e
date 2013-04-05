#include "Page.h"
#include "Func.h"
#include "Tx.h"

s16 PageMenuRfProto;
s16 PageMenuRfChNum;
extern u32 PageEditIdValue;
char PageMenuRfBindTitle[16];
CMENUITEM MenuRf[]={
	{"射频设置",0,0,0,5,0,0,0},
	{PageMenuRfBindTitle,PageBind,0,0,0,0,0,0},
	{"通讯协议:",0,PageOptionProtocol	, 0,9,&PageMenuRfProto,0,0},
	{"通 道 数:",0,0,4,8,&PageMenuRfChNum,0,0},
	{"发射功率:",0,PageOptionRfPwr		, 0,7,&Model.RfPwr,0,0},
	{"识 别 码:",PageEditId,0,0,0,0,0,Model.RfIdStr},
};

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型基本配置菜单处理过程
//
void PageMenuRfIdStr(void)
{
	if(PageEditIdValue==0)
	{
		strcpy(Model.RfIdStr,"(自动)");
		return;
	}
	
	u8 i;
	u32 mask=100000;
	for(i=0;i<MODELIDL;i++)
	{
		Model.RfIdStr[i]=PageEditIdValue/mask%10+'0';	
		mask/=10;
	}
}

u32 PageMenuRf(u8 event)
{
	static u8 CfgChanged;
	static MENUSTAT ms;
			
	if(event==PV_INIT)
	{
		LcdClear(0);
		if(!ms.Init)
		{
			ms.iFocus=ms.iStart=0;
			ms.Init=1;
		}
		ms.Total=MenuRf[0].Max;
		ms.DrawMask=PD_ALL;	
		PageMenuRfProto=Model.Protocol;
		PageMenuRfChNum=Model.RfChNum;
		PageEditIdValue=Model.RfId;
		CfgChanged=1;
		return 1;
	}
	
	//检测配置变化
	if(PageMenuRfProto!=Model.Protocol || PageMenuRfChNum!=Model.RfChNum || PageEditIdValue!=Model.RfId)
	{
		if(!CfgChanged) 
		{
			strcpy(PageMenuRfBindTitle,"变更射频设置");
			ms.DrawMask=PD_ALL;	
		}
		CfgChanged=1;		
	}
	else
	{
		if(CfgChanged)
		{
			strcpy(PageMenuRfBindTitle,"启动对码");
			ms.DrawMask=PD_ALL;	
		}
		CfgChanged=0;
	}
	
	//将ID字符化
	PageMenuRfIdStr();
	
	if(event==PV_REDRAW)
	{
		ms.DrawMask=PD_ALL;	
	}
	
	//菜单通用处理过程
	PageMenuProc(MenuRf,&ms);
	
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{			
		//进入对码
		if(MenuRf[ms.iFocus+1].SubItem==PageBind)
		{
			if(CfgChanged)
			{
				Model.RfId=PageEditIdValue;
				Model.Protocol=PageMenuRfProto;
				Model.RfChNum=PageMenuRfChNum;
				TxLoad(Model.Protocol);
			}
			else
			{
				BeepMusic(MusicEnter);	
				PageEnter(PageBind,PV_INIT);
			}
		}		
		if(MenuRf[ms.iFocus+1].SubItem==PageEditId)
		{
			BeepMusic(MusicEnter);
			PageEnter(PageEditId,PV_INIT);
		}
	}	
	
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		if(CfgChanged)
		{
			PageAlert("没有保存改动!",1000,PV_END);
		}
		else PageReturn(PV_INIT);
	}
	
	if(event==PV_END)
	{
		PageReturn(PV_INIT);
	}
	
	KeyClearDown(KEY_MENUALL);
	
	return 0;
}