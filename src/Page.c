#include "Page.h"
#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
// GUI控制函数
//
PAGEPROC PageStack[PAGELEVEL];
u8 PageStackIdx;
void PageEnter(PAGEPROC page,u8 msg)
{
	if(PageStackIdx>=PAGELEVEL-1)	return;
	PageStack[++PageStackIdx]=page;
	page(msg);
}

void PageGoto(PAGEPROC page,u8 msg)
{
	PageStack[PageStackIdx]=page;	
	page(msg);
}

void PageReturn(u8 msg)
{
	if(PageStackIdx>0)	PageStackIdx--;
	PageStack[PageStackIdx](msg);
}

void PageSet(PAGEPROC page,u8 msg)
{
	PageStack[PageStackIdx=0]=page;
	page(msg);
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  警告对话框
//
char *PageAlertText;
u32   PageAlertDelay;
u8	  PageAlertMsg;

void PageAlertDraw(void)
{
	int s,p,l;		
	l=6*strlen(PageAlertText);
	s=(LCD_W-l)/2;
	p=(LCD_W+l)/2;
	LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  

	if(PageAlertDelay==0)
	{
		LcdDrawRect(s-6, 9,p+6,56,0);
		LcdDrawRect(s-5,10,p+5,55,1);
		LcdDrawRect(s-4,11,p+3,53,0);
		LcdDrawText(s,18,PageAlertText);		//绘制名称	
		LcdBw=1;
		LcdDrawText(LCD_W/2-16,36," EXT ");		//绘制按钮	
		LcdBw=0;
	}
	else
	{
		LcdDrawRect(s-6, 9,p+6,41,0);
		LcdDrawRect(s-5,10,p+5,40,1);
		LcdDrawRect(s-4,11,p+3,38,0);
		LcdDrawText(s,18,PageAlertText);		//绘制名称	
	}
	LcdDrawStop();
}
	
u32 PageAlertProc(u8 event)
{
	static u32 starttime;
	if(event==PV_INIT)
	{
		PageAlertDraw();
		starttime=SysTimerClk;
		return 1;
	}
	
	if(PageAlertDelay==0)
	{
		//按键处理
		if(KeyTstDown(KEY_EXT))
		{
			PageReturn(PageAlertMsg);
		}
	}
	else
	{
		if(SysTimerClk-starttime>PageAlertDelay)
		{
			PageReturn(PageAlertMsg);
		}
	}
	return 0;
}

void PageAlert(CSTR text,u32 delay,u8 retmsg)
{
	if(!text)	return;
	PageAlertText=(char*)text;
	PageAlertDelay=delay;
	PageAlertMsg=retmsg;
	PageEnter(PageAlertProc,PV_INIT);
}

void PageAlertModel(CSTR text,u32 delay)
{
	if(!text)	return;
	PageAlertText=(char*)text;
	PageAlertDelay=delay;
	PageAlertDraw();

	if(PageAlertDelay==0)
	{
		while(!KeyTstStat(KEY_EXT)) 
		{
			SysProc();
			usleep(1000);
		}
	}
	else
	{
		while(delay--)
		{
			SysProc();
			usleep(1000);
		}
	}		
	KeyFlush();	
}
///////////////////////////////////////////////////////////////////////////////////////
//
//  菜单通用过程处理
//
void PageMenuProc(CMENUITEM *pmi,MENUSTAT *pms)
{	
	//开始绘图
	if(pms->DrawMask)    
	{
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
	
		//标题绘制
		if(pms->DrawMask&PD_TITLE)
		{
			LcdDrawRect(0, 0,127,13,0);
			LcdDrawText(3,0,pmi[0].Title);
			LcdDrawHLine(0,127,14,1);
			LcdDrawHLine(0,127,15,1);
		}
		//菜单项索引号
		if(pms->DrawMask&PD_IDX)
		{
			if(pms->Total>9)
			{
				LcdDrawMiniInt(110,4,pms->iFocus+1,2,0,0xff,1);
				LcdDrawMiniNum(110,4,LCD_MN_SPA);
				LcdDrawMiniInt(114,4,pms->Total,2,0,0xff,0);
			}
			else
			{
				LcdDrawMiniInt(120,4,pms->iFocus+1,2,0,0xff,1);
				LcdDrawMiniNum(120,4,LCD_MN_SPA);
				LcdDrawMiniInt(124,4,pms->Total,1,0,0xff,0);
			}
		}
		
		//菜单列表
		if(pms->DrawMask&(PD_LIST|PD_VALUE))
		{
			int i,s,p,y,idx;
			for(i=0,y=16;i<LCD_MENULINES && i<pms->Total;i++,y+=16)
			{
				idx=pms->iStart+i;
				LcdBw=(idx==pms->iFocus);
				idx++;
				
				//如果是绘制整个列表才需要绘制以下内容
				if(pms->DrawMask&PD_LIST || LcdBw)
				{
					LcdDrawRect(1,y,126,y+15,LcdBw);		//绘制选中框
					LcdDrawMiniInt(2,y+4,idx,2,0,0xff,0);	//绘制序号
					LcdDrawMiniNum(10,y+4,LCD_MN_DOT);		//绘制分隔点
					LcdDrawText(13,y,pmi[idx].Title);		//绘制名称
					
					//有子功能的加箭头提示
					if(pmi[idx].SubItem) 
					{
						if(LcdBw)	LcdDrawText(114,y,"\6\2");//>>
						else		LcdDrawText(120,y,"\2");//>
					}
				
					//有文本的选项显示文本
					if(pmi[idx].Text)
					{
						s=110-6*strlen(pmi[idx].Text);
						//LcdDrawMiniInt(60,y+4,s,2,0,0,0);	//绘制序号
						//p=LcdDrawText(60,y,"test");
						p=LcdDrawText(s,y,pmi[idx].Text);
						LcdDrawHLine(s-1,p+1,y+14,0);//画一条下划线
					}
					//没有文本的选项显示值
					else if(pmi[idx].pValue)
					{
						if(pmi[idx].pValue)
						{
							s16 v=*pmi[idx].pValue;
							if(v>pmi[idx].Max) v=pmi[idx].Max;
							if(v<pmi[idx].Min) v=pmi[idx].Min;
							*pmi[idx].pValue=v;
							
							//文字选项的显示文字
							if(pmi[idx].Values)
							{
								s=118-6*strlen(pmi[idx].Values[v]);
								p=LcdDrawText(s,y,(char*)pmi[idx].Values[v]);
							}
							//数字选项的显示数字
							else
							{
								LcdDrawInt(s=90,y,v,3,(u32)pmi[idx].Data,0,0);
								p=90+24+2;
							}
							
							//选中的时候显示编辑状态
							if(LcdBw)
							{
								if(v==pmi[idx].Max)
									LcdDrawText(120,y,"\4");//SPIN
								else if(v==pmi[idx].Min)
									LcdDrawText(120,y,"\5");//SPIN
								else
									LcdDrawText(120,y,"\3");//SPIN
								LcdDrawHLine(s-1,p+1,y+14,0);//画一条下划线
							}
						}
					}
				}
				LcdBw=0;
			}
		}
		
		//完成绘图		
		LcdDrawStop();
		pms->DrawMask=0;
	}
	
	//加减按键处理
	MENUITEM *mi=(MENUITEM *)&pmi[pms->iFocus+1];
	if(KeyTstDown(KEY_R))
	{
		if(mi->pValue)
		{
			if(*mi->pValue<mi->Max) 
			{
				(*mi->pValue)++;
				pms->DrawMask|=PD_VALUE;
			}
		}
	}
	if(KeyTstDown(KEY_L))
	{
		if(mi->pValue)
		{
			if(*mi->pValue>mi->Min) 
			{
				(*mi->pValue)--;
				pms->DrawMask|=PD_VALUE;
			}
		}
	}
	
	//上下按键处理
	if(KeyTstDown(KEY_UP))
	{
		if(pms->iFocus>0) pms->iFocus--;
		else		 pms->iFocus=pms->Total-1;
		pms->DrawMask=PD_LIST|PD_IDX;
	}
	if(KeyTstDown(KEY_DW))
	{
		if(pms->iFocus<pms->Total-1) pms->iFocus++;
		else		 		pms->iFocus=0;
		pms->DrawMask=PD_LIST|PD_IDX;
	}
	if(pms->iFocus>=pms->Total)	pms->iFocus=pms->Total-1;
	if(pms->iFocus<pms->iStart) 				pms->iStart=pms->iFocus;
	if(pms->iFocus>pms->iStart+LCD_MENULINES-1) pms->iStart=pms->iFocus-LCD_MENULINES+1;	
}	

//菜单选项
CSTR PageOptionOnOff[]={"关闭","开启"};
CSTR PageOptionNorRev[]={"正 --","-- 反"};
CSTR PageOptionOkExe[]={"不执行","执行","全部"};
CSTR PageOptionStkType[]={"日本手","美国手","中国手","日本反手"};
CSTR PageOptionBatType[]={"4S 镍电","4S 碱性","1S 锂电","2S 锂电"};
CSTR PageOptionModType[]={"固定翼","直升机"};
CSTR PageOptionSwDef[]={"禁用","HOLD","FMOD","左3档","右3档"};
CSTR PageOptionProtocol[]={"关闭","PPM","DEVO","DSM2","DSMX","WK2801","FlySky","Hubsan","SkyAtc","J6Pro"};
CSTR PageOptionRfPwr[]={"100uW","300uW","1mW","3mw","10mW","30mW","100mW","150mW"};
CSTR PageOptionChSelA[]={"副翼","升降","油门","方向","起落架","襟翼","辅助1","辅助2","摇摆","PPM CH1","PPM CH2","PPM CH3","PPM CH4","PPM CH5","PPM CH6","PPM CH7","PPM CH8"};
CSTR PageOptionChSelH[]={"副翼","升降","油门","方向","陀螺"  ,"螺距","辅助1","辅助2","摇摆","PPM CH1","PPM CH2","PPM CH3","PPM CH4","PPM CH5","PPM CH6","PPM CH7","PPM CH8"};
CSTR PageOptionAuxSrc[]={"0%","+100%","-100%","重力X","重力Y"};
CSTR PageOptionTmrType[]={"禁用","隐藏","计时","倒计时"};
CSTR PageOptionTmrSw[]={"油门","HOLD0","HOLD1","FMOD0","FMOD1"};
CSTR PageOptionSwash[]={"单舵机","120\7CCPM","140\7CCPM","90\7CCPM"};
CSTR PageOptionLightOff[]={"常亮","10秒","20秒","30秒","40秒","50秒","60秒"};
CSTR PageStkCurveTitle[]={"副翼DR0曲线","副翼DR1曲线","副翼DR2曲线","升降DR0曲线","升降DR1曲线","升降DR2曲线","方向DR0曲线","方向DR1曲线","方向DR2曲线"};
CSTR PageThrCurveTitle[]={"常规油门曲线","特技油门曲线"};
CSTR PagePitCurveTitle[]={"锁定螺距曲线","常规螺距曲线","特技螺距曲线"};
CSTR PageOptionAutoOff[]={"禁用","10分","20分","30分"};
CSTR PageOptionFreeWarn[]={"禁用","1分","2分","3分","4分","5分"};