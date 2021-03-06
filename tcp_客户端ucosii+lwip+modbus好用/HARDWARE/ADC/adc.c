#include "adc.h"
#include "sys.h"
#include "delay.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

u16  AD_Value[2];   //用来存放ADC转换结果，也是DMA的目标地址	   
		   
//初始化ADC
//这里我们仅以规则通道为例
//我们开启通道   10 共1个通道																	   
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;


	//RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1	, ENABLE );	  //使能GPIOB,GPIOC,ADC1通道时钟
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1	, ENABLE );	  //使能GPIOC,ADC1通道时钟
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);        //使能DMA传输

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	//PC1 PC2 PC0 PC3 PC4 PC5 作为模拟通道输入引脚                         
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
 // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	//PB0  PB1作为模拟通道输入引脚                         
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//模数转换工作在扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 2;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器
	


  ADC_DMACmd(ADC1, ENABLE);   // 开启ADC的DMA支持（要实现DMA功能，还需独立配置DMA通道等参数）

	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
//设置指定ADC的规则组通道，一个序列，采样时间
// 	ADC_RegularChannelConfig(ADC1, 8,  1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_8通道,采样时间为239.5周期	 	   
// 	ADC_RegularChannelConfig(ADC1, 9,  2, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_9通道,采样时间为239.5周期

	ADC_RegularChannelConfig(ADC1, 10, 1, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_10通道,采样时间为239.5周期
	
  ADC_RegularChannelConfig(ADC1, 11, 2, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_11通道,采样时间为239.5周期
// 	ADC_RegularChannelConfig(ADC1, 12, 3, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_12通道,采样时间为239.5周期
//	ADC_RegularChannelConfig(ADC1, 13, 4, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_13通道,采样时间为239.5周期
// 	ADC_RegularChannelConfig(ADC1, 14, 7, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_14通道,采样时间为239.5周期
// 	ADC_RegularChannelConfig(ADC1, 15, 8, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_15通道,采样时间为239.5周期

   	
    DMA_DeInit(DMA1_Channel1);   //将DMA的通道1寄存器重设为缺省值
    DMA_InitStructure.DMA_PeripheralBaseAddr =  (u32)&ADC1->DR;  //DMA外设ADC基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value;  //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //内存作为数据传输的目的地
    DMA_InitStructure.DMA_BufferSize = 2;  //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;	//关闭接收一次数据后，目标内存地址后�
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道
 
   	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}			

//获得ADC值
//ch:通道值 8 9 10 11 12 13 14 15
//取60次平均值
void Get_Adc(u16 *buff)   
{			    

		DMA_Cmd(DMA1_Channel1, ENABLE);         //启动DMA通道
		
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
		
		while(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=SET);//等待转换结束
		DMA_ClearFlag(DMA1_FLAG_TC1);
//		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束	
//	  ADC_ClearFlag(ADC1, ADC_FLAG_EOC );
 		*buff = AD_Value[0];	//返回最近一次ADC1规则组的转换结果
	  *(buff+1) = AD_Value[1];
	
}


























