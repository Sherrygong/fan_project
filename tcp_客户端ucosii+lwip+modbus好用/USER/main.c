#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	 
#include "usmart.h"
#include "sram.h"
#include "malloc.h"
#include "enc28j60.h" 	 
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "wdg.h"
#include "timer.h"
#include "includes.h"
#include "tcp_client_demo.h"
#include "master.h"
#include "adc.h"

/************************************************
 ALIENTEK��ӢSTM32����������ʵ��
 ����NETCONN API��TCP�ͻ���ʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
//MODBUS����
//�������ȼ�
#define MDBS_TASK_PRIO		7
//�����ջ��С
#define MDBS_STK_SIZE		128
//�����ջ
OS_STK	MDBS_TASK_STK[MDBS_STK_SIZE];
//������
void modbus_task(void *pdata); 

//TCP/IP���ȼ�Ϊ          8

//IWDG����
#define IWDG_TASK_PRIO 		9
//�����ջ��С
#define IWDG_STK_SIZE		64	
//�����ջ
OS_STK IWDG_TASK_STK[IWDG_STK_SIZE];
//������
void iwdg_task(void *pdata);   

//LED����
//�������ȼ�
#define LED_TASK_PRIO		10
//�����ջ��С
#define LED_STK_SIZE		64
//�����ջ
OS_STK	LED_TASK_STK[LED_STK_SIZE];
//������
void led_task(void *pdata);   

//ADC����
//�������ȼ�
#define ADC_TASK_PRIO		6
//�����ջ��С
#define ADC_STK_SIZE		500
//�����ջ
OS_STK	ADC_TASK_STK[ADC_STK_SIZE];
//������
void adc_task(void *pdata);  

//START����
//�������ȼ�
#define START_TASK_PRIO		12
//�����ջ��С
#define START_STK_SIZE		64
//�����ջ
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata); 

u8 modbus_com2_over;
u8 slaveraddr;	
u8 value[38];
u8 send_data;
u8 arm_is_ok = 1;
u8 spi_is_ok = 1;
u16 ain_buff[2];
extern u8 tcp_client_sendbuf[];
OS_EVENT *mybox;  //��Ϣ����

int main(void)
{	 
	delay_init();	    	 	//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart1_init(115200);	 		//���ڳ�ʼ��Ϊ115200
	uart2_init(38400);
	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	Adc_Init();
	
	OSInit();				//UCOS��ʼ��
	while(lwip_comm_init()) //lwip��ʼ��
	{
		printf("��ʼ��ʧ��\n\r");
		delay_ms(500);
		delay_ms(500);
	}
	printf("Lwip Init Success!"); 		//lwip��ʼ���ɹ�
	//����TCP
	while(tcp_client_init()) 									//��ʼ��tcp_client(����tcp_client�߳�)
	{
		printf("tcp client fail\n\r");
		delay_ms(500);
	}
	printf("TCP Client Success!"); 			//tcp�����ɹ�	
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //����UCOS
}

//start����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	IWDG_Init(6,625); 	// �������Ź� 4S   4 1s 
	//IWDG_Init(2,625); 	// �������Ź� 625MS
	OSStatInit();  			//��ʼ��ͳ������
	OS_ENTER_CRITICAL();  	//���ж�
	OSTaskCreate(led_task,(void*)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO); 	//����LED����
	OSTaskCreate(iwdg_task,(void*)0,(OS_STK*)&IWDG_TASK_STK[IWDG_STK_SIZE-1],IWDG_TASK_PRIO); 	//����IWDG����
	OSTaskCreate(modbus_task,(void*)0,(OS_STK*)&MDBS_TASK_STK[MDBS_STK_SIZE-1],MDBS_TASK_PRIO); //����Modbus����
	OSTaskCreate(adc_task,(void*)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO); //����ADC����
	
	OSTaskSuspend(OS_PRIO_SELF); //����start_task����
	OS_EXIT_CRITICAL();  //���ж�
}



//IWDG����
void iwdg_task(void *pdata)
{
	while(1)
	{
		IWDG_Feed();
		OSTimeDlyHMSM(0,0,3,500);  //��ʱ3500ms
		//printf("feed\n\r\r\n");
	}
}

//led����
void led_task(void *pdata)
{
	mybox = OSMboxCreate(0);
	while(1)
	{
		OSMboxPost(mybox,(void *)1);
		LED0 = !LED0;
		OSTimeDlyHMSM(0,0,0,500);  //��ʱ500ms
		
 	}
}
//modbus����
void modbus_task(void *pdata)
{
	while(1)
	{
			OS_CPU_SR cpu_sr;
			OS_ENTER_CRITICAL(); 
			modbus_rtu();
			OS_EXIT_CRITICAL();  //���ж�
			OSTimeDlyHMSM(0,0,0,100);
	} 

}


void adc_task(void *pdata)
{
	u16 m=0;
	while(1)
	{
		Get_Adc(ain_buff);	
		tcp_client_sendbuf[2*m+44] = ain_buff[0]>>8;
		tcp_client_sendbuf[2*m+45] = ain_buff[0]&0xFF;	
		tcp_client_sendbuf[2*m+544] = ain_buff[1]>>8;
		tcp_client_sendbuf[2*m+545] = ain_buff[1]&0xFF;
		m++;
		if(m==250)
		{
			m=0;
			OSTimeDlyHMSM(0,0,0,500);
		}
	}
}




