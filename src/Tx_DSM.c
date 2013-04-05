#include "Tx.h"
#include "RF_CYRF6936.h"

#define RANDOM_CHANNELS 1
#define BIND_CHANNEL 0x0d //This can be any odd channel

//During binding we will send BIND_COUNT/2 packets
//One TxPacket each 10msec
#define BIND_COUNT 600

enum {
    DSM2_BIND = 0,
    DSM2_CHANSEL     = BIND_COUNT + 0,
    DSM2_CH1_WRITE_A = BIND_COUNT + 1,
    DSM2_CH1_CHECK_A = BIND_COUNT + 2,
    DSM2_CH2_WRITE_A = BIND_COUNT + 3,
    DSM2_CH2_CHECK_A = BIND_COUNT + 4,
    DSM2_CH2_READ_A  = BIND_COUNT + 5,
    DSM2_CH1_WRITE_B = BIND_COUNT + 6,
    DSM2_CH1_CHECK_B = BIND_COUNT + 7,
    DSM2_CH2_WRITE_B = BIND_COUNT + 8,
    DSM2_CH2_CHECK_B = BIND_COUNT + 9,
    DSM2_CH2_READ_B  = BIND_COUNT + 10,
};
   
static const u8 TxDsmPnCodes[5][9][8] = {
    /* Note these are in order transmitted (LSB 1st) */
{ /* Row 0 */
  /* Col 0 */ {0x03, 0xBC, 0x6E, 0x8A, 0xEF, 0xBD, 0xFE, 0xF8},
  /* Col 1 */ {0x88, 0x17, 0x13, 0x3B, 0x2D, 0xBF, 0x06, 0xD6},
  /* Col 2 */ {0xF1, 0x94, 0x30, 0x21, 0xA1, 0x1C, 0x88, 0xA9},
  /* Col 3 */ {0xD0, 0xD2, 0x8E, 0xBC, 0x82, 0x2F, 0xE3, 0xB4},
  /* Col 4 */ {0x8C, 0xFA, 0x47, 0x9B, 0x83, 0xA5, 0x66, 0xD0},
  /* Col 5 */ {0x07, 0xBD, 0x9F, 0x26, 0xC8, 0x31, 0x0F, 0xB8},
  /* Col 6 */ {0xEF, 0x03, 0x95, 0x89, 0xB4, 0x71, 0x61, 0x9D},
  /* Col 7 */ {0x40, 0xBA, 0x97, 0xD5, 0x86, 0x4F, 0xCC, 0xD1},
  /* Col 8 */ {0xD7, 0xA1, 0x54, 0xB1, 0x5E, 0x89, 0xAE, 0x86}
},
{ /* Row 1 */
  /* Col 0 */ {0x83, 0xF7, 0xA8, 0x2D, 0x7A, 0x44, 0x64, 0xD3},
  /* Col 1 */ {0x3F, 0x2C, 0x4E, 0xAA, 0x71, 0x48, 0x7A, 0xC9},
  /* Col 2 */ {0x17, 0xFF, 0x9E, 0x21, 0x36, 0x90, 0xC7, 0x82},
  /* Col 3 */ {0xBC, 0x5D, 0x9A, 0x5B, 0xEE, 0x7F, 0x42, 0xEB},
  /* Col 4 */ {0x24, 0xF5, 0xDD, 0xF8, 0x7A, 0x77, 0x74, 0xE7},
  /* Col 5 */ {0x3D, 0x70, 0x7C, 0x94, 0xDC, 0x84, 0xAD, 0x95},
  /* Col 6 */ {0x1E, 0x6A, 0xF0, 0x37, 0x52, 0x7B, 0x11, 0xD4},
  /* Col 7 */ {0x62, 0xF5, 0x2B, 0xAA, 0xFC, 0x33, 0xBF, 0xAF},
  /* Col 8 */ {0x40, 0x56, 0x32, 0xD9, 0x0F, 0xD9, 0x5D, 0x97}
},
{ /* Row 2 */
  /* Col 0 */ {0x40, 0x56, 0x32, 0xD9, 0x0F, 0xD9, 0x5D, 0x97},
  /* Col 1 */ {0x8E, 0x4A, 0xD0, 0xA9, 0xA7, 0xFF, 0x20, 0xCA},
  /* Col 2 */ {0x4C, 0x97, 0x9D, 0xBF, 0xB8, 0x3D, 0xB5, 0xBE},
  /* Col 3 */ {0x0C, 0x5D, 0x24, 0x30, 0x9F, 0xCA, 0x6D, 0xBD},
  /* Col 4 */ {0x50, 0x14, 0x33, 0xDE, 0xF1, 0x78, 0x95, 0xAD},
  /* Col 5 */ {0x0C, 0x3C, 0xFA, 0xF9, 0xF0, 0xF2, 0x10, 0xC9},
  /* Col 6 */ {0xF4, 0xDA, 0x06, 0xDB, 0xBF, 0x4E, 0x6F, 0xB3},
  /* Col 7 */ {0x9E, 0x08, 0xD1, 0xAE, 0x59, 0x5E, 0xE8, 0xF0},
  /* Col 8 */ {0xC0, 0x90, 0x8F, 0xBB, 0x7C, 0x8E, 0x2B, 0x8E}
},
{ /* Row 3 */
  /* Col 0 */ {0xC0, 0x90, 0x8F, 0xBB, 0x7C, 0x8E, 0x2B, 0x8E},
  /* Col 1 */ {0x80, 0x69, 0x26, 0x80, 0x08, 0xF8, 0x49, 0xE7},
  /* Col 2 */ {0x7D, 0x2D, 0x49, 0x54, 0xD0, 0x80, 0x40, 0xC1},
  /* Col 3 */ {0xB6, 0xF2, 0xE6, 0x1B, 0x80, 0x5A, 0x36, 0xB4},
  /* Col 4 */ {0x42, 0xAE, 0x9C, 0x1C, 0xDA, 0x67, 0x05, 0xF6},
  /* Col 5 */ {0x9B, 0x75, 0xF7, 0xE0, 0x14, 0x8D, 0xB5, 0x80},
  /* Col 6 */ {0xBF, 0x54, 0x98, 0xB9, 0xB7, 0x30, 0x5A, 0x88},
  /* Col 7 */ {0x35, 0xD1, 0xFC, 0x97, 0x23, 0xD4, 0xC9, 0x88},
  /* Col 8 */ {0x88, 0xE1, 0xD6, 0x31, 0x26, 0x5F, 0xBD, 0x40}
},
{ /* Row 4 */
  /* Col 0 */ {0xE1, 0xD6, 0x31, 0x26, 0x5F, 0xBD, 0x40, 0x93},
  /* Col 1 */ {0xDC, 0x68, 0x08, 0x99, 0x97, 0xAE, 0xAF, 0x8C},
  /* Col 2 */ {0xC3, 0x0E, 0x01, 0x16, 0x0E, 0x32, 0x06, 0xBA},
  /* Col 3 */ {0xE0, 0x83, 0x01, 0xFA, 0xAB, 0x3E, 0x8F, 0xAC},
  /* Col 4 */ {0x5C, 0xD5, 0x9C, 0xB8, 0x46, 0x9C, 0x7D, 0x84},
  /* Col 5 */ {0xF1, 0xC6, 0xFE, 0x5C, 0x9D, 0xA5, 0x4F, 0xB7},
  /* Col 6 */ {0x58, 0xB5, 0xB3, 0xDD, 0x0E, 0x28, 0xF1, 0xB0},
  /* Col 7 */ {0x5F, 0x30, 0x3B, 0x56, 0x96, 0x45, 0xF4, 0xA1},
  /* Col 8 */ {0x03, 0xBC, 0x6E, 0x8A, 0xEF, 0xBD, 0xFE, 0xF8}
},
};

const u8 TxDsmPnBind[] = { 0xc6,0x94,0x22,0xfe,0x48,0xe6,0x57,0x4e };

const u8 TxDsmChMap4[] = {0, 1, 2, 3, 0xff, 0xff, 0xff};    //Guess
const u8 TxDsmChMap5[] = {0, 1, 2, 3, 4,    0xff, 0xff}; //Guess
const u8 TxDsmChMap6[] = {1, 5, 2, 3, 0,    4,    0xff}; //HP6DSM
const u8 TxDsmChMap7[] = {1, 5, 2, 4, 3,    6,    0}; //DX6i
const u8 TxDsmChMap8[] = {1, 5, 2, 3, 6,    0xff, 0xff, 4, 0, 7,    0xff, 0xff, 0xff, 0xff}; //DX8
const u8 TxDsmChMap9[] = {3, 2, 1, 5, 0,    4,    6,    7, 8, 0xff, 0xff, 0xff, 0xff, 0xff}; //DM9
const u8 * const TxDsmChMap[] = {TxDsmChMap4, TxDsmChMap5, TxDsmChMap6, TxDsmChMap7, TxDsmChMap8, TxDsmChMap9};

u8  TxDsmCh[23];
u8  TxDsmChIdx;
u8  TxDsmSopCol;
u8  TxDsmDataCol;
u16 TxDsmState;
u8  TxDsmCrcIdx;
u16 TxDsmCrc;

void TxDsmBuildBindPkt()
{
    u8 i;
    u16 sum = 384 - 0x10;
    TxPacket[0] = TxDsmCrc >> 8;
    TxPacket[1] = TxDsmCrc & 0xff;
    TxPacket[2] = 0xff ^ CYRF_MfgId[2];
    TxPacket[3] = 0xff ^ CYRF_MfgId[3];
    TxPacket[4] = TxPacket[0];
    TxPacket[5] = TxPacket[1];
    TxPacket[6] = TxPacket[2];
    TxPacket[7] = TxPacket[3];
    for(i = 0; i < 8; i++)
        sum += TxPacket[i];
    TxPacket[8] = sum >> 8;
    TxPacket[9] = sum & 0xff;
    TxPacket[10] = 0x01; //???
    TxPacket[11] = Model.RfChNum;
    if(Model.Protocol == MP_DSMX)       TxPacket[12] = Model.RfChNum < 8 ? 0xb2 : 0xb2;	//老版本此处是a2 b2
    else 						        TxPacket[12] = Model.RfChNum < 8 ? 0x01 : 0x02;
    TxPacket[13] = 0x00; //???
    for(i = 8; i < 14; i++)
        sum += TxPacket[i];
    TxPacket[14] = sum >> 8;
    TxPacket[15] = sum & 0xff;
}

void TxDsmBuildDataPkt(u8 upper)
{
    u8 i;
    const u8 *chmap = TxDsmChMap[Model.RfChNum - 4];
    if (Model.Protocol == MP_DSMX) {
        TxPacket[0] = CYRF_MfgId[2];
        TxPacket[1] = CYRF_MfgId[3];
    } else {
        TxPacket[0] = 0xff ^ CYRF_MfgId[2];
        TxPacket[1] = 0xff ^ CYRF_MfgId[3];
    }
    u8 bits = Model.Protocol == MP_DSMX ? 11 : 10;
    u16 max = 1 << bits;
    u16 pct_100 = (u32)max * 100 / 150;
    for (i = 0; i < 7; i++) {
       s32 value;
       if (chmap[upper*7 + i] == 0xff) {
           value = 0xffff;
       } else {
           value = (s32)TxChValue[chmap[upper * 7 + i]] * (pct_100 / 2) / STK_TRV + (max / 2);
           if (value >= max)
               value = max-1;
           else if (value < 0)
               value = 0;
           value = (upper && i == 0 ? 0x8000 : 0) | (chmap[upper * 7 + i] << bits) | value;
       }
       TxPacket[i*2+2] = (value >> 8) & 0xff;
       TxPacket[i*2+3] = (value >> 0) & 0xff;
    }
}

static u8 TxDsmGetPnRow(u8 channel)
{
    return Model.Protocol == MP_DSMX
           ? (channel - 2) % 5
           : channel % 5;
}

static const u8 TxDsmCyrfRegs[][2] = {
    {CYRF_1D_MODE_OVERRIDE, 0x01},
    {CYRF_28_CLK_EN, 0x02},
    {CYRF_32_AUTO_CAL_TIME, 0x3c},
    {CYRF_35_AUTOCAL_OFFSET, 0x14},
    {CYRF_0D_IO_CFG, 0x04}, //From Devo - Enable PACTL as GPIO
    {CYRF_0E_GPIO_CTRL, 0x20}, //From Devo
    {CYRF_06_RX_CFG, 0x48},
    {CYRF_1B_TX_OFFSET_LSB, 0x55},
    {CYRF_1C_TX_OFFSET_MSB, 0x05},
    {CYRF_0F_XACT_CFG, 0x24},
    {CYRF_03_TX_CFG, 0x38 | 7},
    {CYRF_12_DATA64_THOLD, 0x0a},
    {CYRF_0C_XTAL_CTRL, 0xC0}, //From Devo - Enable XOUT as GPIO
    {CYRF_0F_XACT_CFG, 0x04},
    {CYRF_39_ANALOG_CTRL, 0x01},
    {CYRF_0F_XACT_CFG, 0x24}, //Force IDLE
    {CYRF_29_RX_ABORT, 0x00}, //Clear RX abort
    {CYRF_12_DATA64_THOLD, 0x0a}, //set pn correlation threshold
    {CYRF_10_FRAMING_CFG, 0x4a}, //set sop len and threshold
    {CYRF_29_RX_ABORT, 0x0f}, //Clear RX abort?
    {CYRF_03_TX_CFG, 0x38 | 7}, //Set 64chip, SDE mode, max-power
    {CYRF_10_FRAMING_CFG, 0x4a}, //set sop len and threshold
    {CYRF_1F_TX_OVERRIDE, 0x04}, //disable tx CRC
    {CYRF_1E_RX_OVERRIDE, 0x14}, //disable rx TxDsmCrc
    {CYRF_14_EOP_CTRL, 0x02}, //set EOP sync == 2
    {CYRF_01_TX_LENGTH, 0x10}, //16byte TxPacket
};

static void TxDsmCyrfConfig()
{
    for(u32 i = 0; i < sizeof(TxDsmCyrfRegs) / 2; i++)
        CYRF_WriteRegister(TxDsmCyrfRegs[i][0], TxDsmCyrfRegs[i][1]);
    CYRF_WritePreamble(0x333304);
    CYRF_ConfigRFChannel(0x61);
}

void TxDsmInitBind()
{
    u8 data_code[32];
    CYRF_ConfigRFChannel(BIND_CHANNEL); //This seems to be random?
    u8 pn_row = TxDsmGetPnRow(BIND_CHANNEL);
    //printf("Ch: %d Row: %d SOP: %d Data: %d\n", BIND_CHANNEL, pn_row, TxDsmSopCol, TxDsmDataCol);
    CYRF_ConfigCRCSeed(TxDsmCrc);
    CYRF_ConfigSOPCode(TxDsmPnCodes[pn_row][TxDsmSopCol]);
    memcpy(data_code, TxDsmPnCodes[pn_row][TxDsmDataCol], 16);
    memcpy(data_code + 16, TxDsmPnCodes[0][8], 8);
    memcpy(data_code + 24, TxDsmPnBind, 8);
    CYRF_ConfigDataCode(data_code, 32);
    TxDsmBuildBindPkt();
}

static const u8 TxDsmCyrfDatas[][2] = {
    {CYRF_05_RX_CTRL, 0x83}, //Initialize for reading RSSI
    {CYRF_29_RX_ABORT, 0x20},
    {CYRF_0F_XACT_CFG, 0x24},
    {CYRF_29_RX_ABORT, 0x00},
    {CYRF_03_TX_CFG, 0x08 | 7},
    {CYRF_10_FRAMING_CFG, 0xea},
    {CYRF_1F_TX_OVERRIDE, 0x00},
    {CYRF_1E_RX_OVERRIDE, 0x00},
    {CYRF_03_TX_CFG, 0x28 | 7},
    {CYRF_12_DATA64_THOLD, 0x3f},
    {CYRF_10_FRAMING_CFG, 0xff},
    {CYRF_0F_XACT_CFG, 0x24}, //Switch from reading RSSI to Writing
    {CYRF_29_RX_ABORT, 0x00},
    {CYRF_12_DATA64_THOLD, 0x0a},
    {CYRF_10_FRAMING_CFG, 0xea},
};

static void TxDsmCyrfConfigData()
{
    for(u32 i = 0; i < sizeof(TxDsmCyrfDatas) / 2; i++)
        CYRF_WriteRegister(TxDsmCyrfDatas[i][0], TxDsmCyrfDatas[i][1]);
}

static void TxDsmSetSopDataCrc()
{
    u8 pn_row = TxDsmGetPnRow(TxDsmCh[TxDsmChIdx]);
    //printf("Ch: %d Row: %d SOP: %d Data: %d\n", ch[TxDsmChIdx], pn_row, TxDsmSopCol, TxDsmDataCol);
    CYRF_ConfigRFChannel(TxDsmCh[TxDsmChIdx]);
    CYRF_ConfigCRCSeed(TxDsmCrcIdx ? ~TxDsmCrc : TxDsmCrc);
    CYRF_ConfigSOPCode(TxDsmPnCodes[pn_row][TxDsmSopCol]);
    CYRF_ConfigDataCode(TxDsmPnCodes[pn_row][TxDsmDataCol], 16);
    /* setup for next iteration */
    if(Model.Protocol == MP_DSMX)
        TxDsmChIdx = (TxDsmChIdx + 1) % 23;
    else
        TxDsmChIdx = (TxDsmChIdx + 1) % 2;
    TxDsmCrcIdx = !TxDsmCrcIdx;
}

static void TxDsmXBuildCh()
{
    int idx = 0;
    u32 id = ~((CYRF_MfgId[0] << 24) | (CYRF_MfgId[1] << 16) | (CYRF_MfgId[2] << 8) | (CYRF_MfgId[3] << 0));
    u32 id_tmp = id;
    while(idx < 23) {
        int i;
        int count_3_27 = 0, count_28_51 = 0, count_52_76 = 0;
        id_tmp = id_tmp * 0x0019660D + 0x3C6EF35F; // Randomization
        u8 next_ch = ((id_tmp >> 8) % 0x49) + 3;       // Use least-significant byte and must be larger than 3
        if (((next_ch ^ id) & 0x01 )== 0)
            continue;
        for (i = 0; i < idx; i++) {
            if(TxDsmCh[i] == next_ch)
                break;
            if(TxDsmCh[i] <= 27)
                count_3_27++;
            else if (TxDsmCh[i] <= 51)
                count_28_51++;
            else
                count_52_76++;
        }
        if (i != idx)
            continue;
        if ((next_ch < 28 && count_3_27 < 8)
          ||(next_ch >= 28 && next_ch < 52 && count_28_51 < 7)
          ||(next_ch >= 52 && count_52_76 < 8))
        {
            TxDsmCh[idx++] = next_ch;
        }
    }
}


static u16 TxDsmCallback()
{
#define CH1_CH2_DELAY 4010  // Time between write of channel 1 and channel 2
#define WRITE_DELAY   1550  // Time after write to verify write complete
#define READ_DELAY     400  // Time before write to check read TxDsmState, and switch TxDsmCh
    if(TxDsmState < DSM2_CHANSEL)
    {
    	if(TxDsmState==DSM2_BIND)	TxBindCnt=BIND_COUNT;
    	else if(TxBindCnt)	TxBindCnt--;
        //Binding
        TxDsmState += 1;
        if(TxDsmState & 1)
        {
            //Send TxPacket on even states
            //Note TxDsmState has already incremented,
            // so this is actually 'even' TxDsmState
            CYRF_WriteDataPacket(TxPacket);
            return 8500;
        }
        else
        {
            //Check status on odd states
            CYRF_ReadRegister(CYRF_04_TX_IRQ_STATUS);
            return 1500;
        }
    }
    else if(TxDsmState < DSM2_CH1_WRITE_A)
    {
        if(TxBindCnt) TxBindCnt=0;	//对码结束
        
        //Select TxDsmCh and configure for writing data
        //CYRF_FindBestChannels(ch, 2, 10, 1, 79);
        TxDsmCyrfConfigData();
        CYRF_ConfigRxTx(1);
        TxDsmChIdx = 0;
        TxDsmCrcIdx = 0;
        TxDsmState = DSM2_CH1_WRITE_A;
        TxDsmSetSopDataCrc();
        return 10000;
    }
    else if(TxDsmState == DSM2_CH1_WRITE_A || TxDsmState == DSM2_CH1_WRITE_B
           || TxDsmState == DSM2_CH2_WRITE_A || TxDsmState == DSM2_CH2_WRITE_B)
    {
        if (TxDsmState == DSM2_CH1_WRITE_A || TxDsmState == DSM2_CH1_WRITE_B)
            TxDsmBuildDataPkt(TxDsmState == DSM2_CH1_WRITE_B);
        CYRF_WriteDataPacket(TxPacket);
        TxDsmState++;
        return WRITE_DELAY;
    } else if(TxDsmState == DSM2_CH1_CHECK_A || TxDsmState == DSM2_CH1_CHECK_B) {
        while(! (CYRF_ReadRegister(0x04) & 0x02))
            ;
        TxDsmSetSopDataCrc();
        TxDsmState++;
        return CH1_CH2_DELAY - WRITE_DELAY;
    } else if(TxDsmState == DSM2_CH2_CHECK_A || TxDsmState == DSM2_CH2_CHECK_B) {
        while(! (CYRF_ReadRegister(0x04) & 0x02))
            ;
        if (TxDsmState == DSM2_CH2_CHECK_A) {
            //Keep transmit power in sync
            CYRF_WriteRegister(CYRF_03_TX_CFG, 0x28 | Model.RfPwr);
        }
        if(0)// (Model.proto_opts[PROTOOPTS_TELEMETRY] == TELEM_ON)
        {
            TxDsmState++;
            CYRF_ConfigRxTx(0); //Receive mode
            CYRF_WriteRegister(0x07, 0x80); //Prepare to receive
            CYRF_WriteRegister(CYRF_05_RX_CTRL, 0x87); //Prepare to receive
            return 11000 - CH1_CH2_DELAY - WRITE_DELAY - READ_DELAY;
        }
        else
        {
            TxDsmSetSopDataCrc();
            if (TxDsmState == DSM2_CH2_CHECK_A) {
                if(Model.RfChNum < 8) {
                    TxDsmState = DSM2_CH1_WRITE_A;
                    return 22000 - CH1_CH2_DELAY - WRITE_DELAY;
                }
                TxDsmState = DSM2_CH1_WRITE_B;
            } else {
                TxDsmState = DSM2_CH1_WRITE_A;
            }
            return 11000 - CH1_CH2_DELAY - WRITE_DELAY;
        }
    } else if(TxDsmState == DSM2_CH2_READ_A || TxDsmState == DSM2_CH2_READ_B) {
        //Read telemetry if needed
        if(CYRF_ReadRegister(0x07) & 0x02) {
           CYRF_ReadDataPacket(TxPacket);
           //parse_telemetry_packet();
        }
        if (TxDsmState == DSM2_CH2_READ_A && Model.RfChNum < 8) {
            TxDsmState = DSM2_CH2_READ_B;
            CYRF_WriteRegister(0x07, 0x80); //Prepare to receive
            CYRF_WriteRegister(CYRF_05_RX_CTRL, 0x87); //Prepare to receive
            return 11000;
        }
        if (TxDsmState == DSM2_CH2_READ_A)
            TxDsmState = DSM2_CH1_WRITE_B;
        else
            TxDsmState = DSM2_CH1_WRITE_A;
        CYRF_ConfigRxTx(1); //Write mode
        TxDsmSetSopDataCrc();
        return READ_DELAY;
    } 
    return 0;
}

static void TxDsm2Init(u8 bind)
{
    SysTimerStop();
    CYRF_Reset();

	//自动生成ID，其实这个ID并非随机数，是CYRF高频头的ID，所以每次开机是一样的
    CYRF_GetMfgData(CYRF_MfgId);
    CYRF_MfgId[2] ^= (TxSys.ModelNo & 0xff);//把模型号加入ID，经试验只能修改2，不能修改3
    
    //指定ID(24bit)
	if (Model.RfId) {
	   CYRF_MfgId[0] ^= (Model.RfId >> 0) & 0xff;
	   CYRF_MfgId[1] ^= (Model.RfId >> 8) & 0xff;
	   CYRF_MfgId[2] ^= (Model.RfId >> 16) & 0xff;
	}

    TxDsmCyrfConfig();

    if (Model.Protocol == MP_DSMX)
    {
        TxDsmXBuildCh();
    }
    else
    {
        u8 tmpch[10];
        CYRF_FindBestChannels(tmpch, 10, 5, 3, 75);
        u8 idx = rand() % 10;
        TxDsmCh[0] = tmpch[idx];
        while(1) {
           idx = rand() % 10;
           if (tmpch[idx] != TxDsmCh[0])
               break;
        }
        TxDsmCh[1] = tmpch[idx];
    }

    TxDsmCrc = ~((CYRF_MfgId[0] << 8) + CYRF_MfgId[1]); 
    TxDsmCrcIdx = 0;
    TxDsmSopCol = (CYRF_MfgId[0] + CYRF_MfgId[1] + CYRF_MfgId[2] + 2) & 0x07;
    TxDsmDataCol = 7 - TxDsmSopCol;

    if (Model.RfChNum<6)        Model.RfChNum = 6;
    else if (Model.RfChNum>9)   Model.RfChNum = 9;

    CYRF_ConfigRxTx(1);
    
    if (bind) 
    {
        TxDsmState = DSM2_BIND;
        TxDsmInitBind();
    }
    else
    {
        TxDsmState = DSM2_CHANSEL;
    }
    
    SysTimerStart(10000, TxDsmCallback);
}

u32 TxDsmOpen(void)
{
	TxDsm2Init(0);
	return 1;
}

u32 TxDsmBind(void)
{
	TxDsm2Init(1);
	return TxBindCnt=BIND_COUNT;//返回对码界面显示秒数
}

void TxDsmClose(void)
{
    SysTimerStop();
    CYRF_Reset();
}
