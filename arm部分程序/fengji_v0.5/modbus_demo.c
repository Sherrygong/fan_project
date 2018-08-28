/*************************************************************************
	> File Name: modbus_demo.c
	> Author: 
	> Mail: 
	> Created Time: Sat 06 Jan 2018 10:13:54 PM CHOT
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/time.h>

#include "modbus_private.h"
#include "gpio.h"
#include "debug.h"

extern int dataChanged;
extern unsigned char dataBuf[];
extern volatile	int COM_MODBUS_STATU; 
extern pthread_mutex_t mut;
extern int dataChanged;

unsigned char cmd1[8]={0x01,0x03,0x00,0x00,0x00,0x08,0x44,0x0C};
unsigned char cmd2[8]={0x02,0x03,0x00,0x00,0x00,0x08,0x44,0x3F};
unsigned char cmd3[8]={0x03,0x03,0x00,0x00,0x00,0x03,0x04,0x29};

unsigned char msg[40];

volatile	int fd;
int gpio;
fd_set rd;
int nread,retval;

struct timeval timeout={0,10000};
volatile	pthread_t thread[3]; 
volatile	const int READ_THREAD_ID = 0;  
volatile	const int SEND_THREAD_ID = 1;  
volatile	const int DATA_THREAD_ID = 2; 

volatile	int COM_READ_STATU = 1;  
volatile	int COM_SEND_STATU = 1; 
volatile	int COM_DATA_STATU = 1; 

int modbus_data_change = 0;
int msg_length;

void read_port(void)
{   
    FD_ZERO(&rd);
    FD_SET(fd,&rd);

    retval = select (fd+1,&rd,NULL,NULL,&timeout); 
    switch (retval)
    {
        case 0:
            //printf("no data input within  1s.\n");
            break;
        case -1:
            perror("select");
            break;       
        default:
		pthread_mutex_lock(&mut);
        nread = read(fd,msg,21);
		pthread_mutex_unlock(&mut);
		if(nread>0)
        {
			int tmp = 0;
			if(nread == msg[2] + 5)
			{
                printf("get normal modbus data\n");
			}
			else if (nread < msg[2]+5) {
				//printf("less\n");
				usleep(10000);
		        pthread_mutex_lock(&mut);
				tmp = read(fd,msg+nread,5);
		        pthread_mutex_unlock(&mut);
				//printf("tmp is %d\n",tmp);
			}
			else 
				break;
			msg_length = nread + tmp;
			if(msg_length < 11)
			{
				break;
			}	
			modbus_data_change = 1;
        }
        break;
    }
}
void* com_read(void* pstatu)  
{
	int o;
	while(COM_READ_STATU)
	{
    	read_port();
	}
    pthread_exit(NULL);
}

void* com_send(void* p)  
{
	int ii=0;
	while(COM_SEND_STATU)
	{	
		sendEnable();
		write(fd, cmd1, 8);
		usleep(2000);
		receEnable();
		msleep(100);
		
		sendEnable();
		write(fd, cmd2, 8);
		usleep(2000);
		receEnable();
		msleep(100);
		
		sendEnable();
		write(fd, cmd3, 8);
		usleep(2000);
		receEnable();
		msleep(100);
	
	}
	pthread_exit(NULL);	
}
 
void* proc_data(void* p)
{
	while(COM_DATA_STATU)
	{
		if(modbus_data_change)
		{
			pthread_mutex_lock(&mut);;
            printf("lock\n");
			modbus_data_change = 0;
			if(modbus_rtu_check_integrity(msg,msg_length))
			{
				//printf("msg crc ok\n");
				if(msg[0] == 1)
					rtu_data_anlys(dataBuf,msg,6,16);
				else if(msg[0] == 2)
					rtu_data_anlys(dataBuf,msg,22,16);
				else if(msg[0] == 3)
					rtu_data_anlys(dataBuf,msg,38,6);
			}
			pthread_mutex_unlock(&mut);
            printf("unlock\n");
			msleep(1);
			dataChanged = 1;
		}
		else
		{
			msleep(10);
		}
	}
}



/** 
 * @brief modbus_rtu_thread modbus函数
 * 
 * @param argc 参数个数
 * @param argv[] 参数
 * 
 * @return 
 */
void 
modbus_rtu_thread()
{
    gpio = open("/dev/gpio",O_RDWR);
	char *dev ="/dev/ttymxc1"; 
	//signal(SIGINT,SignHandler);
    	if(dev==NULL)
	{
     		printf("Please input serial device name ,for exmaple /dev/ttymxc2.\n");
     	 	exit(1);
    	}

	fd = OpenDev(dev);

	if (fd>0)
	{}
	else
	{
		printf("Can't Open Serial Port %s \n",dev);
		exit(0);
	}
	
	printf("\nWelcome to TTYtest! Press Ctrl + 'c' to stop.\n\n");

    pthread_mutex_init(&mut,NULL);  
 
	if(start_thread_func(com_read, (pthread_t*)&thread[READ_THREAD_ID],  (int *)&COM_READ_STATU, (int *)&COM_READ_STATU) != 0)  
	{  
		printf("error to leave\n");  
		exit(0);
	}  
  
	if(start_thread_func(com_send, (pthread_t*)&thread[SEND_THREAD_ID], (int *)&COM_SEND_STATU, (int *)&COM_SEND_STATU) != 0)
	{  
		printf("error to leave\n");  
		exit(0);
	}

	if(start_thread_func(proc_data, (pthread_t*)&thread[DATA_THREAD_ID], (int *)&COM_DATA_STATU, (int *)&COM_DATA_STATU) != 0)
	{  
		printf("error to leave\n");  
		exit(0); 
	} 
  	while(1)
  	{ }
}


