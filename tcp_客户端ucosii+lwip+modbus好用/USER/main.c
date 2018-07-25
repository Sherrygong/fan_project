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
 ALIENTEK精英STM32开发板网络实验
 基于NETCONN API的TCP客户端实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/
//MODBUS任务
//任务优先级
#define MDBS_TASK_PRIO		7
//任务堆栈大小
#define MDBS_STK_SIZE		128
//任务堆栈
OS_STK	MDBS_TASK_STK[MDBS_STK_SIZE];
//任务函数
void modbus_task(void *pdata); 

//TCP/IP优先级为          8

//IWDG任务
#define IWDG_TASK_PRIO 		9
//任务堆栈大小
#define IWDG_STK_SIZE		64	
//任务堆栈
OS_STK IWDG_TASK_STK[IWDG_STK_SIZE];
//任务函数
void iwdg_task(void *pdata);   

//LED任务
//任务优先级
#define LED_TASK_PRIO		10
//任务堆栈大小
#define LED_STK_SIZE		64
//任务堆栈
OS_STK	LED_TASK_STK[LED_STK_SIZE];
//任务函数
void led_task(void *pdata);   

//ADC任务
//任务优先级
#define ADC_TASK_PRIO		6
//任务堆栈大小
#define ADC_STK_SIZE		500
//任务堆栈
OS_STK	ADC_TASK_STK[ADC_STK_SIZE];
//任务函数
void adc_task(void *pdata);  

//START任务
//任务优先级
#define START_TASK_PRIO		12
//任务堆栈大小
#define START_STK_SIZE		64
//任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata); 

u8 modbus_com2_over;
u8 slaveraddr;	
u8 value[38];
u8 send_data;
u8 arm_is_ok = 1;
u8 spi_is_ok = 1;
u16 ain_buff[2];
extern u8 tcp_client_sendbuf[];
OS_EVENT *mybox;  //消息邮箱

int main(void)
{	 
	delay_init();	    	 	//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart1_init(115200);	 		//串口初始化为115200
	uart2_init(38400);
	LED_Init();		  			//初始化与LED连接的硬件接口
	Adc_Init();
	
	OSInit();				//UCOS初始化
	while(lwip_comm_init()) //lwip初始化
	{
		printf("初始化失败\n\r");
		delay_ms(500);
		delay_ms(500);
	}
	printf("Lwip Init Success!"); 		//lwip初始化成功
	//创建TCP
	while(tcp_client_init()) 									//初始化tcp_client(创建tcp_client线程)
	{
		printf("tcp client fail\n\r");
		delay_ms(500);
	}
	printf("TCP Client Success!"); 			//tcp创建成功	
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO);
	OSStart(); //开启UCOS
}

//start任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr;
	pdata = pdata ;
	IWDG_Init(6,625); 	// 独立看门狗 4S   4 1s 
	//IWDG_Init(2,625); 	// 独立看门狗 625MS
	OSStatInit();  			//初始化统计任务
	OS_ENTER_CRITICAL();  	//关中断
	OSTaskCreate(led_task,(void*)0,(OS_STK*)&LED_TASK_STK[LED_STK_SIZE-1],LED_TASK_PRIO); 	//创建LED任务
	OSTaskCreate(iwdg_task,(void*)0,(OS_STK*)&IWDG_TASK_STK[IWDG_STK_SIZE-1],IWDG_TASK_PRIO); 	//创建IWDG任务
	OSTaskCreate(modbus_task,(void*)0,(OS_STK*)&MDBS_TASK_STK[MDBS_STK_SIZE-1],MDBS_TASK_PRIO); //创建Modbus任务
	OSTaskCreate(adc_task,(void*)0,(OS_STK*)&ADC_TASK_STK[ADC_STK_SIZE-1],ADC_TASK_PRIO); //创建ADC任务
	
	OSTaskSuspend(OS_PRIO_SELF); //挂起start_task任务
	OS_EXIT_CRITICAL();  //开中断
}



//IWDG任务
void iwdg_task(void *pdata)
{
	while(1)
	{
		IWDG_Feed();
		OSTimeDlyHMSM(0,0,3,500);  //延时3500ms
		//printf("feed\n\r\r\n");
	}
}

//led任务
void led_task(void *pdata)
{
	mybox = OSMboxCreate(0);
	while(1)
	{
		OSMboxPost(mybox,(void *)1);
		LED0 = !LED0;
		OSTimeDlyHMSM(0,0,0,500);  //延时500ms
		
 	}
}
//modbus任务
void modbus_task(void *pdata)
{
	while(1)
	{
			OS_CPU_SR cpu_sr;
			OS_ENTER_CRITICAL(); 
			modbus_rtu();
			OS_EXIT_CRITICAL();  //开中断
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




