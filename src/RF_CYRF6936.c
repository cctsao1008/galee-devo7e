#include "Func.h"
#include "RF_CYRF6936.H"

u8 CYRF_MfgId[6];

#define CS_HI() gpio_set(GPIOB, GPIO12)   
#define CS_LO() gpio_clear(GPIOB, GPIO12)
#define RS_HI() gpio_set(GPIOB, GPIO11)
#define RS_LO() gpio_clear(GPIOB, GPIO11)

void CYRF_WriteRegister(u8 address, u8 data)
{
    CS_LO();
    spi_xfer(SPI2, 0x80 | address);
    spi_xfer(SPI2, data);
    CS_HI();
}

static void WriteRegisterMulti(u8 address, const u8 data[], u8 length)
{
    unsigned char i;

    CS_LO();
    spi_xfer(SPI2, 0x80 | address);
    for(i = 0; i < length; i++)
    {
        spi_xfer(SPI2, data[i]);
    }
    CS_HI();
}

static void ReadRegisterMulti(u8 address, u8 data[], u8 length)
{
    unsigned char i;

    CS_LO();
    spi_xfer(SPI2, address);
    for(i = 0; i < length; i++)
    {
        data[i] = spi_xfer(SPI2, 0);
    }
    CS_HI();
}

u8 CYRF_ReadRegister(u8 address)
{
    u8 data;

    CS_LO();
    spi_xfer(SPI2, address);
    data = spi_xfer(SPI2, 0);
    CS_HI();
    return data;
}

void CYRF_Reset()
{
    /* Reset the CYRF chip */
    RS_HI();
    usleep(100);
    RS_LO();
    usleep(100);
}

u8 CYRF_MaxPower()
{
    return (*((u8*)0x08001007) == 0) ? CYRF_PWR_100MW : CYRF_PWR_10MW;
}
/*
 *
 */
void CYRF_GetMfgData(u8 data[])
{
    /* Fuses power on */
    CYRF_WriteRegister(0x25, 0xFF);

    ReadRegisterMulti(0x25, data, 6);

    /* Fuses power off */
    CYRF_WriteRegister(0x25, 0x00); 
}
/*
 * 1 - Tx else Rx
 */
void CYRF_ConfigRxTx(u32 TxRx)
{
    if(TxRx)
    {
        CYRF_WriteRegister(0x0E,0x80);
        CYRF_WriteRegister(0x0F,0x2C);
    }
    else
    {
        CYRF_WriteRegister(0x0E,0x20);
        CYRF_WriteRegister(0x0F,0x28);
    }
}
/*
 *
 */
void CYRF_ConfigRFChannel(u8 ch)
{
    CYRF_WriteRegister(0x00,ch);
}

void CYRF_SetPower(u8 power)
{
    u8 val = CYRF_ReadRegister(0x03) & 0xF8;
    CYRF_WriteRegister(0x03, val | (power & 0x07));
}

/*
 *
 */
void CYRF_ConfigCRCSeed(u16 crc)
{
    CYRF_WriteRegister(0x15,crc & 0xff);
    CYRF_WriteRegister(0x16,crc >> 8);
}
/*
 * these are the recommended sop codes from Crpress
 * See "WirelessUSB LP/LPstar and PRoC LP/LPstar Technical Reference Manual"
 */
void CYRF_ConfigSOPCode(const u8 *sopcodes)
{
    //NOTE: This can also be implemented as:
    //for(i = 0; i < 8; i++) WriteRegister)0x23, sopcodes[i];
    WriteRegisterMulti(0x22, sopcodes, 8);
}

void CYRF_ConfigDataCode(const u8 *datacodes, u8 len)
{
    //NOTE: This can also be implemented as:
    //for(i = 0; i < len; i++) WriteRegister)0x23, datacodes[i];
    WriteRegisterMulti(0x23, datacodes, len);
}

void CYRF_WritePreamble(u32 preamble)
{
    CS_LO();
    spi_xfer(SPI2, 0x80 | 0x24);
    spi_xfer(SPI2, preamble & 0xff);
    spi_xfer(SPI2, (preamble >> 8) & 0xff);
    spi_xfer(SPI2, (preamble >> 16) & 0xff);
    CS_HI();
}
/*
 *
 */
void CYRF_StartReceive()
{
    CYRF_WriteRegister(0x05,0x87);
}

void CYRF_ReadDataPacket(u8 dpbuffer[])
{
    ReadRegisterMulti(0x21, dpbuffer, 0x10);
}

void CYRF_WriteDataPacketLen(const u8 dpbuffer[], u8 len)
{
    CYRF_WriteRegister(CYRF_01_TX_LENGTH, len);
    CYRF_WriteRegister(0x02, 0x40);
    WriteRegisterMulti(0x20, dpbuffer, len);
    CYRF_WriteRegister(0x02, 0xBF);
}
void CYRF_WriteDataPacket(const u8 dpbuffer[])
{
    CYRF_WriteDataPacketLen(dpbuffer, 16);
}

u8 CYRF_ReadRSSI(u32 dodummyread)
{
    u8 result;
    if(dodummyread)
    {
        result = CYRF_ReadRegister(0x13);
    }
    result = CYRF_ReadRegister(0x13);
    if(result & 0x80)
    {
        result = CYRF_ReadRegister(0x13);
    }
    return (result & 0x0F);
}

//NOTE: This routine will reset the CRC Seed
void CYRF_FindBestChannels(u8 *channels, u8 len, u8 minspace, u8 min, u8 max)
{
    #define NUM_FREQ 80
    #define FREQ_OFFSET 4
    u8 rssi[NUM_FREQ];

    if (min < FREQ_OFFSET)
        min = FREQ_OFFSET;
    if (max > NUM_FREQ)
        max = NUM_FREQ;

    int i;
    int j;
    memset(channels, 0, sizeof(u8) * len);
    CYRF_ConfigCRCSeed(0x0000);
    CYRF_ConfigRxTx(0);
    //Wait for pre-amp to switch from sned to receive
    usleep(1000);
    for(i = 0; i < NUM_FREQ; i++) {
        CYRF_ConfigRFChannel(i);
        CYRF_ReadRegister(0x13);
        CYRF_StartReceive();
        usleep(10);
        rssi[i] = CYRF_ReadRegister(0x13);
    }

    for (i = 0; i < len; i++) {
        channels[i] = min;
        for (j = min; j < max; j++) {
            if (rssi[j] < rssi[channels[i]]) {
                channels[i] = j;
            }
            
        }
        for (j = channels[i] - minspace; j < channels[i] + minspace; j++) {
            //Ensure we don't reuse any channels within minspace of the selected channel again
            if (j < 0 || j >= NUM_FREQ)
                continue;
            rssi[j] = 0xff;
        }
    }
    CYRF_ConfigRxTx(1);
}
