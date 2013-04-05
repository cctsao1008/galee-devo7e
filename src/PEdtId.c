#include "Page.h"
#include "Func.h"
#include "Tx.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  编辑模型名称
//
char PageEditIdBuf[MODELIDL+1];
u32 PageEditIdValue;
#define PAGE_EDIT_LEFT	40
#define PAGE_EDIT_TOP	28
#define PAGE_EDIT_WIDTH	MODELIDL*6
u32 PageEditId(u8 event)
{
	static u32 DrawMask,EditIdx;	
	
	if(event==PV_INIT)
	{		
		LcdClear(0);
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		LcdDrawText(3,0,"编辑识别码");
		EditIdx=0;
		
		//把随机ID拷贝进编辑缓冲区
		if(PageEditIdValue==0)
		{			
			u8 i;
			u32 mask=100000;
			for(i=0;i<MODELIDL;i++)
			{
				PageEditIdBuf[i]=TxRndId/mask%10+'0';	
				mask/=10;
			}
			LcdDrawText(70,0,"(自动)");
		}
		else strcpy(PageEditIdBuf,Model.RfIdStr);
		
		LcdDrawHLine(0,128,14,1);
		LcdDrawHLine(0,128,15,1);
		LcdDrawBmpFile(10,24,"res/editnum.bmp");
		LcdDrawStop();
		DrawMask=PD_ALL;
		
		return 1;
	}	
	
	if(DrawMask)
	{
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		if(DrawMask&PD_IDX)
		{			
			LcdDrawRect(PAGE_EDIT_LEFT,PAGE_EDIT_TOP-8,PAGE_EDIT_LEFT+PAGE_EDIT_WIDTH,PAGE_EDIT_TOP-1,LcdBw);			
			if(EditIdx==MODELIDL)	LcdBw=1;
			else					LcdDrawText(PAGE_EDIT_LEFT+EditIdx*6,PAGE_EDIT_TOP-12,"\4");
			LcdDrawText(PAGE_EDIT_LEFT+PAGE_EDIT_WIDTH+10,PAGE_EDIT_TOP+1,"\1自动 ");
			LcdBw=0;
		}
		if(DrawMask&PD_VALUE)
		{
			LcdDrawText(PAGE_EDIT_LEFT,PAGE_EDIT_TOP,PageEditIdBuf);	
			LcdDrawHLine(PAGE_EDIT_LEFT,PAGE_EDIT_LEFT+PAGE_EDIT_WIDTH,PAGE_EDIT_TOP+13,1);			
			LcdDrawHLine(PAGE_EDIT_LEFT,PAGE_EDIT_LEFT+PAGE_EDIT_WIDTH,PAGE_EDIT_TOP+14,1);		
		}
		LcdDrawStop();
		DrawMask=0;
	}

	//焦点移动
	if(KeyTstDown(KEY_UP))
	{
		if(EditIdx>0) 
		{
			EditIdx--;
			DrawMask=PD_IDX;
		}
	}
	if(KeyTstDown(KEY_DW))
	{
		if(EditIdx<MODELIDL)
		{
			EditIdx++;
			DrawMask=PD_IDX;
		}
	}
	
	//字符编辑
	char c=PageEditIdBuf[EditIdx];
	if(KeyTstDown(KEY_L))
	{
		if(c>'0')
		{
			c--;
			DrawMask=PD_VALUE;
		}
	}
	if(KeyTstDown(KEY_R))
	{
		if(c<'9')
		{
			c++;
			DrawMask=PD_VALUE;
		}
	}
	PageEditIdBuf[EditIdx]=c;
	
	//确定键处理
	if(KeyTstDown(KEY_ENT))
	{
		if(EditIdx==MODELIDL)
		{
			PageEditIdValue=0;
		}
		else
		{
			u8 i;
			u32 mask=100000;
			PageEditIdValue=0;
			for(i=0;i<MODELIDL;i++)
			{
				PageEditIdValue+=((u32)(PageEditIdBuf[i]-'0'))*mask;	
				mask/=10;
			}
		}
			
		PageReturn(PV_REDRAW);
	}
		
	//返回键处理
	if(KeyTstDown(KEY_EXT) || event==PV_END)
	{
		PageReturn(PV_REDRAW);
	}
	
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}
