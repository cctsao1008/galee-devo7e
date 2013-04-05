#include "Func.h"

#define PROGMEM
#include "ASCII6X8.h"
#include "HZK12.H"

//The screen is 129 characters, but we'll only expoise 128 of them
#define LCD_PHY_WIDTH	129
#define LCD_PAGES 		8
static u8 LcdDispImg[LCD_PHY_WIDTH * LCD_PAGES];
static u8 LcdDispBuf[LCD_PHY_WIDTH];

static u16 LcdLeft, LcdRight;  // After introducing logical view for devo10, the coordinate can be >= 5000
static u16 LcdXpos, LcdYpos;
static s8 LcdDir;

volatile u8 LcdBw;

void LcdCmd(u8 cmd) {
    LCD_CMD();
    LCD_CS_L();
    spi_xfer(SPI1, cmd);
    LCD_CS_H();
}

void LcdDat(u8 cmd) {
    LCD_DAT();
    LCD_CS_L();
    spi_xfer(SPI1, cmd);
    LCD_CS_H();
}

void LcdDisplay(uint8_t on)
{
    LcdCmd(0xAE | (on ? 1 : 0));
}

void LcdSetPageAddress(uint8_t page)
{
    LcdCmd(0xB0 | (page & 0x07));
}

void LcdSetColumnAddress(uint8_t column)
{
    LcdCmd(0x10 | ((column >> 4) & 0x0F));  //MSB
    LcdCmd(column & 0x0F);                  //LSB
}

void LcdSetXy(u8 x,u8 y)
{
	LcdCmd(0xb0|y);
	LcdCmd(0x10|(x>>4));
	LcdCmd(x&0x0f);
}

void LcdSetStartLine(int line)
{
	LcdCmd((line & 0x3F) | 0x40); 
}

void LcdContrast(u8 contrast)
{
    //int data = 0x20 + contrast * 0xC / 10;
    LcdCmd(0x81);
    int c = contrast * 8 + 110; //contrast should range from ~72 to ~200
    LcdCmd(c);
}

void LcdDelay(void)
{    
	volatile int i = 0x8000;
    while(i) i--;
}

void LcdInit()
{
	LcdBw=0;
	
    //Initialization is mostly done in SPI Flash
    //Setup CS as B.0 Data/Control = C.5
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO0);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
    LcdCmd(0xE2);  //Reset
	LcdDelay();
	
    LcdCmd(0xAE);  //Display off
    LcdCmd(0xA6);  //Normal display
    LcdCmd(0xA4);  //All Points Normal
    LcdCmd(0xEA);  //??
    LcdCmd(0xA0);  //ADC Normal
    LcdCmd(0xC4);  //Common Output Mode Scan Rate
    LcdCmd(0x2C); //Power Controller:Booster ON
	LcdDelay();
	
    LcdCmd(0x2E); //Power Controller: VReg ON
	LcdDelay();
	
    LcdCmd(0x2F); //Power Controller: VFollower ON
	LcdDelay();
	
    LcdSetStartLine(0);
    
    // Display data write (6)
    //Clear the screen
    for(int page = 0; page < 9; page++)
    {
        LcdSetPageAddress(page);
        LcdSetColumnAddress(0);
        for(int col = 0; col < 129; col++)
        {
            LcdDat(0x00);
		}
    }
    
    LcdDisplay(1);
    LcdContrast(5);
    
    memset(LcdDispImg, 0, sizeof(LcdDispImg));
    memset(LcdDispBuf, 0, sizeof(LcdDispBuf));
    	
    LcdContrast(3);
}

void LcdClear(u8 bw)
{
    int i,j;
    bw=bw ? 0xff : 0x00;
    
    memset(LcdDispImg, 0, sizeof(LcdDispImg));
    memset(LcdDispBuf, 0, sizeof(LcdDispBuf));
    
    for (i=0; i<LCD_PAGES; i++)
    {
        LcdSetPageAddress(i);
        LcdSetColumnAddress(0);
        for (j=0;j<LCD_PHY_WIDTH;j++)  LcdDat(bw);
    }
}

void LcdDrawStart(u16 x0, u16 y0, u16 x1, u16 y1, enum DrawDir _dir)
{
    if (_dir == DRAW_SWNE) {
        LcdYpos = y1;  // bug fix: must do it this way to draw bmp
        LcdDir = -1;
    } else {
        LcdYpos = y0;
        LcdDir = 1;
    }
    LcdLeft = x0;
    LcdRight = x1;
    LcdXpos = x0;
}
/* Screen coordinates are as follows:
 * (128, 32)   ....   (0, 32)
 *   ...       ....     ...
 * (128, 63)   ....   (0, 63)
 * (128, 0)    ....   (0, 0)
 *   ...       ....     ...
 * (128, 31)   ....   (0, 31)
 */
void LcdDrawStop(void)
{
    int col = 0;
    int p, c;
    for (p = 0; p < LCD_PAGES; p++) {
        int init = 0;
        for (c = 0; c < LCD_PHY_WIDTH; c++) {
            if(LcdDispBuf[c] & (1 << p)) {
                if(! init) {
                    LcdSetPageAddress(p);
                    LcdSetColumnAddress(c);
                } else if(col+1 != c) {
                    LcdSetColumnAddress(c);
                }
                LcdDat(LcdDispImg[p * LCD_PHY_WIDTH + c]);
                col = c;
            }
        }
    }
    memset(LcdDispBuf, 0, sizeof(LcdDispBuf));
}

void LcdDrawPixel(u16 color)
{
    int y = LcdYpos;
    int x = LcdXpos;
    int ycol = y / 8;
    int ybit = y & 0x07;
    if(color) {
        LcdDispImg[ycol * LCD_PHY_WIDTH + x] |= 1 << ybit;
    } else {
        LcdDispImg[ycol * LCD_PHY_WIDTH + x] &= ~(1 << ybit);
    }
    LcdDispBuf[x] |= 1 << ycol;
    LcdXpos++;
    if (LcdXpos > LcdRight) {
        LcdXpos = LcdLeft;
        LcdYpos += LcdDir;
    }
}

void LcdDrawPixelXY(u16 x, u16 y, u16 color)
{
    LcdXpos = x;
    LcdYpos = y;
    LcdDrawPixel(color);
}

/////////////////////////////////////////////
//
// RLE光栅显示函数
//
void LcdDrawRLE(const u8 *data, int len, u32 color)
{
    while(len) {
        u32 c = (*data & 0x80) ? color : 0;
        for(int i = 0; i < (*data & 0x7f); i++)
        {
            LcdDrawPixel(c);
        }
        data++;
        len--;
    }
}

/////////////////////////////////////////////
//
// 固件LOGO显示函数
//
void LcdDrawLogo(const u8 *logo)
{
    int width = (logo[0] << 8) |  logo[1];
    int height= (logo[2] << 8) |  logo[3];
    int size  = (logo[4] << 8) |  logo[5];
    int x = (LCD_W - width) / 2;
    int y = (LCD_H - height) / 2;
    LcdDrawStart(x, y, x + width-1, y + height, DRAW_NWSE);
    LcdDrawRLE(logo+6, size, 0xffff);
    LcdDrawStop();
}

/////////////////////////////////////////////
//
// 英文字符显示函数
//
void LcdWriteChar(u8 c)
{
	const u8 *Lib;
	u8 line;
	
	Lib=ASCII+(u16)c*6;
	
	for (line=0; line<6; line++)
	{
		u8 v=Lib[line];
		if(LcdBw)	v=~v;
  		LcdDat(v);
	}
}

/////////////////////////////////////////////
//
// 清空函数
//
void LcdWriteSp(u8 n)
{
	u8 c=(LcdBw?0xff:0);
	while(n--)	LcdDat(c);
}
/////////////////////////////////////////////
//
// 英文字符串显示函数
// x=0~20
// y=0~8
//
void LcdWriteStr(u8 X,u8 Y,const char *s)
{
	LcdSetXy(X,Y);

    while (*s) 
    {
		LcdWriteChar((u8)*s);
		s++;
	}
}
/////////////////////////////////////////////
//
// 有符号整数显示函数
// x=0~20
// y=0~8
//
u8 LcdHexChar(u8 v)
{
	if(v<10)	return v+'0';
	if(v<16)	return v-10+'A';
	return '#';
}

void LcdWriteHex(u8 X,u8 Y,u32 v,u8 n)
{
	u8 i,j=(n-1)*4;
	LcdSetXy(X,Y);		
	for(i=0;i<n;i++)
	{
		LcdWriteChar(LcdHexChar((v>>j)&0xf));
		v<<=4;
	}
}
/////////////////////////////////////////////
//
// 标准数字显示函数
//
// x,y   以左上角为基点的坐标，单位是像素
// value 要显示的整数
// n     显示几位，n=0时自动计算位数
// dot   显示小数点后几位数字，为0则不显示小数点
// plus  在value为正数时是否显示+号，不论是否显示，都会预留符号的宽度，要想不要符号宽度，需令plus=0xff
// ar    为0时左对齐，为1时右对齐输出
void  LcdDrawInt(u16 x,u16 y,s16 value,u8 n,u8 dot,u8 plus,u8 ar)
{
	//保存符号位
	u8 minus=' ';
	if(value<0) 
	{
		minus='-';
		value=-value;
	}
	else if(value>0)
	{
	    if(plus)	minus='+';
	}
	
	//计算数字位数
	s16 v=value;
	u8 w;
	if(n==0)	for(n=0;v;n++,v/=10);
	if(n<=dot)	n=dot+1;	//如果数字位数不足小数点位置，要补上0成为.xx
	
	//计算宽度
	w=n*6;
	if(dot!=0) w+=2;	//如果有小数点，还要加2位宽度
	if(plus!=0xff)	w+=6;//为整齐，符号位宽度始终预留
	
	//计算掩码
	s16 mask=1;
	u8 i;
	for(i=0;i<n-1;i++) mask*=10;
	
	//如果是右对齐，要重新计算起始位置
	if(ar)	x-=w;
    
    //绘制符号
    if(plus!=0xff)
    {
    	LcdWriteEngChar(x,y,minus); x+=6;
	}
    
	//开始绘制数字
	u8 sz=0;//是否显示0
	for(i=0;i<n;i++)
	{
		v=value/mask%10;
		if(i==n-1)	sz=1;//如果已经是最后1位，则0也要显示
		if(i==n-1-dot && dot)
		{
			sz=1;//如果已到达小数点，则0也要显示
			LcdWriteEngChar(x,y,v+'0');	x+=6;
    		LcdWriteEngChar(x,y,'.');	x+=2;
		}
		else
		{
			if(v)	sz=1;	//如果此位非0，后续0都要显示
			else	if(!sz)	v=' '-'0';//如果是0且未置位sz，则将0替换为空格
			LcdWriteEngChar(x,y,v+'0');		x+=6;
		} 
		mask/=10;
    }
}
/////////////////////////////////////////////
//
// 微小数字显示函数
//  4x5 点阵
const u8 LcdMiniNum[][4]={
	{0x1f,0x11,0x1f,0x00},//0
	{0x09,0x1f,0x01,0x00},//1
	{0x17,0x15,0x1d,0x00},//2
	{0x15,0x15,0x1f,0x00},//3
	{0x1c,0x04,0x1f,0x00},//4
	{0x1d,0x15,0x17,0x00},//5
	{0x1f,0x15,0x17,0x00},//6
	{0x10,0x10,0x1f,0x00},//7
	{0x1f,0x15,0x1f,0x00},//8
	{0x1d,0x15,0x1f,0x00},//9
	{0x01,0x00,0x00,0x00},//.
	{0x04,0x04,0x04,0x00},//-
	{0x07,0x04,0x07,0x04},//m
	{0x07,0x00,0x0f,0x01},//
	{0x07,0x01,0x0f,0x00},//W
	{0x00,0x00,0x00,0x00},//SPACE
	{0x04,0x0e,0x04,0x00},//+
	{0x01,0x06,0x18,0x00},///
	{0x0a,0x00,0x00,0x00},//:
	{0x0a,0x00,0x00,0x00},//
	{0x0a,0x00,0x00,0x00},//
	{0x0a,0x00,0x00,0x00},//
	{0x0a,0x00,0x00,0x00},//
	{0x0f,0x14,0x0F,0x00},//A
	{0x1f,0x15,0x0f,0x00},//B
	{0x1f,0x11,0x11,0x00},//C
	{0x1f,0x11,0x0E,0x00},//D
	{0x1f,0x15,0x15,0x00},//E
	{0x1f,0x14,0x14,0x00},//F
	{0x1f,0x11,0x17,0x00},//G
	{0x1f,0x04,0x1f,0x00},//H
	{0x00,0x1F,0x00,0x00},//I
	{0x03,0x01,0x1F,0x00},//J
	{0x1F,0x04,0x1B,0x00},//K
	{0x1F,0x01,0x01,0x00},//L
	{0x1F,0x08,0x1f,0x00},//M
	{0x1F,0x0e,0x1f,0x00},//N
	{0x0E,0x11,0x0E,0x00},//O
	{0x1f,0x14,0x1C,0x00},//P
	{0x1f,0x11,0x0E,0x00},//---Q
	{0x1f,0x16,0x1D,0x00},//R
	{0x1d,0x15,0x17,0x00},//S
	{0x10,0x1F,0x10,0x00},//T
	{0x1f,0x01,0x1f,0x00},//U
	{0x1e,0x01,0x1e,0x00},//V
	{0x1f,0x02,0x1f,0x00},//W
	{0x1B,0x04,0x1B,0x00},//X
	{0x18,0x07,0x18,0x00},//Y
	{0x13,0x15,0x19,0x00},//Z
};
void LcdDrawMaskY(u16 x,u16 y,u8 mask,u8 n)
{
	u8 j,c;
	u8  mv=1<<n;
	for(j=0;j<n;j++)
	{
		mv>>=1;
		c=mask&mv;
		LcdDrawPixelXY(x,y+j,LcdBw?!c:c);
	}
}
void LcdDrawMaskX(u16 x,u16 y,u8 mask,u8 n)
{
	u8  j,c;
	u8  mv=1<<n;
	for(j=0;j<n;j++)
	{
		mv>>=1;
		c=mask&mv;
		LcdDrawPixelXY(x+j,y,LcdBw?!c:c);
	}
}
void LcdDrawMiniNum(u16 x,u16 y,u8 num)
{
	int i;
	for(i=0;i<4;i++)
	{
		LcdDrawMaskY(x+i,y,LcdMiniNum[num][i],5);
    }	
}

void LcdDrawMiniEng(u16 x,u16 y,char *eng)
{
	while(*eng)
	{
		char c=*eng;
		if(c>='a' && c<='z') c=c-'a'+'A';
		if(c>='A' && c<='Z')
		{
			LcdDrawMiniNum(x,y,LCD_MN_CHAR(c));
		}
		else if(c>='0' && c<='9')
		{
			LcdDrawMiniNum(x,y,c-'0');
		}
		else if(c==' ')
		{
			LcdDrawMiniNum(x,y,LCD_MN_SP);
		}
		else
		{
			LcdDrawMiniNum(x,y,c);
		}
		
		eng++;
		x+=4;
	}
}

void  LcdDrawMiniInt(u16 x,u16 y,s16 value,u8 n,u8 dot,u8 plus,u8 ar)
{
	//保存符号位
	u8 minus=LCD_MN_SP;
	if(value<0) 
	{
		minus=LCD_MN_MINUS;
		value=-value;
	}
	else if(value>0)
	{
	    if(plus)	minus=LCD_MN_PLUS;
	}
	
	//计算数字位数
	s16 v=value;
	u8 w;
	if(n==0)	for(n=0;v;n++,v/=10);
	if(n<=dot)	n=dot+1;	//如果位数不足小数点，要补上0.xx
	
	//计算宽度
	w=n*4;
	if(dot!=0) w+=2;	//如果有小数点，还要加2位宽度
	if(plus!=0xff)	w+=4;//为整齐，符号位宽度始终预留
	
	//计算掩码
	s16 mask=1;
	u8 i;
	for(i=0;i<n-1;i++) mask*=10;
	
	//如果是右对齐，要重新计算起始位置
	if(ar)	x-=w;
    
    //绘制符号
    if(plus!=0xff)
    {
    	LcdDrawMiniNum(x,y,minus); x+=4;
	}
    
	//开始绘制数字
	u8 sz=0;//是否显示0
	for(i=0;i<n;i++)
	{
		v=value/mask%10;
		if(i==n-1)	sz=1;//如果已经是最后1位，则0也要显示
		if(i==n-1-dot && dot)
		{
			sz=1;//如果已到达小数点，则0也要显示
			LcdDrawMiniNum(x,y,v);			x+=4;
    		LcdDrawMiniNum(x,y,LCD_MN_DOT);	x+=2;
		}
		else
		{
			if(v)	sz=1;	//如果此位非0，后续0都要显示
			else	if(!sz)	v=LCD_MN_SP;//如果是0且未置位sz，则将0替换为空格
			LcdDrawMiniNum(x,y,v);			x+=4;
		} 
		mask/=10;
    }
}
/////////////////////////////////////////////
//
// 画线函数
//
void LcdDrawHLine(u16 x1,u16 x2,u16 y,u16 color)
{
	while(x1<=x2)
	{
		LcdDrawPixelXY(x1++,y,color);
	}
}

void LcdDrawVLine(u16 x,u16 y1,u16 y2,u16 color)
{
	while(y1<=y2)
	{
		LcdDrawPixelXY(x,y1++,color);
	}
}

// bresenham's algorithm - thx wikpedia
void LcdDrawLine(u16 x0, u16 y0, u16 x1, u16 y1, u16 color)
{
	#define swap(x,y) t=x;x=y;y=t;
	u16 t;
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
	swap(x0, y0);
	swap(x1, y1);
	}
	
	if (x0 > x1) {
	swap(x0, x1);
	swap(y0, y1);
	}
	
	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);
	
	int16_t err = dx / 2;
	int16_t ystep;
	
	if (y0 < y1) {
	ystep = 1;
	} else {
	ystep = -1;
	}
	
	for (; x0<=x1; x0++) {
	if (steep) {
	  LcdDrawPixelXY(y0, x0, color);
	} else {
	  LcdDrawPixelXY(x0, y0, color);
	}
	err -= dy;
	if (err < 0) {
	  y0 += ystep;
	  err += dx;
	}
	}
}
void LcdDrawRect(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	while(x1<=x2)
	{
		u16 y;
		for(y=y1;y<=y2;y++)
		{
			LcdDrawPixelXY(x1,y,color);
		}
		x1++;
	}
}

/////////////////////////////////////////////
//
// 图标显示函数
//
void LcdDrawIcon(u16 x,u16 y,u16 w,u16 h,const u8 *icon)
{
	u16 idx=-1,i,j,k;//idx设置为-1，在首次循环时即+1成为0
    
    LcdDrawStart(x, y, x + w-1, y + h-1, DRAW_SWNE);
	for(j=0;j<h;j++)
	{
		for(i=k=0;i<w;i++)//此种写法是为了避免X非8整数
		{
			if(k==0) 
			{
				idx++;
				k=0x80;
			}
			LcdDrawPixel(icon[idx]&k);
			k>>=1;
		}
	}
    LcdDrawStop();
}

/////////////////////////////////////////////
//
// 单色位图文件显示函数
//
void LcdDrawBmpFile(u16 x, u16 y, const char *filename)
{
	u8 bmphead[62];
	FILE *fbmp;
	
	fbmp=fopen(filename,"rb");
	if(!fbmp)
	{
		return;
	}
	setbuf(fbmp,0);	
	if(fread(bmphead,sizeof(bmphead),1,fbmp)!=1 ||
		bmphead[0]!='B' || bmphead[1]!='M' ||
		bmphead[0x1e]!=0 )
	{
		fclose(fbmp);
		return;
	}
	
	u32 offset,img_w,img_h;
    offset = *((u32 *)(bmphead + 0x0a));
    img_w = *((u32 *)(bmphead + 0x12));
    img_h = *((u32 *)(bmphead + 0x16));
    fseek(fbmp, offset, SEEK_SET);  
	
    LcdDrawStart(x, y, x + img_w - 1, y + img_h - 1, DRAW_SWNE);
	
	u32 i,j;
	u8  pix,k;
	
	//单色位图
	if(bmphead[0x1c]==1)
	{
		for(j=0;j<img_h;j++)
		{
			for(i=k=0;i<img_w;i++)//此种写法是为了避免X非8整数
			{
				if(k==0) 
				{
					fread(&pix,1,1,fbmp);
					k=0x80;
				}
				LcdDrawPixel(pix&k);
				k>>=1;
			}
			
			//一行数据一定要是4的倍数
			i=(i+7)/8;
			while(i%4)
			{
				fread(&pix,1,1,fbmp);
				i++;
			}
		}
	} 
	
	//256色位图
	if(bmphead[0x1c]==8)
	{  
		for(j=0;j<img_h;j++)
		{
			for(i=0;i<img_w;i++)
			{
				fread(&pix,1,1,fbmp);
				LcdDrawPixel(pix);
			}
		}
	} 	

    LcdDrawStop();
    fclose(fbmp);
}

/////////////////////////////////////////////
//
// 在LCD上显示汉字
//
void LcdWriteHzLine(u16 x,u16 y,u16 mask)
{
	int j;
	u16  mv=1;
	for(j=0;j<16;j++)
	{
		LcdDrawPixelXY(x,y+j,mask&mv);
		mv<<=1;
	}
}
void LcdWriteHzChar(u16 x,u16 y,uint hz)
{
	u16 i;
	const u8 * Lib;

	//定位到对应字模
	for(i=0;i<HZNUM;i++)
	{
		u16 hzr=HZK[i*26+1];
		hzr<<=8;
		hzr+=HZK[i*26];
		if(hzr==hz)	break;
	}
	Lib=HZK+i*26+2;

	//绘制
	for(i=0;i<12;i++)
	{
		u16 c=Lib[i]|(Lib[i+12]<<8);
		c<<=1;
		if(LcdBw)	c=~c;
		LcdWriteHzLine(x+i,y,c);
	}
}

void LcdWriteEngChar(u16 x,u16 y,u8 c)
{
	u8 i; 
	const u8* Lib=ASCII+(u16)c*6;	

	//绘制上半部分
	LcdSetXy(x,y);
    for(i=0; i<6; i++)
	{
		u16 c=Lib[i]<<4;
		if(LcdBw)	c=~c;
		LcdWriteHzLine(x+i,y,c);
	}
}

u16 LcdDrawText(u16 x,u16 y,char *hz)
{
	while(*hz)
	{
		if(*hz>'~')
		{
			LcdWriteHzChar(x,y,*(u16*)hz);
			x+=12;
			hz+=2;
		}
		else
		{
			LcdWriteEngChar(x,y,*hz);
			x+=6;
			hz+=1;
		}
	}
	return x;
}
