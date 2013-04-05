#ifndef _FUNC_H_
#define _FUNC_H_

#include "Macros.h"

//端口定义
#define PWR_PORT 		GPIOA
#define PWR_CTL_PIN		GPIO2
#define PWR_SW_PIN		GPIO3

#define KEY_COL_PORT	GPIOB
#define KEY_ROW_PORT	GPIOC
#define KEY_COL_PORT_MASK (GPIO5 | GPIO6 | GPIO7 | GPIO8)
#define	KEY_SW_PORT		GPIOC

#define	VIBRATOR_PORT	GPIOD
#define	VIBRATOR_PIN	GPIO2

#define	BEEP_PORT		GPIOA
#define	BEEP_PIN		GPIO1
#define	BEEP_TIM		TIM2
#define	BEEP_TIM_OC		TIM_OC2
#define BEEP_RCC_APB1ENR_TIMEN   RCC_APB1ENR_TIM2EN

#define PPM_PORT		GPIOA
#define PPM_OUT			GPIO9
#define PPM_IN			GPIO10

#define PPM_H() 		gpio_set(PPM_PORT, PPM_OUT)
#define PPM_L() 		gpio_clear(PPM_PORT, PPM_OUT)

#define LCD_CS_H() 		gpio_set(GPIOB, GPIO0)
#define LCD_CS_L() 		gpio_clear(GPIOB, GPIO0)
#define LCD_CMD() 		gpio_clear(GPIOC,GPIO5)
#define LCD_DAT() 		gpio_set(GPIOC,GPIO5)

//电源控制
void PowerInit(void);
void PowerDown(void);
int  PowerSw(void);
void PowerSleep(void);
void PowerLight(u8 brightness);
void PowerVibrate(u8 onoff);

//键盘扫描
#define KEY_EXT		0x0008
#define	KEY_UP		0x0004
#define KEY_DW		0x0002
#define KEY_ENT		0X0200
#define KEY_R		0X0400
#define KEY_L		0X0800
#define KEY_TRIM_RL	0X0080
#define KEY_TRIM_RR	0X0040
#define KEY_TRIM_AL	0X1000
#define KEY_TRIM_AR	0X2000
#define KEY_TRIM_EU	0X0010
#define KEY_TRIM_ED	0X0020
#define KEY_TRIM_TU	0X4000
#define KEY_TRIM_TD	0X8000
#define	KEY_MASK	0xffff
#define	KEY_MENUALL	(KEY_EXT|KEY_ENT|KEY_UP|KEY_DW|KEY_R|KEY_L)

#define SW_HOLD		0x1
#define SW_FMOD		0x2
#define SW_GEAR0	0x4
#define SW_GEAR1	0x8
#define SW_FLAP0	0x10
#define SW_FLAP1	0x20

extern volatile u16 KeyStat,KeyDown,KeyHold,KeyBuf,KeySwExt;
extern volatile u16 SwStat;
#define SW(X)	(SwStat&(X))
void KeyInit(void);
u16  KeyScanOnce(void);
void KeyScanHandler(void);
u8   KeyTstDown(u16 mask);
u8	 KeyTstStat(u16 mask);
u8	 KeyTstHold(u16 mask);
void KeyFlush(void);
void KeyClearDown(u16 mask);
void KeyStopCnt(void);

//PPM输入
void PpmInInit(void);
void PpmInCtl(u8 onoff);
#define PPMINNUM	8
extern u32 PpmInCnt;
extern u8  PpmInCh;
extern s16 PpmInValue[PPMINNUM];

//系统定时器
extern volatile u32 SysTimerClk;
void SysTimerInit(void);
void SysTimerStart(u16 us, u16 (*callbackfunc)(void));
void SysTimerStop(void);
void SysTimerWatchDogStart(void);
void SysTimerWatchDogRst(void);
void SysTimerSetCallback(void (*cb)(void));
void usleep(u32 x);

//液晶硬件接口
enum DrawDir{
    DRAW_NWSE,
    DRAW_SWNE,
};
void LcdInit();
void LcdClear(u8 bw);
void LcdContrast(u8 contrast);
void LcdSetXy(u8 x,u8 y);
#define LcdWriteLine LcdDat
void LcdDat(u8 c);
void LcdWriteSp(u8 n);
void LcdWriteChar(u8 c);
void LcdWriteStr(u8 X,u8 Y,const char *s);
void LcdWriteHex(u8 X,u8 Y,u32 v,u8 n);

//液晶小字符定义
#define LCD_MN_DOT		10
#define LCD_MN_MINUS	11
#define LCD_MN_MW1		12
#define LCD_MN_MW2		13
#define LCD_MN_MW3		14
#define LCD_MN_SP		15
#define LCD_MN_PLUS		16
#define LCD_MN_SPA		17
#define LCD_MN_COL		18
#define LCD_MN_A		23
#define LCD_MN_CHAR(X)	(X-'A'+LCD_MN_A)

//液晶功能函数
volatile u8 LcdBw;
void LcdDrawLogo(const u8 *logo);
void LcdDrawStart(u16 x0, u16 y0, u16 x1, u16 y1, enum DrawDir _dir);
void LcdDrawPixel(u16 color);
void LcdDrawPixelXY(u16 x, u16 y, u16 color);
void LcdDrawMaskX(u16 x,u16 y,u8 mask,u8 n);
void LcdDrawMaskY(u16 x,u16 y,u8 mask,u8 n);
void LcdDrawStop(void);
void LcdDrawHLine(u16 x1,u16 x2,u16 y,u16 color);
void LcdDrawVLine(u16 x,u16 y1,u16 y2,u16 color);
void LcdDrawLine(u16 x0, u16 y0, u16 x1, u16 y1, u16 color);
void LcdDrawRect(u16 x1,u16 y1,u16 x2,u16 y2,u16 color);
void LcdDrawBmpFile(u16 x, u16 y, const char *filename);
void LcdWriteEngChar(u16 x,u16 y,u8 c);
void LcdDrawInt(u16 x,u16 y,s16 value,u8 n,u8 dot,u8 plus,u8 ar);
void LcdDrawMiniNum(u16 x,u16 y,u8 num);
void LcdDrawMiniEng(u16 x,u16 y,char *eng);
void LcdDrawMiniInt(u16 x,u16 y,s16 value,u8 n,u8 dot,u8 plus,u8 ar);
u16  LcdDrawText(u16 x,u16 y,char *hz);
void LcdDrawIcon(u16 x,u16 y,u16 w,u16 h,const u8 *icon);

//声响函数
extern u8 BeepMusicEnable,BeepVibrator;
void BeepInit(void);
void Beep(u16 frequency, u8 volume);
void BeepHandler(void);
void BeepMusic(const u8 *music);
void BeepShort(u16 frequency, u8 volume,u8 tm);
void BeepStop(void);

//ADC函数
#define ADC_BAT		4
void AdcInit(void);
void AdcHandler(void);
u16 AdcGetValue(u8 i);

//声音列表
extern const u8 MusicStartup[];
extern const u8 MusicTrimZero[];
extern const u8 MusicTrimMax[];
extern const u8 MusicEnter[];
extern const u8 MusicBatLow[];
extern const u8 MusicTimeout[];
extern const u8 MusicStkStop[];
extern const u8 MusicTimeRun[];
#endif//_FUNC_H_