#include "Page.h"
#include "Func.h"
#include "Tx.h"


//旧版标识
#define CFGMARKOLD 0x454c4147

//旧版配置文件
typedef struct{
	u32 	Mark;
	char 	Name[MODELNL+1];
	char 	Icon[8];
	s16		Type;
	s8		Trim[4];
	s16		Protocol;
	u32		RfId;
	char	RfIdStr[MODELIDL+1];
	s16		RfPwr;
	s16		RfChNum;
	s16		ChMap[12];
	TIMER	Timer[TXTMR_NUM];
	s16		Rev[8];
	s16		Epa[8][2];
	s16		Neu[8];
	s16		ThrHold;
	DELTAMIX Delta;
	VTAILMIX Vtail;
	s16		StkCurve[3][2][CURVE_DS];
	s16		ThrCurve[2][CURVE_DS];
	s16		PitCurve[3][CURVE_DS];
	s16		Gyro[2];
	s16		SwashType;
	s16		Swash[3];
	s16		Aux[6];
	s16		Delay[8];
	s16		SwDef[8];
	u8		PpmIn;
	u32		reverse;
	FLAPAILMIX FlpAil;
	s16		Gear[3];
	s16		Flap[3];
	s16		ThrMix[3];
	YAWMIX YawMix;
}MODELOLD;

///////////////////////////////////////////////////////////////////////////////////////
//
//  模型文件保存加载
//

MODEL	Model,ModelBak;
const MODEL ModelDef={
	CFGMARK,	//u32 	Mark;
	"PLANE",	//char 	Name[MODELNL+1];
	"MODEL",	//char 	Icon[8];
	0,			//s16		Type;
	{0,0,0,0},	//s8		Trim[4];
	0,			//s16		Protocol;
	0,			//u32		RfId;
	"000000",	//char		RfIdStr[8];
	4,			//s16		RfPwr;=10mW
	8,			//s16		RfChNum
	{0,1,2,3,4,5,6,7,8,9,10,11},	//ChMap	
	{			//TIMER Timer[3]
		{0,0,-90,0},
		{TMR_HIDE,0,-90,0}, //缺省不显示定时器2
		{TMR_HIDE,0,-90,0},	//缺省不显示定时器3
	},
	{0,0,0,0,0,0,0,0,0,0,0,0},	//s16	Rev[12];
	{					//s16	Epa[12][2];
		{100,+100},	{100,+100},	{100,+100},	{100,+100},	
		{100,+100},	{100,+100},	{100,+100},	{100,+100},
		{100,+100},	{100,+100},	{100,+100},	{100,+100},
	},
	{0,0,0,0,0,0,0,0,0,0,0,0},	//s16	Neu[12];
	{0,0,0,0,0,0,0,0,0,0,0,0},	//s16 Delay[12];
	-10,				//s16	ThrHold;
	{0,100,-100,100,100},	//DELTAMIX Delta;
	{0,100,-100,100,100},	//VTAILMIX Vtail;
	{0,100,-100,100,100},	//FLAPAIL;
	{0,0,0,0},				//YAWMIX
	{0,0,0},				//ThrMix
	{						//StkCurve
		{						
			{0,125,250,375,500,625,750,875,1000,100,0,0},
			{0,125,250,375,500,625,750,875,1000,100,0,0},
			{0,125,250,375,500,625,750,875,1000,100,0,0},
		},                                     
		{                                      
			{0,125,250,375,500,625,750,875,1000,100,0,0},
			{0,125,250,375,500,625,750,875,1000,100,0,0},
			{0,125,250,375,500,625,750,875,1000,100,0,0},
		},                                     
		{                                      
			{0,125,250,375,500,625,750,875,1000,100,0,0},
			{0,125,250,375,500,625,750,875,1000,100,0,0},
			{0,125,250,375,500,625,750,875,1000,100,0,0},
		},
	},
	{		//ThrCurve
		{0,125,250,375,500,625,750,875,1000,100,0,0},
		{0,125,250,375,500,625,750,875,1000,100,0,0},
	},                                     
	{		//PitCurve                     
		{0,125,250,375,500,625,750,875,1000,100,0,0},
		{0,125,250,375,500,625,750,875,1000,100,0,0},
		{0,125,250,375,500,625,750,875,1000,100,0,0},
	},
	{50,50},			//Gyro
	0,					//SwashType
	{100,100,100},		//Swash
	{-100,0,100},		//AUXVALUE Gear;
	{-100,0,100},		//AUXVALUE Flap;
	{					//AUXVALUE Aux[6];
		{-100,0,100},
		{-100,0,100},
		{-100,0,100},
		{-100,0,100},
		{-100,0,100},
		{-100,0,100}
	},
	{1,2,0,1,2,0,0,0},	//s16		SwDef[8];
	0,					//PpmIn
};

char ModelFileName[16]="model/00.bin";
void ModelLoad(u16 id,u8 txopen)
{
	FILE *fcfg;
		
	ModelFileName[6]=id/10+'0';
	ModelFileName[7]=id%10+'0';
		
	fcfg=fopen(ModelFileName,"rb");
	
	ModelBak.Mark=0;
	if(fcfg)
	{
		setbuf(fcfg,0);	
		if(fread(&ModelBak,sizeof(ModelBak),1,fcfg)!=1)
		{
			ModelBak.Mark=0;
		}
		fclose(fcfg);
	}	
	
	//检测配置文件版本和有效性
	if(ModelBak.Mark==CFGMARKOLD)
	{
		//初始化升级
		MODELOLD mo;
		memcpy(&mo,&ModelBak,sizeof(mo));
		memcpy(&Model,&ModelDef,sizeof(Model));
		
		//新旧数据文件转换
		strcpy(Model.Name,mo.Name);
		strcpy(Model.Icon,mo.Icon);
		Model.Type=mo.Type;
		Model.Trim[0]=mo.Trim[0];
		Model.Trim[1]=mo.Trim[1];
		Model.Trim[2]=mo.Trim[2];
		Model.Trim[3]=mo.Trim[3];
		Model.Protocol=mo.Protocol;
		Model.RfId=mo.RfId;
		strcpy(Model.RfIdStr,mo.RfIdStr);
		Model.RfPwr=mo.RfPwr;
		Model.RfChNum=mo.RfChNum;		
		memcpy(&Model.ChMap,&mo.ChMap,sizeof(mo.ChMap));
		memcpy(&Model.Timer,&mo.Timer,sizeof(mo.Timer));
		memcpy(&Model.Rev,&mo.Rev,sizeof(mo.Rev));
		memcpy(&Model.Epa,&mo.Epa,sizeof(mo.Epa));
		memcpy(&Model.Neu,&mo.Neu,sizeof(mo.Neu));
		Model.ThrHold=mo.ThrHold;
		memcpy(&Model.Delta,&mo.Delta,sizeof(mo.Delta));
		memcpy(&Model.Vtail,&mo.Vtail,sizeof(mo.Vtail));
		memcpy(&Model.StkCurve[0],&mo.StkCurve[0],sizeof(mo.StkCurve[0]));
		memcpy(&Model.StkCurve[1],&mo.StkCurve[1],sizeof(mo.StkCurve[1]));
		memcpy(&Model.StkCurve[2],&mo.StkCurve[2],sizeof(mo.StkCurve[2]));
		memcpy(&Model.ThrCurve,&mo.ThrCurve,sizeof(mo.ThrCurve));
		memcpy(&Model.PitCurve,&mo.PitCurve,sizeof(mo.PitCurve));
		Model.Gyro[0]=mo.Gyro[0];
		Model.Gyro[1]=mo.Gyro[1];
		Model.SwashType=mo.SwashType;
		Model.Swash[0]=mo.Swash[0];
		Model.Swash[1]=mo.Swash[1];
		Model.Swash[2]=mo.Swash[2];
		Model.Aux[0][0]=mo.Aux[0];
		Model.Aux[0][1]=mo.Aux[1];
		Model.Aux[0][2]=mo.Aux[2];
		Model.Aux[1][0]=mo.Aux[3];
		Model.Aux[1][1]=mo.Aux[4];
		Model.Aux[1][2]=mo.Aux[5];
		memcpy(&Model.Delay,&mo.Delay,sizeof(mo.Delay));
		memcpy(&Model.SwDef,&mo.SwDef,sizeof(mo.SwDef));
		Model.PpmIn=mo.PpmIn;
		Model.Gear[0]=mo.Gear[0];
		Model.Gear[1]=mo.Gear[1];
		Model.Gear[2]=mo.Gear[2];
		Model.Flap[0]=mo.Flap[0];
		Model.Flap[1]=mo.Flap[1];
		Model.Flap[2]=mo.Flap[2];
		memcpy(&Model.FlpAil,&mo.FlpAil,sizeof(mo.FlpAil));
		memcpy(&Model.ThrMix,&mo.ThrMix,sizeof(mo.ThrMix));
		memcpy(&Model.YawMix,&mo.YawMix,sizeof(mo.YawMix));
		ModelSave(id);
		PageAlertModel("配置文件已升级!",0);
	}
	else if(ModelBak.Mark!=CFGMARK)
	{
		memcpy(&Model,&ModelDef,sizeof(Model));
		PageAlertModel("使用默认模型配置...",300);
	}	
	else
	{
		memcpy(&Model,&ModelBak,sizeof(Model));
		//PageAlertModel(Model.Name,1000);
	}
	
	//备份供比较
	memcpy(&ModelBak,&Model,sizeof(Model));
	
	//定时器复位
	TxTimer[0].Reset=1;		
	TxTimer[1].Reset=1;		
	TxTimer[2].Reset=1;
	
	if(txopen)
	{
		TxClose();
		//加载发射模块
		TxLoad(Model.Protocol);
	}
}

void ModelSave(u16 id)
{
	FILE *fcfg;
	u8 saveok=0;

	if(!memcmp(&ModelBak,&Model,sizeof(Model)))	return;

	PageAlertModel("保存模型参数...",300);
				
	ModelFileName[6]=id/10+'0';
	ModelFileName[7]=id%10+'0';
		
	fcfg=fopen(ModelFileName,"wb");
	
	if(fcfg)
	{
		if(fputbuf(&Model,sizeof(Model),fcfg)==sizeof(Model))
		{
			saveok=1;
		}
		fclose(fcfg);
	}
	
	if(!saveok)
	{		
		PageAlertModel("模型配置保存失败!",1000);
	}
	
	//备份供比较
	memcpy(&ModelBak,&Model,sizeof(Model));
}

//制作模型初始配置文件
void ModelFormat(void)
{
	FILE *fcfg;
	u16 i;
	
	memcpy(&Model,&ModelDef,sizeof(Model));
	memcpy(&ModelBak,&ModelDef,sizeof(Model));
	for(i=1;i<=MODCFG_NUM;i++)
	{
		char modelfile[13];
		ModelBak.Name[5]=i/10+'0';
		ModelBak.Name[6]=i%10+'0';
		ModelBak.Name[7]=0;
		
		strcpy(modelfile,ModelFileName);
		modelfile[6]=i/10+'0';
		modelfile[7]=i%10+'0';
		
		PageAlertModel(ModelBak.Name,50);
		fcfg=fopen(modelfile,"wb");
		if(fcfg)
		{
			fputbuf(&ModelBak,sizeof(ModelBak),fcfg);
			fclose(fcfg);
		}
	}
	
	Model.Name[5]=TxSys.ModelNo/10+'0';
	Model.Name[6]=TxSys.ModelNo%10+'0';
	Model.Name[7]=0;
	memcpy(&ModelBak,&Model,sizeof(Model));
}
///////////////////////////////////////////////////////////////////////////////////////
//
//  模型选择界面过程
//
char PageModFiles[MODCFG_NUM][MODELNL+6];
u16	PageModFileCnt,PageModFileIdx,PageModFileStart;;
u32 PageModelSel(u8 event)
{	
	static u32 DrawMask;
	
	if(event==PV_INIT)
	{
		LcdClear(0);
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		LcdDrawText(3,0,"选择模型");		
		LcdDrawHLine(0,128,14,1);
		LcdDrawHLine(0,128,15,1);
		LcdDrawStop();
		
		DrawMask=PD_ALL;
		
		PageModFileCnt=0;
		PageModFileIdx=0;
		PageModFileStart=0;
		
		//枚举所有模型图标
    	if(FS_OpenDir("model"))
    	{
			char modfile[13];
        	int type;
        	while((type = FS_ReadDir(modfile)) != 0 && PageModFileCnt<MODCFG_NUM)
        	{
        		BeepHandler();//此循环耗时太长，需要执行一些必要过程
            	if (type == 1)
            	{
            		//打开配置文件读取模型名称
            		char filename[32];
            		strcpy(filename,"model/");
            		strcat(filename,modfile);
            		FILE *fcfg=fopen(filename,"rb");
            		if(fcfg)
            		{
            			MODEL model;
            			fread(&model,sizeof(model),1,fcfg);
            			if(model.Mark==CFGMARK || model.Mark==CFGMARKOLD)
            			{            	
		            		u16 idx=(modfile[0]-'0')*10+(modfile[1]-'0');       
		            		
            				PageModFiles[PageModFileCnt][0]=modfile[0];
            				PageModFiles[PageModFileCnt][1]=modfile[1];
            				PageModFiles[PageModFileCnt][2]=0xba;//号
            				PageModFiles[PageModFileCnt][3]=0xc5;//号
            				PageModFiles[PageModFileCnt][4]='.';
            				if(idx==TxSys.ModelNo)	strcpy(&PageModFiles[PageModFileCnt][5],Model.Name);
            				else					strcpy(&PageModFiles[PageModFileCnt][5],model.Name);
            					    		
		            		if(idx==TxSys.ModelNo)//寻找当前配置文件
		            		{
		            			PageModFileIdx=PageModFileCnt;
		            		}
		            		PageModFileCnt++;    
		            	}
		            	fclose(fcfg);
            		}            
            	}
            }
        	FS_CloseDir();
        }
		
		//判断是否有配置文件
		if(PageModFileCnt==0)
		{
			PageAlert("没有配置文件...",1000,PV_END);
		}
		
		return 1;
	}	
	
	//绘制界面
	if(DrawMask)
	{		
		//起始位置整理
		if(PageModFileIdx<PageModFileStart) 				PageModFileStart=PageModFileIdx;
		if(PageModFileIdx>PageModFileStart+LCD_MENULINES-1)	PageModFileStart=PageModFileIdx-LCD_MENULINES+1;	
	
		LcdDrawStart(0, 0,LCD_W-1, LCD_H-1, DRAW_NWSE);  
		/*
		//菜单项索引号
		if(DrawMask&PD_IDX)
		{
			if(PageModFileCnt>9)
			{
				LcdDrawMiniInt(110,4,PageModFileIdx+1,2,0,0xff,1);
				LcdDrawMiniNum(110,4,LCD_MN_SPA);
				LcdDrawMiniInt(114,4,PageModFileCnt,2,0,0xff,0);
			}
			else
			{
				LcdDrawMiniInt(120,4,PageModFileIdx+1,2,0,0xff,1);
				LcdDrawMiniNum(120,4,LCD_MN_SPA);
				LcdDrawMiniInt(124,4,PageModFileCnt,1,0,0xff,0);
			}
		}
		*/
		
		//绘制文件列表和模型图标
		if(DrawMask&PD_LIST)
		{			
			//显示列表
			int i,y,idx;
			for(i=0,y=16;i<LCD_MENULINES;i++,y+=16)
			{
				idx=PageModFileStart+i;
				LcdBw=(idx==PageModFileIdx);				
				LcdDrawRect(2,y,125,y+15,LcdBw);		//绘制选中框
				LcdDrawText(3,y,PageModFiles[idx]);	//绘制名称
				LcdBw=0;
			}
		}
		LcdDrawStop();
		DrawMask=0;
	}
	
	//上下按键处理
	if(KeyTstDown(KEY_UP))
	{
		if(PageModFileIdx>0)	PageModFileIdx--;
		else		 			PageModFileIdx=PageModFileCnt-1;
		DrawMask=PD_LIST|PD_IDX;
	}
	if(KeyTstDown(KEY_DW))
	{
		if(PageModFileIdx<PageModFileCnt-1)	PageModFileIdx++;
		else		 							PageModFileIdx=0;
		DrawMask=PD_LIST|PD_IDX;
	}
	
	//选中位图
	if(KeyTstDown(KEY_ENT))
	{
		//取得选中的模型号
		PageModFileIdx= 10*(PageModFiles[PageModFileIdx][0]-'0')+PageModFiles[PageModFileIdx][1]-'0';
		
		//保存现有模型的配置
		ModelSave(TxSys.ModelNo);
		
		//加载新遥控器配置
		ModelLoad(TxSys.ModelNo=PageModFileIdx,1);
		
		PageAlert("模型已切换!",1000,PV_END);
	}
	
	//退出消息
	if(KeyTstDown(KEY_EXT) || event==PV_END)	PageReturn(PV_INIT);//这里不要用REDRAW，要用INIT，让模型菜单初始化
	KeyClearDown(KEY_MENUALL);	
	
	return 0;
}