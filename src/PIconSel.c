#include "Page.h"
#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  主界面绘制和按键处理
//
char PageIconFiles[ICON_NUM][13];
u16	PageIconFileCnt,PageIconFileIdx,PageIconFileStart;;
u32 PageIconSel(u8 event)
{	
	static u32 DrawMask;
	
	if(event==PV_INIT)
	{
		LcdClear(0);
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		LcdDrawText(3,0,"选择模型图标");		
		LcdDrawHLine(0,128,14,1);
		LcdDrawHLine(0,128,15,1);
		LcdDrawStop();
		
		DrawMask=PD_ALL;
		
		PageIconFileCnt=0;
		PageIconFileIdx=0;
		PageIconFileStart=0;
		
		//枚举所有模型图标
    	if(FS_OpenDir("icon"))
    	{
        	int type;
        	while((type = FS_ReadDir(PageIconFiles[PageIconFileCnt])) != 0 && PageIconFileCnt<ICON_NUM)
        	{
            	if (type == 1)
            	{
            		if(strncasecmp(PageIconFiles[PageIconFileCnt],Model.Icon,strlen(Model.Icon))==0)//寻找当前图标文件
            		{
            			PageIconFileIdx=PageIconFileCnt;
            		}
            		PageIconFileCnt++;                
            	}
            }
        	FS_CloseDir();
        }
		
		//判断是否有图标文件
		if(PageIconFileCnt==0)
		{
			PageAlert("没有图标文件...",1000,PV_END);
		}
		
		return 1;
	}	
	
	//绘制界面
	if(DrawMask)
	{		
		//起始位置整理
		if(PageIconFileIdx<PageIconFileStart) 					PageIconFileStart=PageIconFileIdx;
		if(PageIconFileIdx>PageIconFileStart+LCD_MENULINES-1)	PageIconFileStart=PageIconFileIdx-LCD_MENULINES+1;	
	
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		//菜单项索引号
		if(DrawMask&PD_IDX)
		{
			if(PageIconFileCnt>9)
			{
				LcdDrawMiniInt(110,4,PageIconFileIdx+1,2,0,0xff,1);
				LcdDrawMiniNum(110,4,LCD_MN_SPA);
				LcdDrawMiniInt(114,4,PageIconFileCnt,2,0,0xff,0);
			}
			else
			{
				LcdDrawMiniInt(120,4,PageIconFileIdx+1,2,0,0xff,1);
				LcdDrawMiniNum(120,4,LCD_MN_SPA);
				LcdDrawMiniInt(124,4,PageIconFileCnt,1,0,0xff,0);
			}
		}
		
		//绘制文件列表和模型图标
		if(DrawMask&PD_LIST)
		{
			char file[20];
			
			//画图标
			strcpy(file,"icon/");
			strcat(file,PageIconFiles[PageIconFileIdx]);
			LcdDrawBmpFile(74,22,file);
			
			//显示列表
			int i,y,idx;
			for(i=0,y=16;i<LCD_MENULINES;i++,y+=16)
			{
				idx=PageIconFileStart+i;
				strcpy(file,PageIconFiles[idx]);
				file[strlen(file)-4]=0;
				LcdBw=(idx==PageIconFileIdx);				
				LcdDrawRect(2,y,70,y+15,LcdBw);		//绘制选中框
				LcdDrawMiniInt(2,y+4,idx+1,2,0,0xff,0);	//绘制序号
				LcdDrawMiniNum(10,y+4,LCD_MN_DOT);		//绘制分隔点
				LcdDrawText(14,y,file);	//绘制名称
				LcdBw=0;
			}
		}
		LcdDrawStop();
		DrawMask=0;
	}
	
	//上下按键处理
	if(KeyTstDown(KEY_UP))
	{
		if(PageIconFileIdx>0)	PageIconFileIdx--;
		else		 			PageIconFileIdx=PageIconFileCnt-1;
		DrawMask=PD_LIST|PD_IDX;
	}
	if(KeyTstDown(KEY_DW))
	{
		if(PageIconFileIdx<PageIconFileCnt-1)	PageIconFileIdx++;
		else		 							PageIconFileIdx=0;
		DrawMask=PD_LIST|PD_IDX;
	}
	
	//选中位图
	if(KeyTstDown(KEY_ENT))
	{
		PageIconFiles[PageIconFileIdx][strlen(PageIconFiles[PageIconFileIdx])-4]=0;
		strcpy(Model.Icon,PageIconFiles[PageIconFileIdx]);
		PageReturn(PV_REDRAW);
	}
	
	//退出消息
	if(KeyTstDown(KEY_EXT) || event==PV_END)	PageReturn(PV_REDRAW);
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}