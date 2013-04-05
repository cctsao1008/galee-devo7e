#include "Page.h"
#include "Func.h"
#include "Tx.h"

#define LCD_MON_TMR		50

#define LCD_MON_ICONL	0x1f
#define LCD_MON_ICONM	0x15
#define LCD_MON_ICONH	0x11
#define LCD_MON_ICONHB	0x1f
#define LCD_MON_ICONR	0x1f
#define LCD_MON_W		60
#define	LCD_MON_SIZE	5

void DrawChMonitor(u16 x,u16 y,s16 idx,s16 value)
{
	s16 i,p,s;
	
	//通道限幅
	if(value>TX_TRV)	value=TX_TRV;
	if(value<-TX_TRV)	value=-TX_TRV;
	
	//计算柱状条起止位置 
    p=value*LCD_MON_W/2/TX_TRV;
    if(value>0)
    {
    	s=LCD_MON_W/2+1;
    	p+=LCD_MON_W/2+1;
    }
    else if(value<0)
    {
    	s=p+LCD_MON_W/2;
    	p=LCD_MON_W/2;
    }
    else
    {
    	s=p=LCD_MON_W/2;
    }
    
    //显示通道名称
    LcdDrawMiniNum(x+ 2,y,LCD_MN_CHAR('C'));
    LcdDrawMiniNum(x+ 6,y,LCD_MN_CHAR('H'));
    LcdDrawMiniNum(x+10,y,idx);
    
    x+=16;
    
    //绘制柱状条
    for(i=0;i<s          ;i++)   LcdDrawMaskY(x+1+i,y,LCD_MON_ICONH ,LCD_MON_SIZE);
    for(   ;i<p          ;i++)   LcdDrawMaskY(x+1+i,y,LCD_MON_ICONHB,LCD_MON_SIZE);
    for(   ;i<=LCD_MON_W;i++)   LcdDrawMaskY(x+1+i,y,LCD_MON_ICONH ,LCD_MON_SIZE);
    LcdDrawMaskY(x,y,LCD_MON_ICONL,LCD_MON_SIZE);	//左边
    LcdDrawMaskY(x+1+LCD_MON_W/2,y,LCD_MON_ICONM,LCD_MON_SIZE); //中点
    LcdDrawMaskY(x+2+LCD_MON_W,y,LCD_MON_ICONR,LCD_MON_SIZE);   //右边    
    LcdDrawMiniInt(x+LCD_MON_W+5,y,(s32)value*100/STK_TRV,3,0,1,0);
}
///////////////////////////////////////////////////////////////////////////////////////
//
//  通道监视器
//
u32 PageMonitor(u8 event)
{	
	static u32 montmr;
	
	if(event==PV_INIT)
	{
		montmr=SysTimerClk+LCD_MON_TMR;
		LcdClear(0);
		LcdDrawBmpFile(0,0,"res/svrmon.bmp");	
		return 1;
	}
	
	if(SysTimerClk>montmr)
	{		
		u8 i;
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE); 
		for(i=0;i<TX_CH_NUM;i++)
		{
			DrawChMonitor(30,2+i*8,i+1,TxChValue[i]);
		}		
		LcdDrawStop();
		
		montmr+=LCD_MON_TMR;
	}
	
	//返回键处理
	if(KeyTstDown(KEY_EXT) || KeyTstDown(KEY_ENT))
	{
		PageReturn(PV_INIT);
	}
	
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  PPM通道监视器
//
u32 PagePpmIn(u8 event)
{	
	static u32 montmr;
	static u32 freqtmr;
	
	if(event==PV_INIT)
	{
		montmr=SysTimerClk+LCD_MON_TMR;
		LcdClear(0);
		LcdDrawBmpFile(0,0,"res/ppmmon.bmp");	
		return 1;
	}
	
	
	if(SysTimerClk>montmr)
	{		
		u8 i;
		
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE); 
		for(i=0;i<TX_CH_NUM;i++)
		{
			DrawChMonitor(30,2+i*8,i+1,PpmInValue[i]);
		}		
		LcdDrawMiniInt(3,31,PpmInCh,3,0,0,0);		
		if(++freqtmr>=10)	//统计PPM频率
		{
			LcdDrawMiniInt(3,38,PpmInCnt*2,3,0,0,0);
			PpmInCnt=0;
			PpmInCh=0;
			freqtmr=0;
		}
		LcdBw=1;
		LcdDrawText(0,46,(char*)PageOptionOnOff[Model.PpmIn]);
		LcdDrawText(24,46,"\3");
		LcdBw=0;
		LcdDrawStop();
		
		
		montmr+=LCD_MON_TMR;
	}
	
	//返回键处理
	if(KeyTstDown(KEY_EXT) || KeyTstDown(KEY_ENT))
	{
		PageReturn(PV_INIT);
	}
	//+-键处理
	if(KeyTstDown(KEY_R) || KeyTstDown(KEY_L))
	{
		Model.PpmIn=!Model.PpmIn;
	}	
	
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}