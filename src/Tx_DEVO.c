#include "Tx.h"
#include "RF_CYRF6936.h"

#define DEVO_RX	0

#define PKTS_PER_CHANNEL 4

#define BIND_COUNT 5000

#define TELEMETRY_ENABLE 0x30

enum PktState {
    DEVO_BIND,
    DEVO_BIND_SENDCH,
    DEVO_BOUND,
    DEVO_BOUND_1,
    DEVO_BOUND_2,
    DEVO_BOUND_3,
    DEVO_BOUND_4,
    DEVO_BOUND_5,
    DEVO_BOUND_6,
    DEVO_BOUND_7,
    DEVO_BOUND_8,
    DEVO_BOUND_9,
    DEVO_BOUND_10,
};

static const u8 TxDevoSopCodes[][8] = {
    /* Note these are in order transmitted (LSB 1st) */
    /* 0 */ {0x3C,0x37,0xCC,0x91,0xE2,0xF8,0xCC,0x91}, //0x91CCF8E291CC373C
    /* 1 */ {0x9B,0xC5,0xA1,0x0F,0xAD,0x39,0xA2,0x0F}, //0x0FA239AD0FA1C59B
    /* 2 */ {0xEF,0x64,0xB0,0x2A,0xD2,0x8F,0xB1,0x2A}, //0x2AB18FD22AB064EF
    /* 3 */ {0x66,0xCD,0x7C,0x50,0xDD,0x26,0x7C,0x50}, //0x507C26DD507CCD66
    /* 4 */ {0x5C,0xE1,0xF6,0x44,0xAD,0x16,0xF6,0x44}, //0x44F616AD44F6E15C
    /* 5 */ {0x5A,0xCC,0xAE,0x46,0xB6,0x31,0xAE,0x46}, //0x46AE31B646AECC5A
    /* 6 */ {0xA1,0x78,0xDC,0x3C,0x9E,0x82,0xDC,0x3C}, //0x3CDC829E3CDC78A1
    /* 7 */ {0xB9,0x8E,0x19,0x74,0x6F,0x65,0x18,0x74}, //0x7418656F74198EB9
    /* 8 */ {0xDF,0xB1,0xC0,0x49,0x62,0xDF,0xC1,0x49}, //0x49C1DF6249C0B1DF
    /* 9 */ {0x97,0xE5,0x14,0x72,0x7F,0x1A,0x14,0x72}, //0x72141A7F7214E597
};


enum PktState TxDevoState;
u8  TxDevoTxState;
u32 TxDevoFixId;
u8  TxDevoChNum;
u8	TxDevoPktNo;
u8  TxDevoRadioCh[5];
u8* TxDevoChPtr;
u8  TxDevoChIdx;
u8  TxDevoUseFixId;
static u8 failsafe_pkt;

void TxDevoScramblePkt(void)
{
    for(u8 i=0; i<15; i++) TxPacket[i+1] ^= CYRF_MfgId[i%4];
}

void TxDevoAddPktSuffix(void)
{
    u8 bind_state;
    if (TxDevoUseFixId)	bind_state=(TxBindCnt>0) ? 0xc0 : 0x80;
    else 				bind_state=0x00;
    
    TxPacket[10] = bind_state | (PKTS_PER_CHANNEL - TxDevoPktNo - 1);
    TxPacket[11] = *(TxDevoChPtr + 1);
    TxPacket[12] = *(TxDevoChPtr + 2);
    TxPacket[13] = TxDevoFixId  & 0xff;
    TxPacket[14] = (TxDevoFixId >> 8) & 0xff;
    TxPacket[15] = (TxDevoFixId >> 16) & 0xff;
}

void TxDevoBuildBeaconPacket(int upper)
{
    TxPacket[0] = ((TxDevoChNum << 4) | 0x07);
    u8 enable = 0;
    int max = 8;
    //int offset = 0;
    if (upper) {
        TxPacket[0] += 1;
        max = 4;
        //offset = 8;
    }
    
    for(int i = 0; i < max; i++)
    {
    	//失控保护
        //if (i + offset < TX_CH_NUM && Model.limits[i+offset].flags & CH_FAILSAFE_EN)
        //{
            //enable |= 0x80 >> i;
            //TxPacket[i+1] = Model.limits[i+offset].failsafe;
        //}
        //else
        {
            TxPacket[i+1] = 0;
        }
    }
    TxPacket[9] = enable;
    TxDevoAddPktSuffix();
}

void TxDevoBuildBindPacket(void)
{
    TxPacket[0] = (TxDevoChNum << 4) | 0x0a;
    TxPacket[1] = TxBindCnt & 0xff;
    TxPacket[2] = (TxBindCnt >> 8);
    TxPacket[3] = *TxDevoChPtr;
    TxPacket[4] = *(TxDevoChPtr + 1);
    TxPacket[5] = *(TxDevoChPtr + 2);
    TxPacket[6] = CYRF_MfgId[0];
    TxPacket[7] = CYRF_MfgId[1];
    TxPacket[8] = CYRF_MfgId[2];
    TxPacket[9] = CYRF_MfgId[3];
    TxDevoAddPktSuffix();
    //The fixed-id portion is scrambled in the bind TxPacket
    //I assume it is ignored
    TxPacket[13] ^= CYRF_MfgId[0];
    TxPacket[14] ^= CYRF_MfgId[1];
    TxPacket[15] ^= CYRF_MfgId[2];
}

static void TxDevoBuildDataPacket()
{
    u8 i;
    TxPacket[0] = (TxDevoChNum << 4) | (0x0b + TxDevoChIdx);
    u8 sign = 0x0b;
    for (i = 0; i < 4; i++) {
        s32 value = (s32)TxChValue[TxDevoChIdx * 4 + i] * 0x640 / TX_TRV;
        if(value < 0) {
            value = -value;
            sign |= 1 << (7 - i);
        }
        TxPacket[2 * i + 1] = value & 0xff;
        TxPacket[2 * i + 2] = (value >> 8) & 0xff;
    }
    TxPacket[9] = sign;
    TxDevoChIdx = TxDevoChIdx + 1;
    if (TxDevoChIdx * 4 >= TxDevoChNum)
        TxDevoChIdx = 0;
    TxDevoAddPktSuffix();
}

#if 0
static s32 float_to_int(u8 *ptr)
{
    s32 value = 0;
    int seen_decimal = 0;
    for(int i = 0; i < 7; i++) {
        if(ptr[i] == '.') {
            value *= 1000;
            seen_decimal = 100;
            continue;
        }
        if(ptr[i] == 0)
            break;
        if(seen_decimal) {
            value += (ptr[i] - '0') * seen_decimal;
            seen_decimal /= 10;
            if(! seen_decimal)
                break;
        } else {
            value = value * 10 + (ptr[i] - '0');
        }
    }
    return value;
}
static void parse_telemetry_packet(u8 *TxPacket)
{
    if((TxPacket[0] & 0xF0) != 0x30)
        return;
    TxDevoScramblePkt(); //This will unscramble the TxPacket
    //if (TxPacket[0] < 0x37) {
    //    memcpy(Telemetry.line[TxPacket[0]-0x30], TxPacket+1, 12);
    //}
    if (TxPacket[0] == TELEMETRY_ENABLE) {
        Telemetry.volt[0] = TxPacket[1]; //In 1/10 of Volts
        Telemetry.volt[1] = TxPacket[3]; //In 1/10 of Volts
        Telemetry.volt[2] = TxPacket[5]; //In 1/10 of Volts
        Telemetry.rpm[0]  = TxPacket[7] * 120; //In RPM
        Telemetry.rpm[1]  = TxPacket[9] * 120; //In RPM
        Telemetry.time[0] = CLOCK_getms();
    }
    if (TxPacket[0] == 0x31) {
        Telemetry.temp[0] = TxPacket[1] == 0xff ? 0 : TxPacket[1] - 20; //In degrees-C
        Telemetry.temp[1] = TxPacket[2] == 0xff ? 0 : TxPacket[2] - 20; //In degrees-C
        Telemetry.temp[2] = TxPacket[3] == 0xff ? 0 : TxPacket[3] - 20; //In degrees-C
        Telemetry.temp[3] = TxPacket[3] == 0xff ? 0 : TxPacket[4] - 20; //In degrees-C
        Telemetry.time[1] = CLOCK_getms();
    }
    /* GPS Data
       32: 30333032302e3832373045fb  = 030°20.8270E
       33: 353935342e373737364e0700  = 59°54.776N
       34: 31322e380000004d4d4e45fb  = 12.8 MMNE (altitude maybe)?
       35: 000000000000302e30300000  = 0.00 (probably speed)
       36: 313832353532313531303132  = 2012-10-15 18:25:52 (UTC)
    */
    if (TxPacket[0] == 0x32) {
        Telemetry.time[2] = CLOCK_getms();
        Telemetry.gps.longitude = ((TxPacket[1]-'0') * 100 + (TxPacket[2]-'0') * 10 + (TxPacket[3]-'0')) * 3600000
                                  + ((TxPacket[4]-'0') * 10 + (TxPacket[5]-'0')) * 60000
                                  + ((TxPacket[7]-'0') * 1000 + (TxPacket[8]-'0') * 100
                                     + (TxPacket[9]-'0') * 10 + (TxPacket[10]-'0')) * 6;
        if (TxPacket[11] == 'W')
            Telemetry.gps.longitude *= -1;
    }
    if (TxPacket[0] == 0x33) {
        Telemetry.time[2] = CLOCK_getms();
        Telemetry.gps.latitude = ((TxPacket[1]-'0') * 10 + (TxPacket[2]-'0')) * 3600000
                                  + ((TxPacket[3]-'0') * 10 + (TxPacket[4]-'0')) * 60000
                                  + ((TxPacket[6]-'0') * 1000 + (TxPacket[7]-'0') * 100
                                     + (TxPacket[8]-'0') * 10 + (TxPacket[9]-'0')) * 6;
        if (TxPacket[10] == 'S')
            Telemetry.gps.latitude *= -1;
    }
    if (TxPacket[0] == 0x34) {
        Telemetry.time[2] = CLOCK_getms();
        Telemetry.gps.altitude = float_to_int(TxPacket+1);
    }
    if (TxPacket[0] == 0x35) {
        Telemetry.time[2] = CLOCK_getms();
        Telemetry.gps.velocity = float_to_int(TxPacket+7);
    }
    if (TxPacket[0] == 0x36) {
        Telemetry.time[2] = CLOCK_getms();
        u8 hour  = (TxPacket[1]-'0') * 10 + (TxPacket[2]-'0');
        u8 min   = (TxPacket[3]-'0') * 10 + (TxPacket[4]-'0');
        u8 sec   = (TxPacket[5]-'0') * 10 + (TxPacket[6]-'0');
        u8 day   = (TxPacket[7]-'0') * 10 + (TxPacket[8]-'0');
        u8 month = (TxPacket[9]-'0') * 10 + (TxPacket[10]-'0');
        u8 year  = (TxPacket[11]-'0') * 10 + (TxPacket[12]-'0'); // + 2000
        Telemetry.gps.time = ((year & 0x3F) << 26)
                           | ((month & 0x0F) << 22)
                           | ((day & 0x1F) << 17)
                           | ((hour & 0x1F) << 12)
                           | ((min & 0x3F) << 6)
                           | ((sec & 0x3F) << 0);
    }
}
#endif

static void cyrf_set_bound_sop_code()
{
    /* crc == 0 isn't allowed, so use 1 if the math results in 0 */
    u8 crc = (CYRF_MfgId[0] + (CYRF_MfgId[1] >> 6) + CYRF_MfgId[2]);
    if(! crc)
        crc = 1;
    u8 sopidx = (0xff &((CYRF_MfgId[0] << 2) + CYRF_MfgId[1] + CYRF_MfgId[2])) % 10;
    CYRF_ConfigRxTx(1);
    CYRF_ConfigCRCSeed((crc << 8) + crc);
    CYRF_ConfigSOPCode(TxDevoSopCodes[sopidx]);
    CYRF_WriteRegister(CYRF_03_TX_CFG, 0x08 | Model.RfPwr);
}

static void TxDevoCyrfInit()
{
    /* Initialise CYRF chip */
    CYRF_WriteRegister(CYRF_1D_MODE_OVERRIDE, 0x39);
    CYRF_WriteRegister(CYRF_03_TX_CFG, 0x08 | Model.RfPwr);
    CYRF_WriteRegister(CYRF_06_RX_CFG, 0x4A);
    CYRF_WriteRegister(CYRF_0B_PWR_CTRL, 0x00);
    CYRF_WriteRegister(CYRF_0D_IO_CFG, 0x04);
    CYRF_WriteRegister(CYRF_0E_GPIO_CTRL, 0x20);
    CYRF_WriteRegister(CYRF_10_FRAMING_CFG, 0xA4);
    CYRF_WriteRegister(CYRF_11_DATA32_THOLD, 0x05);
    CYRF_WriteRegister(CYRF_12_DATA64_THOLD, 0x0E);
    CYRF_WriteRegister(CYRF_1B_TX_OFFSET_LSB, 0x55);
    CYRF_WriteRegister(CYRF_1C_TX_OFFSET_MSB, 0x05);
    CYRF_WriteRegister(CYRF_32_AUTO_CAL_TIME, 0x3C);
    CYRF_WriteRegister(CYRF_35_AUTOCAL_OFFSET, 0x14);
    CYRF_WriteRegister(CYRF_39_ANALOG_CTRL, 0x01);
    CYRF_WriteRegister(CYRF_1E_RX_OVERRIDE, 0x10);
    CYRF_WriteRegister(CYRF_1F_TX_OVERRIDE, 0x00);
    CYRF_WriteRegister(CYRF_01_TX_LENGTH, 0x10);
    CYRF_WriteRegister(CYRF_0C_XTAL_CTRL, 0xC0);
    CYRF_WriteRegister(CYRF_0F_XACT_CFG, 0x10);
    CYRF_WriteRegister(CYRF_27_CLK_OVERRIDE, 0x02);
    CYRF_WriteRegister(CYRF_28_CLK_EN, 0x02);
    CYRF_WriteRegister(CYRF_0F_XACT_CFG, 0x28);
}

static void TxDevoSetRadioCh()
{
    CYRF_FindBestChannels(TxDevoRadioCh, 3, 4, 4, 80);

    //Makes code a little easier to duplicate these here
    TxDevoRadioCh[3] = TxDevoRadioCh[0];
    TxDevoRadioCh[4] = TxDevoRadioCh[1];
}

void TxDevoBuildPacket()
{
    switch(TxDevoState) {
        case DEVO_BIND:
            TxBindCnt--;
            TxDevoBuildBindPacket();
            TxDevoState = DEVO_BIND_SENDCH;
            break;
        case DEVO_BIND_SENDCH:
            TxBindCnt--;
            TxDevoBuildDataPacket();
            TxDevoScramblePkt();
            if (TxBindCnt <= 0)	TxDevoState = DEVO_BOUND;
            else		          	TxDevoState = DEVO_BIND;
            break;
        case DEVO_BOUND:
        case DEVO_BOUND_1:
        case DEVO_BOUND_2:
        case DEVO_BOUND_3:
        case DEVO_BOUND_4:
        case DEVO_BOUND_5:
        case DEVO_BOUND_6:
        case DEVO_BOUND_7:
        case DEVO_BOUND_8:
        case DEVO_BOUND_9:
            TxDevoBuildDataPacket();
            TxDevoScramblePkt();
            TxDevoState++;
            if (TxBindCnt>0)	TxBindCnt--;
            break;
        case DEVO_BOUND_10:
            TxDevoBuildBeaconPacket(TxDevoChNum > 8 ? failsafe_pkt : 0);
            failsafe_pkt = failsafe_pkt ? 0 : 1;
            TxDevoScramblePkt();
            TxDevoState = DEVO_BOUND_1;
            break;
    }
    
    TxDevoPktNo++;
    if(TxDevoPktNo>=PKTS_PER_CHANNEL)   TxDevoPktNo=0;
}

static u16 TxDevoTeleCallback()
{
    if (TxDevoTxState == 0) {
        TxDevoTxState = 1;
        TxDevoBuildPacket();
        CYRF_WriteDataPacket(TxPacket);
        return 900;
    }
    int delay = 100;
    if (TxDevoTxState == 1) {
        while(! (CYRF_ReadRegister(0x04) & 0x02))
            ;
        if (TxDevoState == DEVO_BOUND) {
            /* exit binding TxDevoState */
            TxDevoState = DEVO_BOUND_3;
            cyrf_set_bound_sop_code();
        }
        if(TxDevoPktNo == 0 || TxBindCnt > 0) {
            delay = 1500;
            TxDevoTxState = 15;
        } else {
            CYRF_ConfigRxTx(0); //Receive mode
            CYRF_WriteRegister(0x07, 0x80); //Prepare to receive
            CYRF_WriteRegister(CYRF_05_RX_CTRL, 0x87); //Prepare to receive
        }
    } else {
        if(CYRF_ReadRegister(0x07) & 0x20) { // this won't be true in emulator so we need to simulate it somehow
            CYRF_ReadDataPacket(TxPacket);
            //parse_telemetry_packet(TxPacket);
            delay = 100 * (16 - TxDevoTxState);
            TxDevoTxState = 15;
        }
    }
    TxDevoTxState++;
    if(TxDevoTxState == 16) { //2.3msec have passed
        CYRF_ConfigRxTx(1); //Write mode
        if(TxDevoPktNo == 0) {
            //Keep tx power updated
            CYRF_WriteRegister(CYRF_03_TX_CFG, 0x08 | Model.RfPwr);
            TxDevoChPtr = TxDevoChPtr == &TxDevoRadioCh[2] ? TxDevoRadioCh : TxDevoChPtr + 1;
            CYRF_ConfigRFChannel(*TxDevoChPtr);
        }
        TxDevoTxState = 0;
    }
    return delay;
}


u16 TxDevoCallback()
{
    if (TxDevoTxState == 0)
    {
        TxDevoTxState = 1;
        TxDevoBuildPacket();
        CYRF_WriteDataPacket(TxPacket);
        return 1200;
    }
    TxDevoTxState = 0;
    while(! (CYRF_ReadRegister(0x04) & 0x02))
        ;
    
    if (TxDevoState == DEVO_BOUND)
    {
        /* exit binding TxDevoState */
        TxDevoState = DEVO_BOUND_3;
        cyrf_set_bound_sop_code();
    }
    
    if(TxDevoPktNo == 0)
    {
        //Keep tx power updated
        CYRF_WriteRegister(CYRF_03_TX_CFG, 0x08 | Model.RfPwr);
        TxDevoChPtr = (TxDevoChPtr == &TxDevoRadioCh[2]) ? TxDevoRadioCh : TxDevoChPtr + 1;	//三点跳频
        CYRF_ConfigRFChannel(*TxDevoChPtr);
    }
    
    return 1200;
}


void TxDevoInit(void)
{
    SysTimerStop();
    CYRF_Reset();
    TxDevoCyrfInit();
    CYRF_GetMfgData(CYRF_MfgId);
    CYRF_ConfigRxTx(1);
    CYRF_ConfigCRCSeed(0x0000);
    CYRF_ConfigSOPCode(TxDevoSopCodes[0]);
    TxDevoSetRadioCh();
    
    failsafe_pkt = 0;
    TxDevoChPtr = TxDevoRadioCh;
    //memset(&Telemetry, 0, sizeof(Telemetry));

    CYRF_ConfigRFChannel(*TxDevoChPtr);
    //FIXME: Properly setnumber of channels;
    TxDevoChNum = ((TX_CH_NUM + 3) >> 2) * 4;
    TxDevoPktNo = 0;
    TxDevoChIdx = 0;
    TxDevoTxState = 0;
	
	// 固定或动态ID模式
	if(Model.RfId==0)//动态
	{
		TxDevoUseFixId=0;
		TxDevoFixId=TxRndId;
	}
	else
	{
		TxDevoUseFixId=1;
		TxDevoFixId=Model.RfId;
	}
	   
	SysTimerStart(2400,DEVO_RX ? TxDevoTeleCallback : TxDevoCallback);
}

u32 TxDevoOpen(void)
{	
    TxDevoState = DEVO_BIND;    
    TxDevoInit();
    TxBindCnt = BIND_COUNT/5;

	return 1;
}

u32 TxDevoBind(void)
{
    TxDevoState = DEVO_BIND;    
    TxDevoInit();    
    TxBindCnt = BIND_COUNT;
    return TxBindCnt;
}

void TxDevoClose(void)
{
    SysTimerStop();
    CYRF_Reset();
}
