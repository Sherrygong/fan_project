#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "gpio.h"
#define GPIO_U_IOCTL_BASE 'x'
#define GPIOC_OPS   _IOWR(GPIO_U_IOCTL_BASE,0,int)

#define GPIO_SET(no,state) 	( no | (state << 31))
#define GPIO_GET(val)		(val >> 31)


void gpio_set_value(int fd,int gpio_no,int state)
{
	unsigned long val;
	val = (!!state << 31) | gpio_no;

	if(ioctl(fd,GPIOC_OPS,&val) < 0){
		perror("ioctl");
	}
}

void receEnable()
{
	gpio_set_value(gpio,0,0);
}
void sendEnable()
{
	gpio_set_value(gpio,0,1);
}
