#include "Func.h"
#include "RF_A7105.H"

#define CS_HI() gpio_set(GPIOA, GPIO13)   
#define CS_LO() gpio_clear(GPIOA, GPIO13)

void A7105_WriteReg(u8 address, u8 data)
{
    CS_LO();
    spi_xfer(SPI2, address);
    spi_xfer(SPI2, data);
    CS_HI();
}

u8 A7105_ReadReg(u8 address)
{
    u8 data;

    CS_LO();
    spi_xfer(SPI2, 0x40 | address);
    spi_disable(SPI2);
    spi_set_bidirectional_receive_only_mode(SPI2);
    spi_enable(SPI2);
    int i;
    for(i = 0; i < 10; i++)
       ;
    spi_disable(SPI2);
    data = spi_read(SPI2);
    CS_HI();
    spi_set_unidirectional_mode(SPI2);
    spi_enable(SPI2);
    return data;
}

void A7105_WriteData(u8 *dpbuffer, u8 len, u8 channel)
{
    int i;
    CS_LO();
    spi_xfer(SPI2, A7105_RST_WRPTR);
    spi_xfer(SPI2, 0x05);
    for (i = 0; i < len; i++)
        spi_xfer(SPI2, dpbuffer[i]);
    CS_HI();

    A7105_WriteReg(0x0F, channel);

    CS_LO();
    spi_xfer(SPI2, A7105_TX);
    CS_HI();
}

void A7105_ReadData(u8 *dpbuffer, u8 len)
{
    A7105_Strobe(0xF0); //A7105_RST_RDPTR
    for(int i = 0; i < len; i++)
        dpbuffer[i] = A7105_ReadReg(0x05);
/*
    CS_LO();
    spi_xfer(SPI2, 0x40 | 0x05);
    spi_disable(SPI2);
    spi_set_bidirectional_receive_only_mode(SPI2);
    spi_enable(SPI2);
    int i;
    for(i = 0; i < 10; i++)
       ;
    spi_disable(SPI2);
    for(i = 0; i < len; i++)
        dpbuffer[i] = spi_read(SPI2);
    CS_HI();
    spi_set_unidirectional_mode(SPI2);
    spi_enable(SPI2);
*/
    return;
}

void A7105_Reset()
{
    A7105_WriteReg(0x00, 0x00);
}
void A7105_WriteID(u32 id)
{
    CS_LO();
    spi_xfer(SPI2, 0x06);
    spi_xfer(SPI2, (id >> 24) & 0xFF);
    spi_xfer(SPI2, (id >> 16) & 0xFF);
    spi_xfer(SPI2, (id >> 8) & 0xFF);
    spi_xfer(SPI2, (id >> 0) & 0xFF);
    CS_HI();
}

void A7105_SetPower(int power)
{
    /*
    Power amp is ~+16dBm so:
    TXPOWER_100uW  = -23dBm == PAC=0 TBG=0
    TXPOWER_300uW  = -20dBm == PAC=0 TBG=1
    TXPOWER_1mW    = -16dBm == PAC=0 TBG=2
    TXPOWER_3mW    = -11dBm == PAC=0 TBG=4
    TXPOWER_10mW   = -6dBm  == PAC=1 TBG=5
    TXPOWER_30mW   = 0dBm   == PAC=2 TBG=7
    TXPOWER_100mW  = 1dBm   == PAC=3 TBG=7
    TXPOWER_150mW  = 1dBm   == PAC=3 TBG=7
    */
    u8 pac, tbg;
    switch(power) {
        case 0: pac = 0; tbg = 0; break;
        case 1: pac = 0; tbg = 1; break;
        case 2: pac = 0; tbg = 2; break;
        case 3: pac = 0; tbg = 4; break;
        case 4: pac = 1; tbg = 5; break;
        case 5: pac = 2; tbg = 7; break;
        case 6: pac = 3; tbg = 7; break;
        case 7: pac = 3; tbg = 7; break;
        default: pac = 0; tbg = 0; break;
    };
    A7105_WriteReg(0x28, (pac << 3) | tbg);
}

void A7105_Strobe(enum A7105_State state)
{
    CS_LO();
    spi_xfer(SPI2, state);
    CS_HI();
}

u8 A7105_CaliCh(u8 ch,u32 timeout)
{
	timeout+=SysTimerClk;
	
    //Set Channel
    A7105_WriteReg(A7105_0F_CHANNEL, ch);
    
    //VCO Calibration
    A7105_WriteReg(A7105_02_CALC, 2);
    
    SysTimerWatchDogRst();
    
    while(SysTimerClk < timeout) 
    {
        if(! A7105_ReadReg(A7105_02_CALC))	
        {
		    if (A7105_ReadReg(A7105_25_VCO_SBCAL_I) & A7105_MASK_VBCF)
		    {
		        //Calibration failed...what do we do?
		    }
        	return 1;
        }
    }
    
    return 0;
}

u8 A7105CaliIf(u32 timeout)
{
	timeout+=SysTimerClk;
	
    //IF Filter Bank Calibration
    A7105_WriteReg(A7105_02_CALC, 1);
    //vco_current =
    A7105_ReadReg(A7105_02_CALC);
    
    SysTimerWatchDogRst();

    while(SysTimerClk < timeout) 
    {
        if(! A7105_ReadReg(A7105_02_CALC))     
        {
        	u8 ifcali = A7105_ReadReg(A7105_22_IF_CALIB_I);
		    A7105_ReadReg(A7105_24_VCO_CURCAL);
		    if(ifcali & A7105_MASK_FBCF)
		    {
		        //Calibration failed...what do we do?
		    }
		    return 1;
        }
    }
    return 0;

}