#include "udp_demo.h"
#include "lwip_comm.h"
#include "usart.h"
#include "led.h"
#include "includes.h"
#include "lwip/api.h"
#include "lwip/lwip_sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ENC28J60ģ��
//NETCONN API��̷�ʽ��UDP���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/4/30
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   
 
//TCP�ͻ�������
#define UDP_PRIO		6
//�����ջ��С
#define UDP_STK_SIZE	300
//�����ջ
OS_STK UDP_TASK_STK[UDP_STK_SIZE];


u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP�������ݻ�����
//UDP������������
//const u8 udp_demo_sendbuf[44];
const u8 *udp_demo_sendbuf="hello udp\r\n";
u8 udp_flag;							//UDP���ݷ��ͱ�־λ
extern u8 send_data;

//udp������
static void udp_thread(void *arg)
{
	OS_CPU_SR cpu_sr;
	err_t err;
	static struct netconn *udpconn;
	static struct netbuf  *recvbuf;
	static struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	u32 data_len = 0;
	struct pbuf *q;
	
	LWIP_UNUSED_ARG(arg);
	udpconn = netconn_new(NETCONN_UDP);  //����һ��UDP����
	udpconn->recv_timeout = 10;  		
	
	if(udpconn != NULL)  //����UDP���ӳɹ�
	{
		err = netconn_bind(udpconn,IP_ADDR_ANY,UDP_DEMO_PORT); 
		IP4_ADDR(&destipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]); //����Ŀ��IP��ַ
		netconn_connect(udpconn,&destipaddr,UDP_DEMO_PORT); 	//���ӵ�Զ������
		if(err == ERR_OK)//�����
		{
			while(1)
			{
				if(send_data) //������Ҫ����
				{
					sentbuf = netbuf_new();
					netbuf_alloc(sentbuf,strlen((char *)udp_demo_sendbuf));
					sentbuf->p->payload = (char*)udp_demo_sendbuf;   	//ָudp_demo_sendbuf��
					err = netconn_send(udpconn,sentbuf);  	//��netbuf�е����ݷ��ͳ�ȥ
					if(err != ERR_OK)
					{
						printf("����ʧ��\r\n");
						netbuf_delete(sentbuf);      //ɾ��buf
					}
					send_data = 0;	//������ݷ��ͱ�־
					netbuf_delete(sentbuf);      	//ɾ��buf
				}	
				
				netconn_recv(udpconn,&recvbuf); //��������
				if(recvbuf != NULL)          //���յ�����
				{ 
					OS_ENTER_CRITICAL(); //���ж�
					memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
					for(q=recvbuf->p;q!=NULL;q=q->next)  //����������pbuf����
					{
						//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
						//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
						if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//��������
						else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
						data_len += q->len;  	
						if(data_len > UDP_DEMO_RX_BUFSIZE) break; //����UDP��������,����	
					}
					OS_EXIT_CRITICAL();  //���ж�
					data_len=0;  //������ɺ�data_lenҪ���㡣
					printf("%s\r\n",udp_demo_recvbuf);  //��ӡ���յ�������
					netbuf_delete(recvbuf);      //ɾ��buf
				}else OSTimeDlyHMSM(0,0,0,5);  //��ʱ5ms
			}
		}else printf("UDP��ʧ��\r\n");
	}else printf("UDP���Ӵ���ʧ��\r\n");
}


//����UDP�߳�
//����ֵ:0 UDP�����ɹ�
//		���� UDP����ʧ��
INT8U udp_demo_init(void)
{
	INT8U res;
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();	//���ж�
	res = OSTaskCreate(udp_thread,(void*)0,(OS_STK*)&UDP_TASK_STK[UDP_STK_SIZE-1],UDP_PRIO); //����UDP�߳�
	OS_EXIT_CRITICAL();		//���ж�
	
	return res;
}

