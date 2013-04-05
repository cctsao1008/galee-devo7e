#include "Tx.h"
#include "RF_CYRF6936.h"


#define PKTS_PER_CHANNEL 4

#define BIND_COUNT 2980

enum PktState {
    WK_BIND,
    WK_BOUND_1,
    WK_BOUND_2,
    WK_BOUND_3,
    WK_BOUND_4,
    WK_BOUND_5,
    WK_BOUND_6,
    WK_BOUND_7,
    WK_BOUND_8,
};

static const u8 TxWkSopCodes[8] = {
    /* Note these are in order transmitted (LSB 1st) */
    0xDF,0xB1,0xC0,0x49,0x62,0xDF,0xC1,0x49 //0x49C1DF6249C0B1DF
};
static const u8 TxWkFailMap[8] = {2, 1, 0, 3, 4, 5, 6, 7};

static enum PktState TxWkPktState;
static u8 TxWkState;
static u32 TxWkFixedId;
static u8 TxWkRadioCh[3];
static u8 *TxWkRadioChPtr;
static u8 TxWkPktNum;
static u8 TxWkLastBeacon;


#define WK2601_CHANMODE  	0		//通道模式，0是5+1；2是6+1；1是直升机模式
#define WK2601_PIT_INV 		1		//直升机模式螺距反向
#define WK2601_PIT_LIMIT	100		//直升机模式螺距行程 看代码100应该是满幅

u8 TxWk2601Mode=WK2601_CHANMODE;

static void TxWkAddPktCrc(u8 init)
{
    u8 add = init;
    u8 xor = init;
    int i;
    for (i = 0; i < 14; i++) {
        add += TxPacket[i];
        xor ^= TxPacket[i];
    }
    TxPacket[14] = xor;
    TxPacket[15] = add & 0xff;
}

const char TxWkBind2801[] = {0xc5, 0x34, 0x60, 0x00, 0x25};
const char TxWkBind2601[] = {0xb9, 0x45, 0xb0, 0xf1, 0x3a};
const char TxWkBind2401[] = {0xa5, 0x23, 0xd0, 0xf0, 0x00};

void TxWkBuildBindPkt(const char *init)
{
    TxPacket[0] = init[0];
    TxPacket[1] = init[1];
    TxPacket[2] = TxWkRadioCh[0];
    TxPacket[3] = TxWkRadioCh[1];
    TxPacket[4] = init[2];
    TxPacket[5] = TxWkRadioCh[2];
    TxPacket[6] = 0xff;
    TxPacket[7] = 0x00;
    TxPacket[8] = 0x00;
    TxPacket[9] = 0x32;
    if (Model.RfChNum <= 4) TxPacket[10]  = 0x10 | ((TxWkFixedId >> 0)  & 0x0e);
    else					TxPacket[10]  = (TxWkFixedId >> 0) & 0xff;
    TxPacket[11] = (TxWkFixedId >> 8)  & 0xff;
    TxPacket[12] = ((TxWkFixedId >> 12) & 0xf0) | TxWkPktNum;
    TxPacket[13] = init[3];
    TxWkAddPktCrc(init[4]);
}

static s16 TxWkGetCh(u8 ch, s32 scale, s32 center, s32 range)
{
    s32 value = (s32)TxChValue[ch] * scale / STK_TRV + center;
    if (value < center - range)	value = center - range;
    if (value > center + range)	value = center + range;
    return value;
}

static void TxWkBuildDataPkt2401()
{
    u8 i;
    u16 msb = 0;
    u8 offset = 0;
    for (i = 0; i < 4; i++)
    {
        if (i == 2)   offset = 1;
        s16 value = TxWkGetCh(i, 0x800, 0, 0xA00); //12 bits, allow value to go to 125%
        u16 base = abs(value) >> 2;  //10 bits is the base value
        u16 trim = abs(value) & 0x03; //lowest 2 bits represent trim
        if (base >= 0x200) {  //if value is > 100%, remainder goes to trim
            trim = 4 *(base - 0x200);
            base = 0x1ff;
        }
        base = (value >= 0) ? 0x200 + base : 0x200 - base;
        trim = (value >= 0) ? 0x200 + trim : 0x200 - trim;
     
        TxPacket[2*i+offset]   = base & 0xff;
        TxPacket[2*i+offset+1] = trim & 0xff;
        msb = (msb << 4) | ((base >> 6) & 0x0c) | ((trim >> 8) & 0x03);
    }
    TxPacket[4] = msb >> 8; //Ele/Ail MSB
    TxPacket[9] = msb & 0xff; //Thr/Rud MSB
    TxPacket[10]  = 0xe0 | ((TxWkFixedId >> 0)  & 0x0e);
    TxPacket[11] = (TxWkFixedId >> 8)  & 0xff;
    TxPacket[12] = ((TxWkFixedId >> 12) & 0xf0) | TxWkPktNum;
    TxPacket[13] = 0xf0; //FIXME - What is this?
    TxWkAddPktCrc(0x00);
}

#define PCT(pct, max) (((max) * (pct) + 1L) / 1000)
#define MAXTHR 426 //Measured to provide equal value at +/-0
static void TxWkCh6p1_2601(int frame, int *_v1, int *_v2)
{
    s16 thr = TxWkGetCh(2, 1000, 0, 1000);
    int v1;
    int thr_rev = 0, pitch_rev = 0;
    /*Throttle is computed as follows:
        val <= -78%    : thr = (100-42.6)% * (-val-78%) / 22% + 42.6%, tcurve=100%, thr_rev=1
        -78% < val < 0 : thr = 42.6%, tcurve = (-val)/78%, thr_rev=1
        0 <= val < 78% : thr = 42.6%, tcurve = 100% - val/78%, thr_rev=0
        78% <= val     : thr = (100-42.6)% * (val-78%) / 22% + 42.6%, tcurve=0, thr_rev=0
     */
    if(thr > 0) {
        if(thr >= 780) { //78%
            v1 = 0; //thr = 60% * (x - 78%) / 22% + 40%
            thr = PCT(1000-MAXTHR,512) * (thr-PCT(780,1000)) / PCT(220,1000) + PCT(MAXTHR,512);
        } else {
            v1 = 1023 - 1023 * thr / 780;
            thr = PCT(MAXTHR, 512); //40%
        }
    } else {
        thr = -thr;
        thr_rev = 1;
        if(thr >= 780) { //78%
            v1 = 1023; //thr = 60% * (x - 78%) / 22% + 40%
            thr = PCT(1000-MAXTHR,512) * (thr-PCT(780,1000)) / PCT(220,1000) + PCT(MAXTHR,512);
            if (thr >= 512)
                thr = 511;
        } else {
            v1 = 1023 * thr / 780;
            thr = PCT(MAXTHR, 512); //40%
        }
    }
    if (thr >= 512)
        thr = 511;
    TxPacket[2] = thr & 0xff;
    TxPacket[4] = (TxPacket[4] & 0xF3) | ((thr >> 6) & 0x04);

    s16 pitch= TxWkGetCh(5, 0x400, 0, 0x400);
    if (pitch < 0) {
        pitch_rev = 1;
        pitch = -pitch;
    }
    if (frame == 1) {
        //Pitch curve and range
        if (thr > PCT(MAXTHR, 512)) {
            //v2 = pit% * ( 1 - (thr% - 40) / 60 * 16%)
            *_v2 = pitch - pitch * 16 * (thr - PCT(MAXTHR, 512)) / PCT(1000 - MAXTHR, 512) / 100;
        } else {
            *_v2 = pitch;
        }
        *_v1 = 0;
    } else if (frame == 2) {
        //Throttle curve & Expo
        *_v1 = v1;
        *_v2 = 512;
    }
    TxPacket[7] = (thr_rev << 5) | (pitch_rev << 2); //reverse bits
    TxPacket[8] = 0;
}

static void TxWkCh5p1_2601(int frame, int *v1, int *v2)
{
    (void)v1;
    //Zero out pitch, provide ail, ele, thr, rud, gyr + gear
    if (frame == 1) {
        //Pitch curve and range
        *v2 = 0;
    }
    TxPacket[7] = 0;
    TxPacket[8] = 0;
}

static void TxWkChHeli_2601(int frame, int *v1, int *v2)
{
    (void)frame;
    //pitch is controlled by rx
    //we can only control fmode, pit-reverse and pit/thr rate

    s16 pit_rate = TxWkGetCh(5, 0x400, 0, 0x400);
    int fmode = 1;
    if (pit_rate < 0) {
        pit_rate = -pit_rate;
        fmode = 0;
    }
    if (frame == 1) {
        //Pitch curve and range
        *v1 = pit_rate;
        *v2 = WK2601_PIT_LIMIT * 0x400 / 100 + 0x400;
    }
    TxPacket[7] = (WK2601_PIT_INV << 2); //reverse bits
    TxPacket[8] = fmode ? 0x02 : 0x00;
}

static void TxWkBuildDataPkt2601()
{
    u8 i;
    u8 msb = 0;
    u8 frame = (TxWkPktNum % 3);
    for (i = 0; i < 4; i++) {
        s16 value = TxWkGetCh(i, 0x190, 0, 0x1FF);
        u16 mag = value < 0 ? -value : value;
        TxPacket[i] = mag & 0xff;
        msb = (msb << 2) | ((mag >> 8) & 0x01) | (value < 0 ? 0x02 : 0x00);
    }
    TxPacket[4] = msb;
    int v1 = 0x200, v2 = 0x200;
    if (frame == 0) {
        //Gyro & Rudder mix
        v1 = TxWkGetCh(6, 0x200, 0x200, 0x200);
        v2 = 0;
    }
    if (TxWk2601Mode== 1)        TxWkChHeli_2601(frame, &v1, &v2);
    else if (TxWk2601Mode == 2)  TxWkCh6p1_2601(frame, &v1, &v2);
    else 				         TxWkCh5p1_2601(frame, &v1, &v2);
    	
    if (v1 > 1023)    v1 = 1023;
    if (v2 > 1023)    v2 = 1023;
    TxPacket[5] = v2 & 0xff;
    TxPacket[6] = v1 & 0xff;
    //TxPacket[7] handled by channel code
    TxPacket[8] |= (TxWkGetCh(4, 0x190, 0, 0x1FF) > 0 ? 1 : 0);
    TxPacket[9] =  ((v1 >> 4) & 0x30)
               | ((v2 >> 2) & 0xc0)
               | 0x04 | frame;
    TxPacket[10]  = (TxWkFixedId >> 0)  & 0xff;
    TxPacket[11] = (TxWkFixedId >> 8)  & 0xff;
    TxPacket[12] = ((TxWkFixedId >> 12) & 0xf0) | TxWkPktNum;
    TxPacket[13] = 0xff;

    TxWkAddPktCrc(0x3A);
}

static void TxWkBuildDataPkt2801()
{
    u8 i;
    u16 msb = 0;
    u8 offset = 0;
    u8 sign = 0;
    for (i = 0; i < 8; i++)
    {
        if (i == 4)   offset = 1;
        s16 value = TxWkGetCh(i, 0x190, 0, 0x3FF);
        u16 mag = value < 0 ? -value : value;
        TxPacket[i+offset] = mag & 0xff;
        msb = (msb << 2) | ((mag >> 8) & 0x03);
        if (value < 0)     sign |= 1 << i;
    }
    TxPacket[4] = msb >> 8;
    TxPacket[9] = msb  & 0xff;
    TxPacket[10]  = (TxWkFixedId >> 0)  & 0xff;
    TxPacket[11] = (TxWkFixedId >> 8)  & 0xff;
    TxPacket[12] = ((TxWkFixedId >> 12) & 0xf0) | TxWkPktNum;
    TxPacket[13] = sign;
    TxWkAddPktCrc(0x25);
}

void TxWkBuildBeaconPkt2801()
{
    TxWkLastBeacon ^= 1;
    u8 i;
    u8 en = 0;
    u8 bind_state;
    if (Model.RfId)
    {
        if (TxBindCnt)	bind_state = 0xe4;
        else            	bind_state = 0x1b;
    }
    else					bind_state = 0x99;
    	
    for (i = 0; i < 4; i++)
    {
    	//失控保护
        //if (Model.limits[TxWkFailMap[i + TxWkLastBeacon * 4]].flags & CH_FAILSAFE_EN)
        //{
        //    s32 value = Model.limits[TxWkFailMap[i + TxWkLastBeacon * 4]].failsafe + 128;
        //    if (value > 255)      value = 255;
        //    if (value < 0)        value = 0;
        //    TxPacket[i+1] = value;
        //    en |= 1 << i;
        //}
        //else
        {
            TxPacket[i+1] = 0;
        }
    }

    TxPacket[0] = en;
    TxPacket[5] = TxPacket[4];
    TxPacket[4] = TxWkLastBeacon << 6;
    TxPacket[6] = TxWkRadioCh[0];
    TxPacket[7] = TxWkRadioCh[1];
    TxPacket[8] = TxWkRadioCh[2];
    TxPacket[9] = bind_state;
    TxPacket[10]  = (TxWkFixedId >> 0)  & 0xff;
    TxPacket[11] = (TxWkFixedId >> 8)  & 0xff;
    TxPacket[12] = ((TxWkFixedId >> 12) & 0xf0) | TxWkPktNum;
    TxPacket[13] = 0x00; //Does this matter?  in the docs it is the same as the data TxPacket
    TxWkAddPktCrc(0x1C);
}

static void TxWkCyrfInit()
{
    /* Initialise CYRF chip */
    CYRF_WriteRegister(CYRF_03_TX_CFG, 0x28 | Model.RfPwr);
    CYRF_WriteRegister(CYRF_06_RX_CFG, 0x4A);
    CYRF_WriteRegister(CYRF_0B_PWR_CTRL, 0x00);
    CYRF_WriteRegister(CYRF_0C_XTAL_CTRL, 0xC0);
    CYRF_WriteRegister(CYRF_0D_IO_CFG, 0x04);
    CYRF_WriteRegister(CYRF_0F_XACT_CFG, 0x2C);
    CYRF_WriteRegister(CYRF_10_FRAMING_CFG, 0xEE);
    CYRF_WriteRegister(CYRF_1B_TX_OFFSET_LSB, 0x55);
    CYRF_WriteRegister(CYRF_1C_TX_OFFSET_MSB, 0x05);
    CYRF_WriteRegister(CYRF_1D_MODE_OVERRIDE, 0x18);
    CYRF_WriteRegister(CYRF_32_AUTO_CAL_TIME, 0x3C);
    CYRF_WriteRegister(CYRF_35_AUTOCAL_OFFSET, 0x14);
    CYRF_WriteRegister(CYRF_1E_RX_OVERRIDE, 0x90);
    CYRF_WriteRegister(CYRF_1F_TX_OVERRIDE, 0x00);
    CYRF_WriteRegister(CYRF_01_TX_LENGTH, 0x10);
    CYRF_WriteRegister(CYRF_0F_XACT_CFG, 0x2C);
    CYRF_WriteRegister(CYRF_28_CLK_EN, 0x02);
    CYRF_WriteRegister(CYRF_27_CLK_OVERRIDE, 0x02);
    CYRF_ConfigSOPCode(TxWkSopCodes);
    CYRF_WriteRegister(CYRF_0F_XACT_CFG, 0x28);
    CYRF_WriteRegister(CYRF_1E_RX_OVERRIDE, 0x10);
    CYRF_WriteRegister(CYRF_0E_GPIO_CTRL, 0x20);
    CYRF_WriteRegister(CYRF_0F_XACT_CFG, 0x2C);
}
 
static u16 TxWkCallback()
{
    if (TxWkState == 0)
    {
        TxWkState = 1;
		///////////////////////////////////////////////////2801
        if(Model.RfChNum >= 7)
		{
		    switch(TxWkPktState) {
		        case WK_BIND:
		            TxWkBuildBindPkt(TxWkBind2801);
		            if (--TxBindCnt == 0)
		            {
		                TxWkPktState = WK_BOUND_1;
		            }
		            break;
		        case WK_BOUND_1:
		        case WK_BOUND_2:
		        case WK_BOUND_3:
		        case WK_BOUND_4:
		        case WK_BOUND_5:
		        case WK_BOUND_6:
		        case WK_BOUND_7:
		            TxWkBuildDataPkt2801();
		            TxWkPktState++;
		            break;
		        case WK_BOUND_8:
		            TxWkBuildBeaconPkt2801();
		            TxWkPktState = WK_BOUND_1;
		            if (TxBindCnt) TxBindCnt--;
		            break;
		    }
		    TxWkPktNum = (TxWkPktNum + 1) % 12;
		}        	
		///////////////////////////////////////////////////2601
        else if(Model.RfChNum >= 5)
        {
		    if (TxBindCnt)
		    {
		        TxBindCnt--;
		        TxWkBuildBindPkt(TxWkBind2601);
		    }
		    else
		    {
		        TxWkBuildDataPkt2601();
		    }
		    TxWkPktNum = (TxWkPktNum + 1) % 12;
		}
		///////////////////////////////////////////////////2401
        else
        {        	
		    if (TxBindCnt)
		    {
		        TxBindCnt--;
		        TxWkBuildBindPkt(TxWkBind2401);
		    }
		   	else
		    {
		        TxWkBuildDataPkt2401();
		    }
		    TxWkPktNum = (TxWkPktNum + 1) % 12;
		}

        CYRF_WriteDataPacket(TxPacket);
        return 1600;
    }
    
    TxWkState = 0;
    while(! (CYRF_ReadRegister(0x04) & 0x02)) ;
    
    if((TxWkPktNum & 0x03) == 0)
    {
        TxWkRadioChPtr = (TxWkRadioChPtr == &TxWkRadioCh[2]) ? TxWkRadioCh : TxWkRadioChPtr + 1;
        CYRF_ConfigRFChannel(*TxWkRadioChPtr);
        //Keep transmit power updated
        CYRF_WriteRegister(CYRF_03_TX_CFG, 0x28 | Model.RfPwr);
    }
    return 1200;
}

void TxWkInit(u8 bind)
{
    SysTimerStop();
    CYRF_Reset();
    TxWkCyrfInit();
    CYRF_ConfigRxTx(1);
    CYRF_FindBestChannels(TxWkRadioCh, 3, 4, 4, 80);
    TxWkRadioChPtr = TxWkRadioCh;
    CYRF_ConfigRFChannel(*TxWkRadioChPtr);

    TxWkPktNum = 0;
    TxWkState = 0;
    TxWkLastBeacon = 0;
    
    if (!Model.RfId)	TxWkFixedId = TxRndId;
    else		        TxWkFixedId = Model.RfId;
    
    if (Model.RfChNum <= 4)   TxWkFixedId |= 0x01;  //Fixed ID must be odd for 2401
    
    //如果是2401 2601 或者是动态ID就进行对码
    if(Model.RfChNum<7 || !Model.RfId)	bind=1;
    
    //对码判断
    if(bind)
    {
        TxBindCnt = BIND_COUNT;
        TxWkPktState = WK_BIND;
    }
    else
    {
    	TxWkPktState = WK_BOUND_1;
        TxBindCnt = 0;
    }
    
    CYRF_ConfigRFChannel(*TxWkRadioChPtr);
    SysTimerStart(2800, TxWkCallback);
}


u32 TxWkOpen(void)
{
	TxWkInit(0);
	return 1;
}

u32 TxWkBind(void)
{
	TxWkInit(1);
	return BIND_COUNT;//返回对码界面显示秒数
}

void TxWkClose(void)
{
    SysTimerStop();
    CYRF_Reset();
}
