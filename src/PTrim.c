#include "Page.h"
#include "Func.h"


u32 PageMainTrimShowCnt;

///////////////////////////////////////////////////////////////////////////////////////
//
//  微调按键处理
//
void PageMainTrimBeep(s8 v)
{
	if(!TxSys.Music)	return;
	
	u32 x;
	if(v<0) v=-v;
	x=v;
	x=x*1000/TRIM_MAX+1000;	
	BeepShort(x, 80,100);
}

void PageMainTrimShow(s8 v)
{
	//如果不是首页，不显示
	if(PageStackIdx)	return;
	
	if(v==TRIM_NUM_HIDE)
	{
		LcdDrawMiniNum(59,59, LCD_MN_SP);
		LcdDrawMiniNum(63,59, LCD_MN_SP);
		LcdDrawMiniNum(67,59, LCD_MN_SP);
	}
	else
	{
		LcdDrawMiniInt(59,59, v,2,0,1,0);
		PageMainTrimShowCnt=TRIM_NUM_TIME;
	}
}

void PageMainTrimProc(u16 keydec,u16 keyinc,u8 trimidx)
{
	s8 v=Model.Trim[trimidx];
	if(KeyTstDown(keyinc))
	{
		if(v<TRIM_MAX) 
		{
			if(++v==0)
			{
				BeepMusic(MusicTrimZero);
				KeyStopCnt();
			}
			else
			{
				PageMainTrimBeep(v);
			}
			PageMainDrawMask|=PMD_TRIM;
			PageMainTrimShow(v);
		}
		else
		{
			BeepMusic(MusicTrimMax);
			KeyStopCnt();
		}
	}
	if(KeyTstDown(keydec))
	{
		if(v>-TRIM_MAX)
		{
			if(--v==0)
			{
				BeepMusic(MusicTrimZero);
				KeyStopCnt();
			}
			else
			{
				PageMainTrimBeep(v);
			}
			PageMainDrawMask|=PMD_TRIM;
			PageMainTrimShow(v);
		}
		else
		{
			BeepMusic(MusicTrimMax);
			KeyStopCnt();
		}
	}
	Model.Trim[trimidx]=v;	
}


///////////////////////////////////////////////////////////////////////////////////////
//
//  主界面绘制和按键处理
//
void TrimProc(void)
{	
	//如果首页不是主页，不进行微调处理（在开机的时候）	
	if(PageStack[0]!=PageMain)	return;
	
	//微调按钮处理
	PageMainTrimProc(KEY_TRIM_AL,KEY_TRIM_AR,0);
	PageMainTrimProc(KEY_TRIM_ED,KEY_TRIM_EU,1);
	PageMainTrimProc(KEY_TRIM_TD,KEY_TRIM_TU,2);
	PageMainTrimProc(KEY_TRIM_RL,KEY_TRIM_RR,3);	
	
	//微调数字自动消隐
	if(PageMainTrimShowCnt==1)
	{
		PageMainTrimShow(TRIM_NUM_HIDE);
		PageMainDrawMask|=PMD_TRIM;
	}
}