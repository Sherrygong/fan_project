#include "adc.h"
#include "sys.h"
#include "delay.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

u16  AD_Value[2];   //�������ADCת�������Ҳ��DMA��Ŀ���ַ	   
		   
//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//���ǿ���ͨ��   10 ��1��ͨ��																	   
void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;


	//RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC |RCC_APB2Periph_GPIOB |RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��GPIOB,GPIOC,ADC1ͨ��ʱ��
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC |RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��GPIOC,ADC1ͨ��ʱ��
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);        //ʹ��DMA����

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	//PC1 PC2 PC0 PC3 PC4 PC5 ��Ϊģ��ͨ����������                         
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
 // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	//PB0  PB1��Ϊģ��ͨ����������                         
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_0;
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//ģ��ת��������ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 2;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���
	


  ADC_DMACmd(ADC1, ENABLE);   // ����ADC��DMA֧�֣�Ҫʵ��DMA���ܣ������������DMAͨ���Ȳ�����

	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
// 	ADC_RegularChannelConfig(ADC1, 8,  1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_8ͨ��,����ʱ��Ϊ239.5����	 	   
// 	ADC_RegularChannelConfig(ADC1, 9,  2, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_9ͨ��,����ʱ��Ϊ239.5����

	ADC_RegularChannelConfig(ADC1, 10, 1, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_10ͨ��,����ʱ��Ϊ239.5����
	
  ADC_RegularChannelConfig(ADC1, 11, 2, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_11ͨ��,����ʱ��Ϊ239.5����
// 	ADC_RegularChannelConfig(ADC1, 12, 3, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_12ͨ��,����ʱ��Ϊ239.5����
//	ADC_RegularChannelConfig(ADC1, 13, 4, ADC_SampleTime_55Cycles5 );	//ADC1,ADC1_13ͨ��,����ʱ��Ϊ239.5����
// 	ADC_RegularChannelConfig(ADC1, 14, 7, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_14ͨ��,����ʱ��Ϊ239.5����
// 	ADC_RegularChannelConfig(ADC1, 15, 8, ADC_SampleTime_239Cycles5 );	//ADC1,ADC1_15ͨ��,����ʱ��Ϊ239.5����

   	
    DMA_DeInit(DMA1_Channel1);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
    DMA_InitStructure.DMA_PeripheralBaseAddr =  (u32)&ADC1->DR;  //DMA����ADC����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value;  //DMA�ڴ����ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //�ڴ���Ϊ���ݴ����Ŀ�ĵ�
    DMA_InitStructure.DMA_BufferSize = 2;  //DMAͨ����DMA����Ĵ�С
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;	//�رս���һ�����ݺ�Ŀ���ڴ��ַ���
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ������ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ� 
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��
 
   	//ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

}			

//���ADCֵ
//ch:ͨ��ֵ 8 9 10 11 12 13 14 15
//ȡ60��ƽ��ֵ
void Get_Adc(u16 *buff)   
{			    

		DMA_Cmd(DMA1_Channel1, ENABLE);         //����DMAͨ��
		
		ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
		
		while(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=SET);//�ȴ�ת������
		DMA_ClearFlag(DMA1_FLAG_TC1);
//		while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������	
//	  ADC_ClearFlag(ADC1, ADC_FLAG_EOC );
 		*buff = AD_Value[0];	//�������һ��ADC1�������ת�����
	  *(buff+1) = AD_Value[1];
	
}


























