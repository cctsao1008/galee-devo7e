#include "Page.h"
#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  摇杆校准
//
u32 PageStkCal(u8 event)
{
	u8 i;
	static s8 StkCalStep,Step;
	static s16 stkmax[STK_NUM],stkmin[STK_NUM],stkmid[STK_NUM];
	
	if(event==PV_INIT)
	{
		StkCalStep=0;
		Step=-1;
		
		LcdClear(0);
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		LcdDrawText(3,0,"摇杆校准");
		LcdDrawHLine(0,128,14,1);
		LcdDrawHLine(0,128,15,1);
		LcdDrawStop();
		
		stkmin[0]=5000;
		stkmin[1]=5000;
		stkmin[2]=5000;
		stkmin[3]=5000;
		stkmax[0]=0;
		stkmax[1]=0;
		stkmax[2]=0;
		stkmax[3]=0;
		
		return 1;
	}
		
	if(Step!=StkCalStep)
	{
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		if(StkCalStep==0)
		{
			LcdDrawBmpFile(8,18,"res/stk.bmp");
			LcdDrawText(36,18,"将所有摇杆置中");
			LcdDrawText(42,32,"然后按ENT键");
		}
		if(StkCalStep==1)
		{
			LcdDrawBmpFile(8,18,"res/stkmax.bmp");
			LcdDrawText(36,18,"打满各摇杆行程");
			LcdDrawText(42,32,"然后按ENT键");
		}
		LcdDrawStop();
		Step=StkCalStep;
	}
	
	//记录最大最小值
	for(i=0;i<STK_NUM;i++)
	{
		if(StickRaw[i]>stkmax[i])	stkmax[i]=StickRaw[i];
		if(StickRaw[i]<stkmin[i])	stkmin[i]=StickRaw[i];
	}
		
	//返回键处理
	if(KeyTstDown(KEY_EXT) || event==PV_END)
	{
		PageReturn(PV_REDRAW);
	}
	
	//回车键处理
	if(KeyTstDown(KEY_ENT))
	{
		StkCalStep++;
		if(StkCalStep==1)//记录中点
		{
			for(i=0;i<STK_NUM;i++)
			{
				stkmid[i]=StickRaw[i];
			}
		}
		if(StkCalStep>=2)
		{
			for(i=0;i<STK_NUM;i++)
			{
				TxSys.StkCali[i][1]=stkmid[i];
				TxSys.StkCali[i][0]=stkmid[i]-stkmin[i];
				TxSys.StkCali[i][2]=stkmax[i]-stkmid[i];
			}
			PageAlert("摇杆校准完成!",1000,PV_END);
			TxSys.StkCalied=CALMARK;
		}
	}
	
	//100ms定时处理
	if(SysTimerClk%100==0)
	{		
		LcdDrawStart(0, 50,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		LcdDrawRect(0, 50,127,63,0);
		for(i=0;i<STK_NUM;i++)
		{		
    		LcdDrawMiniInt(i*20+22,50,StickRaw[i],0,0,0,1);
    		LcdDrawMiniInt(i*20+22,58,StickCal[i],0,0,1,1);
    	}
		LcdDrawStop();
	}
	
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}