#include "Func.h"
#include "Page.h"
#include "Tx.h"

//定时器计数器
TIMERRUN TxTimer[TXTMR_NUM]={
	{0,0,0,1,&Model.Timer[0]},
	{0,0,0,1,&Model.Timer[1]},
	{0,0,0,1,&Model.Timer[2]},
};


///////////////////////////////////////////////////////////////////////////////////////
//
// 定时器处理
//
void TimerRun(u8 idx)
{	
	TIMERRUN *pt=&TxTimer[idx];
	
	//复位判断
	if(pt->Reset)
	{
		pt->Reset=0;
		if(pt->Cfg->Type==TMR_CLK)	pt->Cnt=0;
		else						pt->Cnt=pt->Cfg->Time;
	}
	
	//运行判断
	if(pt->Cfg->Sw==TMRSW_THR)
	{
		pt->Run=(MixerValue[CH_THR]*100/STK_TRV>=pt->Cfg->Thr);
	}
	else if(pt->Cfg->Sw==TMRSW_HOLD)
	{
		pt->Run=SW(SW_HOLD);
	}
	else if(pt->Cfg->Sw==TMRSW_FMOD)
	{
		pt->Run=SW(SW_FMOD);
	}
	else if(pt->Cfg->Sw==TMRSW_HOLDR)
	{
		pt->Run=!SW(SW_HOLD);
	}
	else if(pt->Cfg->Sw==TMRSW_FMODR)
	{
		pt->Run=!SW(SW_FMOD);
	}
	else pt->Run=0;
	
	//禁用和隐藏判断
	if(pt->Cfg->Type<=TMR_HIDE)
	{
		pt->Run=0;
	}
	
	//不运行的不处理
	if(!pt->Run)	return;
	
	//正反向计数操作
	s32 tr=0;
	if(pt->Cfg->Type==TMR_CLK)
	{
		pt->Cnt++;
		pt->Alert=(pt->Cnt>=pt->Cfg->Time);//到时间
		tr=pt->Cfg->Time-pt->Cnt;
	}
	else if(pt->Cfg->Type==TMR_DCK)
	{
		pt->Cnt--;
		pt->Alert=(pt->Cnt<=0);
		tr=pt->Cnt;
	}
	
	//0~30秒提前预警
	if(tr<=30 && tr>=10 && tr%10==0)
	{
		tr=(tr-10)/10*8;
		BeepMusic(&MusicTimeRun[tr]);
	}
	
	//告警操作	
	if(pt->Alert && pt->Cnt%10==0)//每10秒提醒一次
	{
		BeepMusic(MusicTimeout);
		LightStartTmr=SysTimerClk;//背光关闭计数器重置
	}
}

void TimerProc(void)
{
	static u32 timercnt;
	
	//秒分频器
	if(SysTimerClk<timercnt)	return;
	timercnt=SysTimerClk+1000;
	
	TimerRun(0);
	TimerRun(1);	
	TimerRun(2);
}