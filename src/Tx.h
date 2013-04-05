#ifndef _TX_H_
#define _TX_H_

#include "Func.h"

#define MP_DSMX		4	//DSMX协议在菜单中的序号

#define	TX_CH_NUM	8
#define	TX_TRV		614	//512*120%

#define TX_BIND_WAIT	((u32)-1)
extern s16 TxChValue[TX_CH_NUM];
extern u8  TxPacket[32];
extern s32 MixerValue[TX_CH_NUM];
extern u32 TxRndId;
extern u32 TxBindCnt;
extern s32 TxValueSum;

enum{
	CH_AIL=0,
	CH_ELE,
	CH_THR,
	CH_RUD,
	CH_GEAR,
	CH_FLAP,
	CH_AUX1,
	CH_AUX2
};

enum TxPower {
    TXPOWER_100uW,
    TXPOWER_300uW,
    TXPOWER_1mW,
    TXPOWER_3mW,
    TXPOWER_10mW,
    TXPOWER_30mW,
    TXPOWER_100mW,
    TXPOWER_150mW,
    TXPOWER_LAST,
};

void TxSpiInit(void);
u32 TxLoad(u8 id);
u32 TxBind(void);
void TxClose(void);
void TxMixer(void);

extern u8 FlyMode,DrMode;

#endif//_TX_H_