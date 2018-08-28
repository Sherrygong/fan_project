#ifndef _GPIO_H__
#define _GPIO_H__
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define GPIO_U_IOCTL_BASE 'x'
#define GPIOC_OPS   _IOWR(GPIO_U_IOCTL_BASE,0,int)

#define GPIO_SET(no,state) 	( no | (state << 31))
#define GPIO_GET(val)		(val >> 31)

extern int gpio;
void gpio_set_value(int fd,int gpio_no,int state);
void receEnable();
void sendEnable();
#endif
