#include "Tx.h"
#include "RF_CC2500.h"

static u32 TxSkaState;
static u32 TxSkaFixedId;

#define TX_ADDR ((TxSkaFixedId >> 16) & 0xff)
#define TX_CHANNEL ((TxSkaFixedId >> 24) & 0xff)

enum {
    SKYARTEC_PKT1 = 0,
    SKYARTEC_SLEEP1, 
    SKYARTEC_PKT2,
    SKYARTEC_SLEEP2, 
    SKYARTEC_PKT3,
    SKYARTEC_SLEEP3, 
    SKYARTEC_PKT4,
    SKYARTEC_SLEEP4, 
    SKYARTEC_PKT5,
    SKYARTEC_SLEEP5, 
    SKYARTEC_PKT6,
    SKYARTEC_LAST,
};

static void TxSkaCC2500Init()
{
	CC2500_Reset();
	
	CC2500_WriteReg(CC2500_16_MCSM2, 0x07);
	CC2500_WriteReg(CC2500_17_MCSM1, 0x30);
	CC2500_WriteReg(CC2500_1E_WOREVT1, 0x87);
	CC2500_WriteReg(CC2500_1F_WOREVT0, 0x6b);
	CC2500_WriteReg(CC2500_20_WORCTRL, 0xf8);
	CC2500_WriteReg(CC2500_2A_PTEST, 0x7f);
	CC2500_WriteReg(CC2500_2B_AGCTEST, 0x3f);
	CC2500_WriteReg(CC2500_0B_FSCTRL1, 0x09);
	CC2500_WriteReg(CC2500_0C_FSCTRL0, 0x00);
	CC2500_WriteReg(CC2500_0D_FREQ2, 0x5d);
	CC2500_WriteReg(CC2500_0E_FREQ1, 0x93);
	CC2500_WriteReg(CC2500_0F_FREQ0, 0xb1);
	CC2500_WriteReg(CC2500_10_MDMCFG4, 0x2d);
	CC2500_WriteReg(CC2500_11_MDMCFG3, 0x20);
	CC2500_WriteReg(CC2500_12_MDMCFG2, 0x73);
	CC2500_WriteReg(CC2500_13_MDMCFG1, 0x22);
	CC2500_WriteReg(CC2500_14_MDMCFG0, 0xf8);
	CC2500_WriteReg(CC2500_0A_CHANNR, 0xcd);
	CC2500_WriteReg(CC2500_15_DEVIATN, 0x50);
	CC2500_WriteReg(CC2500_21_FREND1, 0xb6);
	CC2500_WriteReg(CC2500_22_FREND0, 0x10);
	CC2500_WriteReg(CC2500_18_MCSM0, 0x18);
	CC2500_WriteReg(CC2500_19_FOCCFG, 0x1d);
	CC2500_WriteReg(CC2500_1A_BSCFG, 0x1c);
	CC2500_WriteReg(CC2500_1B_AGCCTRL2, 0xc7);
	CC2500_WriteReg(CC2500_1C_AGCCTRL1, 0x00);
	CC2500_WriteReg(CC2500_1D_AGCCTRL0, 0xb2);
	CC2500_WriteReg(CC2500_23_FSCAL3, 0xea);
	CC2500_WriteReg(CC2500_24_FSCAL2, 0x0a);
	CC2500_WriteReg(CC2500_25_FSCAL1, 0x00);
	CC2500_WriteReg(CC2500_26_FSCAL0, 0x11);
	CC2500_WriteReg(CC2500_29_FSTEST, 0x59);
	CC2500_WriteReg(CC2500_2C_TEST2, 0x88);
	CC2500_WriteReg(CC2500_2D_TEST1, 0x31);
	CC2500_WriteReg(CC2500_2E_TEST0, 0x0b);
	CC2500_WriteReg(CC2500_00_IOCFG2, 0x0b);
	CC2500_WriteReg(CC2500_02_IOCFG0, 0x06);
	CC2500_WriteReg(CC2500_07_PKTCTRL1, 0x05);
	CC2500_WriteReg(CC2500_08_PKTCTRL0, 0x05);
	CC2500_WriteReg(CC2500_09_ADDR, 0x43);
	CC2500_WriteReg(CC2500_06_PKTLEN, 0xff);
	CC2500_WriteReg(CC2500_04_SYNC1, 0x13);
	CC2500_WriteReg(CC2500_05_SYNC0, 0x18);
	CC2500_Strobe(CC2500_SFTX);
	CC2500_Strobe(CC2500_SFRX);
	CC2500_Strobe(CC2500_SXOFF);
	CC2500_Strobe(CC2500_SIDLE);
}

static void TxSkaAddPktSuffix()
{
    int xor1 = 0;
    int xor2 = 0;
    for(int i = 3; i <= 16; i++)
        xor1 ^= TxPacket[i];
    for(int i = 3; i <= 14; i++)
        xor2 ^= TxPacket[i];

    int sum = TxPacket[3] + TxPacket[5] + TxPacket[7] + TxPacket[9] + TxPacket[11] + TxPacket[13];
    TxPacket[17] = xor1;
    TxPacket[18] = xor2;
    TxPacket[19] = sum & 0xff;
}

static void TxSkaSendDataPkt()
{
    //13 c5 01 0259 0168 0000 0259 030c 021a 0489 f3 7e 0a
    TxPacket[0] = 0x13;                //Length
    TxPacket[1] = TX_ADDR;             //Tx Addr?
    TxPacket[2] = 0x01;                //???
    
    for(int i = 0; i < 7; i++)
    {
        s32 value = (s32)TxChValue[i] * 0x280 / STK_TRV + 0x280;
        if(value < 0)           value = 0;
        if(value > 0x500)       value = 0x500;
        TxPacket[3+2*i] = value >> 8;
        TxPacket[4+2*i] = value & 0xff;
    }
    TxSkaAddPktSuffix();
    //for(int i = 0; i < 20; i++) printf("%02x ", TxPacket[i]); printf("\n");
    CC2500_WriteReg(CC2500_04_SYNC1, ((TxSkaFixedId >> 0) & 0xff));
    CC2500_WriteReg(CC2500_05_SYNC0, ((TxSkaFixedId >> 8) & 0xff));
    CC2500_WriteReg(CC2500_09_ADDR, TX_ADDR);
    CC2500_WriteReg(CC2500_0A_CHANNR, TX_CHANNEL);
    CC2500_WriteData(TxPacket, 20);
}

static void TxSkaSendBindPkt()
{
    //0b 7d 01 01 b2 c5 4a 2f 00 00 c5 d6
    TxPacket[0] = 0x0b;       //Length
    TxPacket[1] = 0x7d;
    TxPacket[2] = 0x01;
    TxPacket[3] = 0x01;
    TxPacket[4] = (TxSkaFixedId >> 24) & 0xff;
    TxPacket[5] = (TxSkaFixedId >> 16) & 0xff;
    TxPacket[6] = (TxSkaFixedId >> 8)  & 0xff;
    TxPacket[7] = (TxSkaFixedId >> 0)  & 0xff;
    TxPacket[8] = 0x00;
    TxPacket[9] = 0x00;
    TxPacket[10] = TX_ADDR;
    u8 xor = 0;
    for(int i = 3; i < 11; i++)   xor ^= TxPacket[i];
    TxPacket[11] = xor;
    CC2500_WriteReg(CC2500_04_SYNC1, 0x7d);
    CC2500_WriteReg(CC2500_05_SYNC0, 0x7d);
    CC2500_WriteReg(CC2500_09_ADDR, 0x7d);
    CC2500_WriteReg(CC2500_0A_CHANNR, 0x7d);
    CC2500_WriteData(TxPacket, 12);
}

static u16 TxSkaCallback()
{
    if (TxSkaState & 0x01)
    {
        CC2500_Strobe(CC2500_SIDLE);
        TxSkaState = (TxSkaState == SKYARTEC_LAST) ? SKYARTEC_PKT1 : TxSkaState + 1;
        return 3000;
    }
    
    if (TxSkaState == SKYARTEC_PKT1 && TxBindCnt)
    {
        TxSkaSendBindPkt();
        TxBindCnt--;
    }
    else
    {
        TxSkaSendDataPkt();
    }
    TxSkaState++;
    return 3000;
}

static void TxSkaInit(void)
{
    SysTimerStop();
    TxSkaCC2500Init();
    
    if (Model.RfId)
    {
        TxSkaFixedId = Model.RfId;
    }
    else
    {
    	TxSkaFixedId = 0xb2c54a2f;
    	TxSkaFixedId ^= CC2500_ReadReg(0xF0)<<24;
        TxSkaFixedId ^= CC2500_ReadReg(0xF1)<<16;
        TxSkaFixedId ^= rand();
    }
    
    if (0 == (TxSkaFixedId & 0xff000000))    TxSkaFixedId |= 0xb2;
    if (0 == (TxSkaFixedId & 0x00ff0000))    TxSkaFixedId |= 0xc5;
    if (0 == (TxSkaFixedId & 0x0000ff00))    TxSkaFixedId |= 0x4a;
    if (0 == (TxSkaFixedId & 0x000000ff))    TxSkaFixedId |= 0x2f;
    
    TxSkaState = SKYARTEC_PKT1;
    TxBindCnt=500;//泉速机子要收到300个对码包才能对码成功

    SysTimerStart(10000, TxSkaCallback);
}


u32 TxSkaOpen(void)
{
	TxSkaInit();
	return 1;
}

u32 TxSkaBind(void)
{
	TxSkaInit();
	return TxBindCnt;
}

void TxSkaClose(void)
{
    SysTimerStop();
	usleep(10000);
	CC2500_Strobe(CC2500_SIDLE);
	usleep(10000);
    CC2500_Reset();
}

