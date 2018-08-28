#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>  
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#include "debug.h"

#define  SER_PORT  			9779
#define  SER_ADDR  			"192.168.0.10"

extern unsigned char dataBuf[];
extern int dataChanged;
extern volatile int COM_TCP_STATU;
extern pthread_mutex_t mut;
int tcp_client_thread()
{
	int sockfd, rvl;
	struct sockaddr_in  clientaddr;
	struct sockaddr_in  serveraddr;

	memset(&serveraddr,'\0',sizeof(serveraddr));

	signal(SIGPIPE, SIG_IGN);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SER_PORT);
	serveraddr.sin_addr.s_addr = inet_addr(SER_ADDR);
	
    while(COM_TCP_STATU) 
	{
		if (( sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			perror("socket error");
            close(sockfd);
			usleep(10000);
			continue;
		}
		else {
			printf("socket ok\n");
		}
		if (connect(sockfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0){
			perror(" connnect error");
            close(sockfd);
			usleep(10000);
		} else {
			printf("connect ok\n");
			while(COM_TCP_STATU){
                printf("tcp thread ok\n");
				//if(dataChanged)
                if(1)
				{
					pthread_mutex_lock(&mut);
                    printf("lock\n");
					if ( (rvl = send(sockfd, dataBuf, 44, 0)) < 0){
						#if DEBUG
						#endif
						perror("send error");
						pthread_mutex_unlock(&mut);
                        printf("unlock\n");
						close(sockfd);
						break;
					}
                    printf("tcp ok\n");
					
					pthread_mutex_unlock(&mut);
                    printf("unlock\n");
				}
				usleep(500000);
			}
		}
	}
	while(1){
		printf("can't get here\n");
	}
	return 0;
}
