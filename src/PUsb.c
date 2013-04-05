#include "Page.h"
#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  USB模式
//
u32 PageUsb(u8 event)
{
	if(event==PV_INIT)
	{
		USB_Enable(1);

		LcdClear(0);
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		LcdDrawText(3,0,"遥控器已连接为U盘");
		LcdDrawHLine(0,128,14,1);
		LcdDrawHLine(0,128,15,1);
		LcdDrawBmpFile(8,28,"res/tf.bmp");
		LcdDrawText(40,32,"按EXT键退出");
		LcdDrawStop();
	
		PowerLight(20);//读写U盘时候背光灯会闪烁，设置固定值
		
		return 1;
	}	
	
		
	//返回键处理
	if(KeyTstDown(KEY_EXT))
	{
		USB_Disable(1);
		FS_Mount();
		
		PowerLight(TxSys.Light*8);
		
		PageAlert("U盘已断开...",1000,PV_END);
	}
	
	if(event==PV_END)
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}