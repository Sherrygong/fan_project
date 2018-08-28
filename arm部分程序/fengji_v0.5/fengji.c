/**
 * @file fengji.c
 * @brief imx6ul处理器
 * @author Dong Wenhao
 * @version 1.0
 * @date 2018/1/9
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "debug.h"
#include "spi_heart.h"
#include "modbus_demo.h"
 
//预定义
#define DATA_SIZE 44
 
//函数声明
int start_thread_func(void*(*func)(void*), pthread_t* pthread, void* par, int* COM_STATU);//创建线程

//全局变量声明
unsigned char dataBuf[DATA_SIZE];
pthread_mutex_t mut;//互斥锁
volatile	pthread_t thread[3]; //线程声明
int dataChanged = 0;

const int SPI_THREAD_ID = 0;  //SPI线程ID
const int MODBUS_THREAD_ID = 1;  //ModBus线程ID
const int TCP_THREAD_ID = 2;  //TCP线程ID
//线程状态标志
volatile	int COM_SPI_STATU = 1;
volatile	int COM_MODBUS_STATU = 1; 
volatile	int COM_TCP_STATU = 1; 

/** 
 * @brief start_thread_func创建线程函数
 * 
 * @param argc 参数个数
 * @param argv[] 参数
 * 
 * @return 0 成功
 */
int start_thread_func(void*(*func)(void*), pthread_t* pthread, void* par, int* COM_STATU)  
{  
	*COM_STATU = 1;  
	memset(pthread, 0, sizeof(pthread_t));  
	int temp;  
		/*creat thread*/  
	if((temp = pthread_create(pthread, NULL, func, par)) != 0)  
	{
		printf("creat thread failer!\n");
	}  
	else  
	{  
		int id = pthread_self();  
		printf("%d  creat thread %lu sucess\n", id, *pthread);  
	}  
	return temp;  
} 

/** 
 * @brief spi_send spi发送函数
 * 
 * @主要功能是通过spi向stm32发送ok信息，代表arm正常工作
 * @param argc 参数个数
 * @param argv[] 参数
 * 
 * @return
 */
 
 void* spi_send(void* pstatu)  
{
	#if DEBUG
	printf("spi thread ok\n");
	#endif
	spi_heart_thread();
    pthread_exit(NULL);
}
/** 
 * @brief modbus_com modbus通讯函数
 * 
 * @主要功能是通过ModBus协议取数据
 * @param argc 参数个数
 * @param argv[] 参数
 * 
 * @return
 */
 void* modbus_cmd(void* pstatu)  
{
	#if DEBUG
	printf("modub thread ok\n");
	#endif
	modbus_rtu_thread();
    pthread_exit(NULL);
}

 void* tcp_con(void* pstatu)  
{
	#if DEBUG
	printf("tcp thread ok\n");
	#endif
	tcp_client_thread();
    pthread_exit(NULL);
}

/** 
 * @brief Main 主函数
 * 
 * @param argc 参数个数
 * @param argv[] 参数
 * 
 * @return 
 */
int main(int argc, char *argv[])
{
	dataBuf[0] = 'D';
	dataBuf[1] = 'A';
	dataBuf[2] = 'T';
	dataBuf[3] = 'A';
	dataBuf[4] = 0x00;
	dataBuf[5] = 0x26;
	
	printf("\nWelcome to fengji client\n\n");
	pthread_mutex_init(&mut,NULL);
	//创建SPI线程
	if(start_thread_func(spi_send, (pthread_t*)&thread[SPI_THREAD_ID],  (int *)&COM_SPI_STATU, (int *)&COM_SPI_STATU) != 0)  
	{  
		printf("error to leave\n");  
		return -1;  
	}
	//创建ModBus线程
	if(start_thread_func(modbus_cmd, (pthread_t*)&thread[MODBUS_THREAD_ID], (int *)&COM_MODBUS_STATU, (int *)&COM_MODBUS_STATU) != 0)
	{  
		printf("error to leave\n");  
		return -1;  
	}  
	//创建TCP线程
	if(start_thread_func(tcp_con, (pthread_t*)&thread[TCP_THREAD_ID], (int *)&COM_TCP_STATU, (int *)&COM_TCP_STATU) != 0)
	{  
		printf("error to leave\n");  
		return -1;  
	}  
  	while(1)
  	{ }
	return 0;
}
