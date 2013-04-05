#include "Func.h"
#include "Page.h"

TXSYS TxSysBak;
///////////////////////////////////////////////////////////////////////////////////////
//
// 配置读写函数
//

int fputbuf(void *buf,int n,FILE *f)
{
	int i;
	u8 *p=(u8*)buf;
	for(i=0;i<n;i++)
	{
		if(fputc(*p++,f)==-1)		break;
	}
	return i;
}

void LoadCfg(void)
{
	FILE *fcfg=fopen("tx.bin","rb");	
	
	TxSysBak.Mark=0;
	if(fcfg)
	{
		setbuf(fcfg,0);	
		if(fread(&TxSysBak,sizeof(TxSysBak),1,fcfg)!=1)
		{
			TxSysBak.Mark=0;
		}
		fclose(fcfg);
	}	
	
	if(TxSysBak.Mark!=TXCFGMARK)
	{
		memcpy(&TxSys,&TxSysDef,sizeof(TxSys));
		PageAlertModel("使用默认配置...",300);
	}	
	else
	{
		memcpy(&TxSys,&TxSysBak,sizeof(TxSys));
	}
}


void SaveCfg(void)
{	
	if(!memcmp(&TxSysBak,&TxSys,sizeof(TxSys)))	return;
	
	PageAlertModel("保存遥控器配置...",300);
	
	FILE *fcfg=fopen("tx.bin","wb");	
	if(fcfg)
	{
		if(fputbuf(&TxSys,sizeof(TxSys),fcfg)!=sizeof(TxSys))
		{
			PageAlertModel("无法写入TX.BIN!",0);
		}
		fclose(fcfg);
	}
	else
	{
		PageAlertModel("无法打开TX.BIN!",0);
	}
	
	//保存备份以备比较
	memcpy(&TxSysBak,&TxSys,sizeof(TxSys));
}