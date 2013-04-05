#include "Page.h"
#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  编辑模型名称
//
char PageEditNameBuf[MODELNL+1];
#define PAGE_EDIT_LEFT	50
#define PAGE_EDIT_TOP	28
#define PAGE_EDIT_WIDTH	MODELNL*6
u32 PageEditName(u8 event)
{
	static u32 DrawMask,EditIdx;	
	
	if(event==PV_INIT)
	{		
		LcdClear(0);
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		LcdDrawText(3,0,"编辑模型名称");
		LcdDrawHLine(0,128,14,1);
		LcdDrawHLine(0,128,15,1);
		LcdDrawBmpFile(14,24,"res/edit.bmp");
		LcdDrawStop();
		DrawMask=PD_ALL;
		EditIdx=0;
		
		//把字符串拷贝进编辑缓冲区，长度不足的用空格填充
		u8 i,se;
		for(i=se=0;i<MODELNL;i++)
		{
			if(Model.Name[i]==0 || se)
			{
				PageEditNameBuf[i]=' ';
				se=1;
			}
			else
			{
				PageEditNameBuf[i]=Model.Name[i];		
			}
		}
		return 1;
	}	
	
	if(DrawMask)
	{
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		if(DrawMask&PD_IDX)
		{			
			LcdDrawRect(PAGE_EDIT_LEFT,PAGE_EDIT_TOP-8,PAGE_EDIT_LEFT+PAGE_EDIT_WIDTH,PAGE_EDIT_TOP-1,LcdBw);			
			LcdDrawText(PAGE_EDIT_LEFT+EditIdx*6,PAGE_EDIT_TOP-12,"\4");	
		}
		if(DrawMask&PD_VALUE)
		{
			LcdDrawText(PAGE_EDIT_LEFT,PAGE_EDIT_TOP,PageEditNameBuf);	
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
		if(EditIdx<MODELNL-1)
		{
			EditIdx++;
			DrawMask=PD_IDX;
		}
	}
	
	//字符编辑
	char c=PageEditNameBuf[EditIdx];
	if(KeyTstDown(KEY_L))
	{
		if(c>' ')
		{
			c--;
			DrawMask=PD_VALUE;
		}
	}
	if(KeyTstDown(KEY_R))
	{
		if(c<'~')
		{
			c++;
			DrawMask=PD_VALUE;
		}
	}
	PageEditNameBuf[EditIdx]=c;
	
	//确定键处理
	if(KeyTstDown(KEY_ENT))
	{
		//拷贝结果并清除尾部空格
		char *p=PageEditNameBuf;
		for(p=PageEditNameBuf; *p ; p++);
		for(p--; *p==' ' && p>PageEditNameBuf ; p--) *p=0;
		strncpy(Model.Name,PageEditNameBuf,MODELNL);		
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
