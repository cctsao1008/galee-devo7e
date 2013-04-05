#include "Tx.h"
#include "RF_A7105.h"

#define HUBSAN_ID 0xdb042679

u8 TxHbsChIdx;
const u8 TxHbsAllowedCh[] = {0x14, 0x1e, 0x28, 0x32, 0x3c, 0x46, 0x50, 0x5a, 0x64, 0x6e, 0x78, 0x82};
u32 TxHbsSessionId;
u8 TxHbsState;
enum {
    BIND_1,
    BIND_2,
    BIND_3,
    BIND_4,
    BIND_5,
    BIND_6,
    BIND_7,
    BIND_8,
    DATA_1,
    DATA_2,
    DATA_3,
    DATA_4,
    DATA_5,
};
#define WAIT_WRITE 0x80

static int TxHbsA7105Init()
{
    A7105_WriteID(0x55201041);
    A7105_WriteReg(A7105_01_MODE_CONTROL, 0x63);
    A7105_WriteReg(A7105_03_FIFOI, 0x0f);
    A7105_WriteReg(A7105_0D_CLOCK, 0x05);
    A7105_WriteReg(A7105_0E_DATA_RATE, 0x04);
    A7105_WriteReg(A7105_15_TX_II, 0x2b);
    A7105_WriteReg(A7105_18_RX, 0x62);
    A7105_WriteReg(A7105_19_RX_GAIN_I, 0x80);
    A7105_WriteReg(A7105_1C_RX_GAIN_IV, 0x0A);
    A7105_WriteReg(A7105_1F_CODE_I, 0x07);
    A7105_WriteReg(A7105_20_CODE_II, 0x17);
    A7105_WriteReg(A7105_29_RX_DEM_TEST_I, 0x47);

    A7105_Strobe(A7105_STANDBY);

	//用500ms校准IF
	A7105CaliIf(500);

    //VCO Current Calibration
    A7105_WriteReg(0x24, 0x13); //Recomended calibration from A7105 Datasheet

    //VCO Bank Calibration
    A7105_WriteReg(0x26, 0x3b); //Recomended limits from A7105 Datasheet

	//用500ms校准0频点
	if(!A7105_CaliCh(0x00,500))	return 0;

	//用500ms校准A0频点
	if(!A7105_CaliCh(0xa0,500))	return 0;

    //Reset VCO Band calibration
    A7105_WriteReg(0x25, 0x08);

    A7105_SetPower(Model.RfPwr);

    A7105_Strobe(A7105_STANDBY);
    return 1;
}

static void TxHbsUpdateCrc()
{
    int sum = 0;
    for(int i = 0; i < 15; i++)
        sum += TxPacket[i];
    TxPacket[15] = (256 - (sum % 256)) & 0xff;
}

static void TxHbsBuildBindPkt(u8 TxHbsState)
{
    TxPacket[0] = TxHbsState;
    TxPacket[1] = TxHbsChIdx;
    TxPacket[2] = (TxHbsSessionId >> 24) & 0xff;
    TxPacket[3] = (TxHbsSessionId >> 16) & 0xff;
    TxPacket[4] = (TxHbsSessionId >>  8) & 0xff;
    TxPacket[5] = (TxHbsSessionId >>  0) & 0xff;
    TxPacket[6] = 0x08;
    TxPacket[7] = 0xe4; //???
    TxPacket[8] = 0xea;
    TxPacket[9] = 0x9e;
    TxPacket[10] = 0x50;
    TxPacket[11] = (HUBSAN_ID >> 24) & 0xff;
    TxPacket[12] = (HUBSAN_ID >> 16) & 0xff;
    TxPacket[13] = (HUBSAN_ID >>  8) & 0xff;
    TxPacket[14] = (HUBSAN_ID >>  0) & 0xff;
    TxHbsUpdateCrc();
}

static s16 TxHbsGetCh(u8 ch, s32 scale, s32 center, s32 range)
{
    s32 value = (s32)TxChValue[ch] * scale / STK_TRV + center;
    if (value < center - range)        value = center - range;
    if (value >= center + range)       value = center + range -1;
    return value;
}

static void TxHbsBuildPkt()
{
    memset(TxPacket, 0, 16);
    //20 00 00 00 80 00 7d 00 84 02 64 db 04 26 79 7b
    TxPacket[0] = 0x20;
    TxPacket[2] = TxHbsGetCh(2, 0x80, 0x80, 0x80);
    TxPacket[4] = TxHbsGetCh(3, 0x80, 0x80, 0x80); 			//方向
    TxPacket[6] = 0xff - TxHbsGetCh(1, 0x80, 0x80, 0x80);	//升降
    TxPacket[8] = 0xff - TxHbsGetCh(0, 0x80, 0x80, 0x80);	//副翼
    TxPacket[9] = 0x02;
    TxPacket[10] = 0x64;
    TxPacket[11] = (HUBSAN_ID >> 24) & 0xff;
    TxPacket[12] = (HUBSAN_ID >> 16) & 0xff;
    TxPacket[13] = (HUBSAN_ID >>  8) & 0xff;
    TxPacket[14] = (HUBSAN_ID >>  0) & 0xff;
    TxHbsUpdateCrc();
}

static u16 TxHbsCallback()
{
    int i;
    switch(TxHbsState) {
    case BIND_1:
    	TxBindCnt=TX_BIND_WAIT;
    case BIND_3:
    case BIND_5:
    case BIND_7:
        TxHbsBuildBindPkt(TxHbsState == BIND_7 ? 9 : (TxHbsState == BIND_5 ? 1 : TxHbsState + 1 - BIND_1));
        A7105_Strobe(A7105_STANDBY);
        A7105_WriteData(TxPacket, 16, TxHbsChIdx);
        TxHbsState |= WAIT_WRITE;
        return 3000;
    case BIND_1 | WAIT_WRITE:
    case BIND_3 | WAIT_WRITE:
    case BIND_5 | WAIT_WRITE:
    case BIND_7 | WAIT_WRITE:
        //wait for completion
        for(i = 0; i< 20; i++) {
           if(! (A7105_ReadReg(A7105_00_MODE) & 0x01))
               break;
        }
        //if (i == 20)
        //    printf("Failed to complete write\n");
        A7105_Strobe(A7105_RX);
        TxHbsState &= ~WAIT_WRITE;
        TxHbsState++;
        return 4500; //7.5msec elapsed since last write
    case BIND_2:
    case BIND_4:
    case BIND_6:
        if(A7105_ReadReg(A7105_00_MODE) & 0x01) {
            TxHbsState = BIND_1;
            return 4500; //No signal, restart binding procedure.  12msec elapsed since last write
        }
        A7105_ReadData(TxPacket, 16);
        TxHbsState++;
        if (TxHbsState == BIND_5)
            A7105_WriteID((TxPacket[2] << 24) | (TxPacket[3] << 16) | (TxPacket[4] << 8) | TxPacket[5]);

        return 500;  //8msec elapsed time since last write;
    case BIND_8:
        if(A7105_ReadReg(A7105_00_MODE) & 0x01) {
            TxHbsState = BIND_7;
            return 15000; //22.5msec elapsed since last write
        }
        A7105_ReadData(TxPacket, 16);
        if(TxPacket[1] == 9) {
            TxHbsState = DATA_1;
            A7105_WriteReg(A7105_1F_CODE_I, 0x0F);
            TxBindCnt=0;;
            return 28000; //35.5msec elapsed since last write
        } else {
            TxHbsState = BIND_7;
            return 15000; //22.5 msec elapsed since last write
        }
    case DATA_1:
        //Keep transmit power in sync
        A7105_SetPower(Model.RfPwr);
    case DATA_2:
    case DATA_3:
    case DATA_4:
    case DATA_5:
        TxHbsBuildPkt();
        A7105_WriteData(TxPacket, 16, TxHbsState == DATA_5 ? TxHbsChIdx + 0x23 : TxHbsChIdx);
        if (TxHbsState == DATA_5)
            TxHbsState = DATA_1;
        else
            TxHbsState++;
        return 10000;
    }
    return 0;
}

static u8 TxHbsInit()
{
	u8 i;

    SysTimerStop();

	for(i=0;i<5;i++)
	{
		A7105_Reset();
		usleep(100000);
		SysTimerWatchDogRst();
		if (TxHbsA7105Init())
		{
		    TxHbsSessionId = Model.RfId ? Model.RfId:TxRndId;
		    TxHbsChIdx = TxHbsAllowedCh[rand() % sizeof(TxHbsAllowedCh)];

		    TxHbsState = BIND_1;
		    SysTimerStart(10000, TxHbsCallback);
		    return 1;
		}
	}
	return 0;
}


u32 TxHbsOpen(void)
{
	return TxHbsInit();
}

u32 TxHbsBind(void)
{
	if(TxHbsInit())
	{
		return TxBindCnt=TX_BIND_WAIT;
	}
	return 0;
}

void TxHbsClose(void)
{
    SysTimerStop();
    A7105_Reset();
}
