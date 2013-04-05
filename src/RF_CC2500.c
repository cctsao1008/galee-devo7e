#include "Func.h"
#include "RF_CC2500.h"

//GPIOA.14
#define CS_HI() gpio_set(GPIOA, GPIO14)
#define CS_LO() gpio_clear(GPIOA,GPIO14)

u8 CC2500_RfPwr;

void CC2500_WriteReg(u8 address, u8 data)
{
    CS_LO();
    spi_xfer(SPI2, address);
    spi_xfer(SPI2, data);
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

u8 CC2500_ReadReg(u8 address)
{
    CS_LO();
    spi_xfer(SPI2, CC2500_READ_SINGLE | address);
    u8 data = spi_xfer(SPI2, 0);
    CS_HI();
    return data;
}

void CC2500_ReadData(u8 *dpbuffer, int len)
{
    ReadRegisterMulti(CC2500_3F_RXFIFO | CC2500_READ_BURST, dpbuffer, len);
}

void CC2500_Strobe(u8 state)
{
    CS_LO();
    spi_xfer(SPI2, state);
    CS_HI();
}

void CC2500_WriteRegisterMulti(u8 address, const u8 data[], u8 length)
{
    CS_LO();
    spi_xfer(SPI2, CC2500_WRITE_BURST | address);
    for(int i = 0; i < length; i++)
    {
        spi_xfer(SPI2, data[i]);
    }
    CS_HI();
}

void CC2500_WriteData(u8 *dpbuffer, u8 len)
{
    CC2500_Strobe(CC2500_SFTX);
    CC2500_WriteRegisterMulti(CC2500_3F_TXFIFO, dpbuffer, len);
    CC2500_Strobe(CC2500_STX);
}

void CC2500_Reset()
{
	CC2500_RfPwr=0xff;
    CC2500_Strobe(CC2500_SRES);
}


#define PATABLELEN	8
#define PAREGLEN	8  
const u8 PaTableValue[PATABLELEN]={
    0x83,	//TXPOWER_100uW  = -23dBm 
    0x46,	//TXPOWER_300uW  = -20dBm 
    0x86,	//TXPOWER_1mW    = -16dBm 
    0x69,	//TXPOWER_3mW    = -11dBm 
    0x7f,	//TXPOWER_10mW   = -6dBm  
    0xfb,	//TXPOWER_30mW   = 0dBm   
    0xfe,	//TXPOWER_100mW  = 1dBm   
    0xff,	//TXPOWER_150mW  = 1.5dBm   
};
void CC2500_SetPower(u8 p)
{
	if(p>PATABLELEN-1)		p=PATABLELEN-1;
	
	if(p==CC2500_RfPwr)	return;	
	CC2500_RfPwr=p;
	
    CS_LO();
    spi_xfer(SPI2, CC2500_WRITE_BURST | CC2500_3E_PATABLE);
    for(u8 i = 0; i < PAREGLEN; i++)
    {
        spi_xfer(SPI2,PaTableValue[p]);
    }
    CS_HI();
}