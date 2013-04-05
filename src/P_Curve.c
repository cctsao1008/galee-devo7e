#include "Page.h"
#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  根据EPA/EXP调节曲线
//
s32 CurveExpo(s16 i,s16 e)
{
	return e*i*i*i+(100-e)*i;
}

void CurveMake(s16 *curve,s16 epa,s16 exp)
{
	s32 i;
	s32 expmax=CurveExpo(4,exp);
	for(i=0;i<CURVE_NUM;i++)	
	{
		curve[i]=500+CurveExpo(i-4,exp)*epa*5/expmax;
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  曲线设置
//
#define LCD_CURVE_LINES 6
#define LCD_CV_X 79
#define LCD_CV_Y 14
#define LCD_CV_W 48
#define LCD_CV_H 48

#define	LCD_CV_IX 44

#define CURVE_FUNC	4

CSTR *PageCurveTitle;
u8  PageCurveCnt;
s16 *PageCurveValue;

u32 PageCurveSet(u8 event)
{
	static u8 DrawMask;
	static u8 iStk;
	static u8 iFocus,Total,iStart;
	
	if(event==PV_INIT)
	{
		LcdClear(0);
		iStk=0;
		iFocus=CURVE_NUM;
		iStart=0;
		Total=CURVE_NUM+CURVE_FUNC;
		DrawMask=PD_ALL;	
		return 1;
	}
		
	if(event==PV_USER)
	{
		LcdClear(0);
		DrawMask=PD_ALL;
	}
	
	//开始绘图
	if(DrawMask)    
	{
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
	
		//标题绘制
		if(DrawMask&PD_TITLE)
		{
			LcdDrawRect(0, 0,127,13,0);
			LcdDrawText(2,0,(char*)PageCurveTitle[iStk]);
			if(PageCurveCnt>1)	LcdDrawText(96,0,"\1ENT\x2");
			LcdDrawHLine(0,LCD_CV_X-2,14,1);
			LcdDrawHLine(0,LCD_CV_X-2,15,1);
		}
		
		//曲线点列表
		if(DrawMask&(PD_LIST|PD_VALUE))
		{			
			u16 i,x,y,idx;
			for(i=0,y=18;i<LCD_CURVE_LINES ;i++,y+=7)
			{
				idx=iStart+i;
				LcdBw=(idx==iFocus);
				
				LcdDrawRect(LCD_CV_IX,y,LCD_CV_IX+31,y+7,LcdBw);				//绘制选中框
				LcdDrawMiniInt(LCD_CV_IX+3,y+1,idx+1,1,0,0xff,0);				//绘制曲线序号
				LcdDrawMiniNum(LCD_CV_IX+7,y+1,LCD_MN_COL);						//绘制分隔点
				LcdDrawMiniInt(LCD_CV_IX+30,y+1,PageCurveValue[iStk*CURVE_DS+idx],0,1,0,1);	//绘制曲线值
				LcdBw=0;
			}
			
			//绘制EXP EPA菜单
			LcdBw=(iFocus==CURVE_NUM);
			LcdDrawRect(3,20,LCD_CV_IX-4,26,LcdBw);	
			LcdDrawMiniEng(5,21,"EPA\x12");
			LcdDrawMiniInt(40,21,PageCurveValue[iStk*CURVE_DS+CURVE_NUM],0,0,1,1);	//绘制EPA
			
			LcdBw=(iFocus==CURVE_NUM+1);
			LcdDrawRect(3,30,LCD_CV_IX-4,36,LcdBw);	
			LcdDrawMiniEng(5,31,"EXP\x12");
			LcdDrawMiniInt(40,31,PageCurveValue[iStk*CURVE_DS+CURVE_NUM+1],0,0,1,1);//绘制EXP

			LcdBw=(iFocus==CURVE_NUM+2);
			LcdDrawRect(3,40,LCD_CV_IX-4,46,LcdBw);	
			LcdDrawMiniEng(5,41,"LEVEL");
			
			LcdBw=(iFocus==CURVE_NUM+3);
			LcdDrawRect(3,50,LCD_CV_IX-4,56,LcdBw);	
			LcdDrawMiniEng(5,51,"RESET");
			
			LcdBw=0;
			
			//绘制曲线
			LcdDrawRect(LCD_CV_X,LCD_CV_Y,LCD_CV_X+LCD_CV_W,LCD_CV_Y+LCD_CV_H,1);
			LcdDrawRect(LCD_CV_X+1,LCD_CV_Y+1,LCD_CV_X+LCD_CV_W-1,LCD_CV_Y+LCD_CV_H-1,0);
			LcdDrawHLine(LCD_CV_X,LCD_CV_X+LCD_CV_W,LCD_CV_Y+LCD_CV_H/2,1);
			LcdDrawVLine(LCD_CV_X+LCD_CV_W/2,LCD_CV_Y,LCD_CV_Y+LCD_CV_H,1);
			u16 x0,y0;
			for(i=0,x0=y0=0;i<CURVE_NUM;i++)
			{
				x=i*LCD_CV_W/(CURVE_NUM-1);
				y=LCD_CV_H-(s32)PageCurveValue[iStk*CURVE_DS+i]*LCD_CV_H/CURVE_TRV;
				if(y>LCD_CV_H) y=LCD_CV_H;
				LcdDrawLine(LCD_CV_X+x0,LCD_CV_Y+y0,LCD_CV_X+x,LCD_CV_Y+y,1);
				if(i==iFocus) LcdDrawVLine(LCD_CV_X+x,LCD_CV_Y,LCD_CV_Y+LCD_CV_H,1);
				x0=x;
				y0=y;
			}
		}
		//完成绘图		
		LcdDrawStop();
		DrawMask=0;
	}
			
	//加减按键处理
	s16 v=PageCurveValue[iStk*CURVE_DS+iFocus];
	///////////////////////////////////////////////////////////////
	if(iFocus<CURVE_NUM)//曲线值
	{
		if(KeyTstHold(KEY_R))
		{
			v=(v/10+1)*10;
			if(v>CURVE_TRV)	v=CURVE_TRV;
			DrawMask=PD_LIST;
		}
		else if(KeyTstDown(KEY_R))
		{
			if(v<CURVE_TRV)	v++;		
			DrawMask=PD_LIST;
		}
		if(KeyTstHold(KEY_L))
		{
			v=(v/10-1)*10;
			if(v<0)	v=0;
			DrawMask=PD_LIST;
		}
		else if(KeyTstDown(KEY_L))
		{
			if(v>0)	v--;		
			DrawMask=PD_LIST;
		}
	}
	///////////////////////////////////////////////////////////////
	else//EXP EPA等
	{
		////////////////////////////////////////
		if(iFocus==CURVE_NUM+3)//RESET
		{
			if(KeyTstDown(KEY_R) || KeyTstDown(KEY_L))
			{
				u32 i;
				for(i=0;i<CURVE_NUM;i++)	PageCurveValue[iStk*CURVE_DS+i]=i*125;
				PageCurveValue[iStk*CURVE_DS+CURVE_NUM  ]=100;		//epa
				PageCurveValue[iStk*CURVE_DS+CURVE_NUM+1]=0;		//exp
				PageAlert("曲线参数已复位!",500,PV_USER);
			}
		}
		////////////////////////////////////////
		if(iFocus==CURVE_NUM+2)//LEVEL
		{
			s16 delta=0;
			if(KeyTstHold(KEY_R))		delta=10;
			else if(KeyTstDown(KEY_R))	delta=1;
			
			if(KeyTstHold(KEY_L))		delta=-10;
			else if(KeyTstDown(KEY_L))	delta=-1;
			
			if(delta)
			{
				for(u32 i=0;i<CURVE_NUM;i++)	
				{
					s16 v=PageCurveValue[iStk*CURVE_DS+i];
					v+=delta;
					if(v>1000)	v=1000;
					if(v<0)		v=0;
					PageCurveValue[iStk*CURVE_DS+i]=v;
				}
				DrawMask=PD_LIST;
			}
		}
		////////////////////////////////////////
		if(iFocus==CURVE_NUM+1)//EXP
		{
			if(KeyTstDown(KEY_R))		if(v<50) 	v++;
			if(KeyTstDown(KEY_L))		if(v>0) 	v--;
			if(PageCurveValue[iStk*CURVE_DS+CURVE_NUM+1]!=v)
			{
				CurveMake(&PageCurveValue[iStk*CURVE_DS],PageCurveValue[iStk*CURVE_DS+CURVE_NUM],v);
				DrawMask=PD_LIST;
			}
		}
		////////////////////////////////////////
		if(iFocus==CURVE_NUM)//EPA
		{
			if(KeyTstDown(KEY_R))		if(v<100) 	v++;
			if(KeyTstDown(KEY_L))		if(v>-100) 	v--;
			if(PageCurveValue[iStk*CURVE_DS+CURVE_NUM]!=v)
			{
				CurveMake(&PageCurveValue[iStk*CURVE_DS],v,PageCurveValue[iStk*CURVE_DS+CURVE_NUM+1]);
				DrawMask=PD_LIST;
			}
		}
	}
	PageCurveValue[iStk*CURVE_DS+iFocus]=v;
	
	///////////////////////////////////////////////////////////////
	//上下按键处理
	if(KeyTstDown(KEY_UP))
	{
		if(iFocus>0) iFocus--;
		else		 iFocus=Total-1;
		DrawMask=PD_LIST|PD_IDX;
	}
	if(KeyTstDown(KEY_DW))
	{
		if(iFocus<Total-1) iFocus++;
		else		 		iFocus=0;
		DrawMask=PD_LIST|PD_IDX;
	}
	if(iFocus>=Total)							iFocus=Total-1;
	if(iFocus>=CURVE_NUM)						iStart=0;
	else if(iFocus<iStart) 						iStart=iFocus;
	else if(iFocus>iStart+LCD_CURVE_LINES-1) 	iStart=iFocus-LCD_CURVE_LINES+1;	
	
	///////////////////////////////////////////////////////////////
	//回车键切换曲线		
	if(KeyTstDown(KEY_ENT) )
	{
		iStk++;
		if(iStk>=PageCurveCnt) iStk=0;
		DrawMask|=PD_ALL;
	}
	
	///////////////////////////////////////////////////////////////
	//返回键处理
	if(KeyTstDown(KEY_EXT) )
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}