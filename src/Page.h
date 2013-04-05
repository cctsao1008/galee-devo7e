#ifndef _PAGE_H_
#define _PAGE_H_

#include "Macros.h"

//页面处理过程定义
#define PV_RUN		0
#define PV_INIT		1
#define PV_END		2
#define PV_REDRAW	3
#define PV_TIMER	4
#define PV_USER		0x80

typedef u32 (*PAGEPROC)(u8 event);

//菜单数据结构定义
typedef const char * CSTR;
typedef struct{
	char *		Title;
	void *		SubItem;	//子功能|子菜单
	CSTR *		Values;		//值列表	 
	s16			Min,Max;	//菜单的最大最小值
	s16			*pValue;	//菜单对应的内存位置
	void *		Data;	//参数
	char *		Text;		//显示字符串
}MENUITEM,*PMENUITEM;
typedef const MENUITEM CMENUITEM;
#define SUBMENU(X)	((CPMENUITEM)((X)->SubItem))
#define SUBPROC(X)	((PAGEPROC)((X)->SubItem))

//菜单状态定义
typedef struct{
	u8 Init;
	u8 DrawMask,iFocus,iStart,Total;
	u8 Param;
}MENUSTAT;

#define PD_TITLE	1
#define PD_LIST		2
#define PD_IDX		4
#define PD_VALUE	8
#define PD_ALL		0xff

//页面控制函数
#define PAGELEVEL	5			//最多允许进入几层UI
extern PAGEPROC PageStack[PAGELEVEL];
extern u8 PageStackIdx;
void PageEnter(PAGEPROC page,u8 msg);
void PageGoto(PAGEPROC page,u8 msg);
void PageReturn(u8 msg);
void PageSet(PAGEPROC page,u8 msg);
void PageAlertModel(CSTR text,u32 delay);
void PageAlert(CSTR text,u32 delay,u8 msg);
void PageMenuProc(CMENUITEM *pmi,MENUSTAT *pms);

//主界面绘制控制
#define PMD_BACK	1
#define PMD_ICON	2
#define PMD_NAME	4
#define PMD_BAT		8
#define PMD_TRIM	0X10
#define PMD_INFO	0x20
#define PMD_ALL		0xffffffff
extern u32 PageMainDrawMask;
u32 PageMain(u8 init);
void BatteryHandler(void);

//微调控制
#define TRIM_NUM_HIDE 99
extern u32 PageMainTrimShowCnt;
void TrimProc(void);

//菜单系列
u32 PageMenuMain(u8 event);
u32 PageMenuSys(u8 event);
u32 PageMenuMod(u8 event);
u32 PageMenuRf(u8 event);
u32 PageStkCal(u8 event);
u32 PageMonitor(u8 event);
u32 PagePpmIn(u8 event);
u32 PageUsb(u8 event);
u32 PageBind(u8 event);
u32 PageIconSel(u8 event);
u32 PageEditName(u8 event);
u32 PageEditId(u8 event);
u32 PageModelSel(u8 event);
u32 PageHeliSet(u8 event);
u32 PageAeroSet(u8 event);
u32 PageMenuChMap(u8 event);
u32 PageTimerSet1(u8 event);
u32 PageTimerSet2(u8 event);
u32 PageTimerSet3(u8 event);

//模型参数菜单系列
u32 PageNorRevSet(u8 event);
u32 PageEpaSet(u8 event);
u32 PageNeuSet(u8 event);
u32 PageDeltaSet(u8 event);
u32 PageVtailSet(u8 event);
u32 PageFlpAilSet(u8 event);
u32 PageAuxChSet(u8 event);
u32 PageGyroSet(u8 event);
u32 PageSwashSet(u8 event);
u32 PageDelaySet(u8 event);
u32 PageSwDef(u8 event);
u32 PageMixerSet(u8 event);
u32 PageThrMixSet(u8 event);
u32 PageYawMixSet(u8 event);

//曲线设置函数
extern CSTR *PageCurveTitle;
extern u8 PageCurveCnt;
extern s16 *PageCurveValue;
u32 PageCurveSet(u8 event);

//模型参数存取函数
void ModelLoad(u16 id,u8 txopen);
void ModelSave(u16 id);
void ModelFormat(void);

//关于
u32 PageAbout(u8 init);

extern CSTR PageOptionNorRev[];
extern CSTR PageOptionOnOff[];
extern CSTR PageOptionStkType[];
extern CSTR PageOptionBatType[];
extern CSTR PageOptionOkExe[];
extern CSTR PageOptionModType[];
extern CSTR PageOptionSwDef[];
extern CSTR PageOptionProtocol[];
extern CSTR PageOptionRfPwr[];
extern CSTR PageOptionChSelA[];
extern CSTR PageOptionChSelH[];
extern CSTR PageOptionAuxSrc[];
extern CSTR PageOptionTmrType[];
extern CSTR PageOptionTmrSw[];
extern CSTR PageStkCurveTitle[];
extern CSTR PageThrCurveTitle[];
extern CSTR PagePitCurveTitle[];
extern CSTR PageOptionSwash[];
extern CSTR PageOptionLightOff[];
extern CSTR PageOptionAutoOff[];
extern CSTR PageOptionFreeWarn[];
#endif//_PAGE_H_