#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  ADC数据结构
//
u16 StickRaw[STK_NUM];
s16 StickCal[STK_NUM];
s16 StickValue[STK_NUM];
s16 StickTrim[STK_NUM];

	//摇杆模式映射
const u8 StickMap[4][4]={
	{0,1,2,3},//日本
	{0,2,1,3},//美国
	{3,1,2,0},//反日本
	{3,2,1,0},//反美国
};
	
void StickHandler(void)
{
	u8 i;
	
	//取得原始数据
	StickRaw[0]=ADC_TRV*2-AdcGetValue(0);
	StickRaw[1]=AdcGetValue(1);
	StickRaw[2]=ADC_TRV*2-AdcGetValue(2);
	StickRaw[3]=AdcGetValue(3);
	
	//校准
	for(i=0;i<STK_NUM;i++)
	{
		s32 v=StickRaw[i]-TxSys.StkCali[i][1];
		
		//死区调整
		if(v<0)
		{
			if(v<-TxSys.StkDa)	v+=TxSys.StkDa;
			else				v=0;
		}  
		else
		{
			if(v>TxSys.StkDa)	v-=TxSys.StkDa;
			else				v=0;
		}  
		
		v*=STK_TRV;		
		StickCal[i]= v/(TxSys.StkCali[i][v>0?2:0]-TxSys.StkDa);
		
		//通道映射
		u8 idx=StickMap[TxSys.StkType][i];
		StickValue[idx]=StickCal[i];
		StickTrim[idx]=Model.Trim[i]*TRIM_STEP;
	}
}