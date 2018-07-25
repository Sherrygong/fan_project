#include "tcp_client_demo.h"
#include "lwip/opt.h"
#include "lwip_comm.h"
#include "lwip/lwip_sys.h"
#include "lwip/api.h"
#include "includes.h"
#include "key.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ENC28J60ģ��
//NETCONN API��̷�ʽ��TCP�ͻ��˲��Դ���	   
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
extern u8 send_data;
struct netconn *tcp_clientconn;					//TCP CLIENT�������ӽṹ��
extern u8 arm_is_ok;
u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//TCP�ͻ��˽������ݻ�����
u8 tcp_client_sendbuf[1044];	//TCP�ͻ��˷������ݻ�����
u8 tcp_client_flag;		//TCP�ͻ������ݷ��ͱ�־λ
extern OS_EVENT *mybox;
//TCP�ͻ�������
#define TCPCLIENT_PRIO		8
//�����ջ��С
#define TCPCLIENT_STK_SIZE	300
//�����ջ
OS_STK TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE];

//tcp�ͻ���������
static void tcp_client_thread(void *arg)
{
	OS_CPU_SR cpu_sr;
	u32 data_len = 0;
	struct pbuf *q;
	err_t err,recv_err;
	static ip_addr_t server_ipaddr,loca_ipaddr;
	static u16_t 		 server_port,loca_port;
	unsigned char er;

	LWIP_UNUSED_ARG(arg);
	server_port = REMOTE_PORT;
	IP4_ADDR(&server_ipaddr, lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]);
	
	tcp_client_sendbuf[0]='D';
	tcp_client_sendbuf[1]='A';
	tcp_client_sendbuf[2]='T';
	tcp_client_sendbuf[3]='A';
	tcp_client_sendbuf[4]=0x04;
	tcp_client_sendbuf[5]=0x0E;
	
	while (1) 
	{
			printf("tcp is start \n");
			tcp_clientconn = netconn_new(NETCONN_TCP);  //����һ��TCP����
			err = netconn_connect(tcp_clientconn,&server_ipaddr,server_port);//���ӷ�����
			if(err != ERR_OK)  
			{
				netconn_delete(tcp_clientconn); //����ֵ������ERR_OK,ɾ��tcp_clientconn����
				printf("tcp connect error %d\n\r", err);
			}
			else if (err == ERR_OK)    //���������ӵ�����
			{ 
				struct netbuf *recvbuf;
				tcp_clientconn->recv_timeout = 10;
				netconn_getaddr(tcp_clientconn,&loca_ipaddr,&loca_port,1); //��ȡ����IP����IP��ַ�Ͷ˿ں�
				printf("�����Ϸ�����%d.%d.%d.%d,�����˿ں�Ϊ:%d\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3],loca_port);
				while(1)
				{
					  OSMboxPend(mybox,0,&er);
						err = netconn_write(tcp_clientconn ,tcp_client_sendbuf,1044,NETCONN_COPY); //����tcp_server_sentbuf�е�����
						if(err != ERR_OK)
						{
							printf("����ʧ��\r\n");
							netconn_close(tcp_clientconn);
							netconn_delete(tcp_clientconn);
							printf("������%d.%d.%d.%d�Ͽ�����\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]);
							break;
						}
					//memset(tcp_client_sendbuf+6,0,38);					
					if((recv_err = netconn_recv(tcp_clientconn,&recvbuf)) == ERR_OK)  //���յ�����
					{	
						OS_ENTER_CRITICAL(); //���ж�
						memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
						for(q=recvbuf->p;q!=NULL;q=q->next)  //����������pbuf����
						{
							//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
							//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
							if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
							else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
							data_len += q->len;  	
							if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
						}
						OS_EXIT_CRITICAL();  //���ж�
						data_len=0;  //������ɺ�data_lenҪ���㡣					
						printf("%s\r\n",tcp_client_recvbuf);
						netbuf_delete(recvbuf);
					}else if(recv_err == ERR_CLSD)  //�ر�����
					{
						netconn_close(tcp_clientconn);
						netconn_delete(tcp_clientconn);
						printf("������%d.%d.%d.%d�Ͽ�����\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]);
						break;
					}
					if(recv_err == ERR_CLSD)  //�ر�����
					{
						netconn_close(tcp_clientconn);
						netconn_delete(tcp_clientconn);
						printf("������%d.%d.%d.%d�Ͽ�����\r\n",lwipdev.remoteip[0],lwipdev.remoteip[1], lwipdev.remoteip[2],lwipdev.remoteip[3]);
						break;
					}
					//OSTimeDlyHMSM(0,0,0,10); 
				}
				printf("dddddddddddddddd");
			}
		else
		{
			printf("tcp is sleep \r\n");
			OSTimeDlyHMSM(0,0,0,100);
		}
	}
}

//����TCP�ͻ����߳�
//����ֵ:0 TCP�ͻ��˴����ɹ�
//		���� TCP�ͻ��˴���ʧ��
INT8U tcp_client_init(void)
{
	INT8U res;
	OS_CPU_SR cpu_sr;
	
	OS_ENTER_CRITICAL();	//���ж�
	res = OSTaskCreate(tcp_client_thread,(void*)0,(OS_STK*)&TCPCLIENT_TASK_STK[TCPCLIENT_STK_SIZE-1],TCPCLIENT_PRIO); //����TCP�ͻ����߳�
	OS_EXIT_CRITICAL();		//���ж�
	
	return res;
}

