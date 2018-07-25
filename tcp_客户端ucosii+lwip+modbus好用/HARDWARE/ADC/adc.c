#include "adc.h"
#include "sys.h"
#include "delay.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

u16  AD_Value[2];   //ÓÃÀ´´æ·ÅADC×ª»»½á¹û£¬Ò²ÊÇDMAµÄÄ¿±êµØÖ·	   
		   
//³õÊ¼»¯ADC
//ÕâÀïÎÒÃÇ½öÒÔ¹æÔòÍ¨µÀÎªÀı
//ÎÒÃÇ¿ªÆôÍ¨µÀ   10 ¹²1¸öÍ¨µÀ																	   
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;


	//RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1	, ENABLE );	  //Ê¹ÄÜGPIOB,GPIOC,ADC1Í¨µÀÊ±ÖÓ
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1	, ENABLE );	  //Ê¹ÄÜGPIOC,ADC1Í¨µÀÊ±ÖÓ
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);        //Ê¹ÄÜDMA´«Êä

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //ÉèÖÃADC·ÖÆµÒò×Ó6 72M/6=12,ADC×î´óÊ±¼ä²»ÄÜ³¬¹ı14M

	//PC1 PC2 PC0 PC3 PC4 PC5 ×÷ÎªÄ£ÄâÍ¨µÀÊäÈëÒı½Å                         
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
 // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//Ä£ÄâÊäÈëÒı½Å
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	//PB0  PB1×÷ÎªÄ£ÄâÍ¨µÀÊäÈëÒı½Å                         
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//Ä£ÄâÊäÈëÒı½Å
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //¸´Î»ADC1,½«ÍâÉè ADC1 µÄÈ«²¿¼Ä´æÆ÷ÖØÉèÎªÈ±Ê¡Öµ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC¹¤×÷Ä£Ê½:ADC1ºÍADC2¹¤×÷ÔÚ¶ÀÁ¢Ä£Ê½
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//Ä£Êı×ª»»¹¤×÷ÔÚÉ¨ÃèÄ£Ê½
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//Ä£Êı×ª»»¹¤×÷ÔÚµ¥´Î×ª»»Ä£Ê½
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//×ª»»ÓÉÈí¼ş¶ø²»ÊÇÍâ²¿´¥·¢Æô¶¯
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADCÊı¾İÓÒ¶ÔÆë
	ADC_InitStructure.ADC_NbrOfChannel = 2;	//Ë³Ğò½øĞĞ¹æÔò×ª»»µÄADCÍ¨µÀµÄÊıÄ¿
	ADC_Init(ADC1, &ADC_InitStructure);	//¸ù¾İADC_InitStructÖĞÖ¸¶¨µÄ²ÎÊı³õÊ¼»¯ÍâÉèADCxµÄ¼Ä´æÆ÷
	


  ADC_DMACmd(ADC1, ENABLE);   // ¿ªÆôADCµÄDMAÖ§³Ö£¨ÒªÊµÏÖDMA¹¦ÄÜ£¬»¹Ğè¶ÀÁ¢ÅäÖÃDMAÍ¨µÀµÈ²ÎÊı£©

	ADC_Cmd(ADC1, ENABLE);	//Ê¹ÄÜÖ¸¶¨µÄADC1
	
	ADC_ResetCalibration(ADC1);	//Ê¹ÄÜ¸´Î»Ğ£×¼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//µÈ´ı¸´Î»Ğ£×¼½áÊø
	
	ADC_StartCalibration(ADC1);	 //¿ªÆôADĞ£×¼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //µÈ´ıĞ£×¼½áÊø
//ÉèÖÃÖ¸¶¨ADCµÄ¹æÔò×éÍ¨µÀ£¬Ò»¸öĞòÁĞ£¬²ÉÑùÊ±¼ä
// 	ADC_RegularChannelConfig(ADC1, 8,  1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_8Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ	 	   
// 	ADC_RegularChannelConfig(ADC1, 9,  2, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_9Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ

	ADC_RegularChannelConfig(ADC1, 10, 1, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_10Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ
	
  ADC_RegularChannelConfig(ADC1, 11, 2, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_11Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ
// 	ADC_RegularChannelConfig(ADC1, 12, 3, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_12Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ
//	ADC_RegularChannelConfig(ADC1, 13, 4, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_13Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ
// 	ADC_RegularChannelConfig(ADC1, 14, 7, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_14Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ
// 	ADC_RegularChannelConfig(ADC1, 15, 8, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_15Í¨µÀ,²ÉÑùÊ±¼äÎª239.5ÖÜÆÚ

   	
    DMA_DeInit(DMA1_Channel1);   //½«DMAµÄÍ¨µÀ1¼Ä´æÆ÷ÖØÉèÎªÈ±Ê¡Öµ
    DMA_InitStructure.DMA_PeripheralBaseAddr =  (u32)&ADC1->DR;  //DMAÍâÉèADC»ùµØÖ·
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value;  //DMAÄÚ´æ»ùµØÖ·
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //ÄÚ´æ×÷ÎªÊı¾İ´«ÊäµÄÄ¿µÄµØ
    DMA_InitStructure.DMA_BufferSize = 2;  //DMAÍ¨µÀµÄDMA»º´æµÄ´óĞ¡
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //ÍâÉèµØÖ·¼Ä´æÆ÷²»±ä
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //ÄÚ´æµØÖ·¼Ä´æÆ÷µİÔö
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;	//¹Ø±Õ½ÓÊÕÒ»´ÎÊı¾İºó£¬Ä¿±êÄÚ´æµØÖ·ºóÒ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //Êı¾İ¿í¶ÈÎª16Î»
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //Êı¾İ¿í¶ÈÎª16Î»
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //¹¤×÷ÔÚÑ­»·»º´æÄ£Ê½
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAÍ¨µÀ xÓµÓĞ¸ßÓÅÏÈ¼¶ 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAÍ¨µÀxÃ»ÓĞÉèÖÃÎªÄÚ´æµ½ÄÚ´æ´«Êä
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //¸ù¾İDMA_InitStructÖĞÖ¸¶¨µÄ²ÎÊı³õÊ¼»¯DMAµÄÍ¨µÀ
 
   	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//Ê¹ÄÜÖ¸¶¨µÄADC1µÄÈí¼ş×ª»»Æô¶¯¹¦ÄÜ

}			

//»ñµÃADCÖµ
//ch:Í¨µÀÖµ 8 9 10 11 12 13 14 15
//È¡60´ÎÆ½¾ùÖµ
void Get_Adc(u16 *buff)   
{			    

		DMA_Cmd(DMA1_Channel1, ENABLE);         //Æô¶¯DMAÍ¨µÀ
		
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//Ê¹ÄÜÖ¸¶¨µÄADC1µÄÈí¼ş×ª»»Æô¶¯¹¦ÄÜ	
		
		while(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=SET);//µÈ´ı×ª»»½áÊø
		DMA_ClearFlag(DMA1_FLAG_TC1);
//		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//µÈ´ı×ª»»½áÊø	
//	  ADC_ClearFlag(ADC1, ADC_FLAG_EOC );
 		*buff = AD_Value[0];	//·µ»Ø×î½üÒ»´ÎADC1¹æÔò×éµÄ×ª»»½á¹û
	  *(buff+1) = AD_Value[1];
	
}


























