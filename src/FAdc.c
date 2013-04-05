#include "Func.h"

///////////////////////////////////////////////////////////////////////////////////////
//
//  ADC数据结构
//
const u8 AdcSeq[ADC_NUM]={12,11,10,13,14,0,4};
volatile u32 AdcBuffer[ADC_NUM][ADC_AVG_NUM*ADC_OVERSAMPLE];
u32 AdcIdx,AdcCnt;

///////////////////////////////////////////////////////////////////////////////////////
//
//  ADC初始化
//
void AdcInit(void)
{    
    //配置模拟通道
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);//PC0 CH10 : STICK RV
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);//PC0 CH11 : STICK LV
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO2);//PC0 CH12 : STICK LH
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO3);//PC0 CH13 : STICK RH
    gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO4);//PC4 CH14 : BATTERY
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO0);//PA0 CH00 : REVERSE
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO4);//PA4 CH04 : REVERSE

	//使能ADC模块                   
    rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_ADC1EN);
    
    //配置ADC时关闭ADC模块
    adc_off(ADC1);
    rcc_peripheral_reset(&RCC_APB2RSTR, RCC_APB2RSTR_ADC1RST);
    rcc_peripheral_clear_reset(&RCC_APB2RSTR, RCC_APB2RSTR_ADC1RST);
    rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV6);
    
    //配置ADC参数
    adc_disable_scan_mode(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_disable_discontinuous_mode_regular(ADC1);
    adc_disable_external_trigger_regular(ADC1);
    adc_set_right_aligned(ADC1);
    
    //打开ADC
    adc_power_on(ADC1);
    usleep(100);
    
    //校准ADC
    adc_reset_calibration(ADC1);
    adc_calibration(ADC1);
    
    //设置扫描顺序
	ADC_SQR1(ADC1) = 0;
	ADC_SQR2(ADC1) = 0;
	ADC_SQR3(ADC1) = AdcSeq[AdcIdx=0];
    
    //打开中断，启动ADC    
    AdcCnt=0;
    adc_start_conversion_direct(ADC1);
}
///////////////////////////////////////////////////////////////////////////////////////
//
//  ADC处理过程
//
void AdcHandler(void)
{
	if(!(adc_eoc(ADC1))) return;
	
	//读取本次转换结果
	AdcBuffer[AdcIdx][AdcCnt]=adc_read_regular(ADC1);	
	
	//启动下一个通道的转换
    if(++AdcIdx>=ADC_NUM) 
    {
    	AdcIdx=0;
    	if(++AdcCnt>=ADC_AVG_NUM * ADC_OVERSAMPLE)
    	{
    		AdcCnt=0;
    	}
    }
	ADC_SQR3(ADC1) = AdcSeq[AdcIdx];
    adc_start_conversion_direct(ADC1);
}

///////////////////////////////////////////////////////////////////////////////////////
//
//  ADC结果计算
//
u16 AdcGetValue(u8 i)
{
	int j;
	u32 result=0;
	for(j=0;j<ADC_AVG_NUM*ADC_OVERSAMPLE;j++)
	{
		result+=AdcBuffer[i][j];
	}
	return result/ADC_AVG_NUM;
}
