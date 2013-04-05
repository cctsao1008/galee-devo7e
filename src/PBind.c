#include "Page.h"
#include "Func.h"
#include "Tx.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  主界面绘制和按键处理
//
const char PageBindBmpFile[]="res/bind1.bmp";
u32 PageBind(u8 event)
{
	u16 x;
	char iconfile[16];
	static u32 secclk;
	static u8  iconidx;
	static u32 BindCnt;
	
	if(event==PV_INIT)
	{
		LcdClear(0);
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		x=LcdDrawText(3,0,(char*)PageOptionProtocol[Model.Protocol]);
		LcdDrawText(x+2,0,"\6\2正在对码");		
		LcdDrawHLine(0,128,14,1);
		LcdDrawHLine(0,128,15,1);
		LcdDrawBmpFile(52,22,PageBindBmpFile);
		LcdDrawStop();
		
		iconidx=1;
		BindCnt=TxBind();
		
		secclk=SysTimerClk+250;
		
		return 1;
	}	
	
	//动画0.25秒定时器
	if(SysTimerClk>secclk)
	{
		secclk+=250;
		
		iconidx++;
		if(iconidx>4) iconidx=1;
		strcpy(iconfile,PageBindBmpFile);
		iconfile[8]='0'+iconidx;
		LcdDrawBmpFile(52,22,iconfile);
		
		if(TxBindCnt!=TX_BIND_WAIT)
		{
			#define BINDBAR_W	100
			u32 per=BINDBAR_W-TxBindCnt*BINDBAR_W/BindCnt;
			
			LcdSetXy((128-BINDBAR_W)/2,6);
			for(u32 i=0;i<BINDBAR_W;i++)
			{
				LcdWriteLine(i>per?0x81:0xff);
			}
			LcdWriteLine(0xff);
		}
	}
	
	//对码时间完成
	if(TxBindCnt==0)
	{
		PageReturn(PV_REDRAW);
	}

	//退出对码	
	if(KeyTstDown(KEY_EXT))
	{
		TxBindCnt=0;//强制停止对码，并非对所有协议都有效
	}
	
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}