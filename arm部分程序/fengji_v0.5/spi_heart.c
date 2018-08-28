/* SPI testing utility (using spidev driver) 
 * 
 * Copyright (c) 2007  MontaVista Software, Inc. 
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com> 
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License. 
 * 
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include 
 */  
  
#include <stdint.h>  
#include <unistd.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <getopt.h>  
#include <fcntl.h>  
#include <sys/ioctl.h>  
#include <linux/types.h>  
#include <linux/spi/spidev.h>  
  
#include "debug.h"
  
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))  
  
static void pabort(const char *s)  
{  
    perror(s);  
    abort();  
}  
   
static char device[20] = {0};
static uint8_t mode=1;  
static uint8_t bits = 8;  
static uint32_t speed = 500000;  
static uint16_t delay;
  
static void transfer(int fd)  
{  
    int ret;  
    uint8_t tx[] = {  
       0x55,
    };  
    uint8_t rx[ARRAY_SIZE(tx)] = {0, };  
    struct spi_ioc_transfer tr = {  
        .tx_buf = (unsigned long)tx,  
        .rx_buf = (unsigned long)rx,  
        .len = ARRAY_SIZE(tx),  
        .delay_usecs = delay,  
        .speed_hz = speed,  
        .bits_per_word = bits,  
    };  
  
    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);  
    if (ret < 1)  
        pabort("can't send spi message");  
    for(ret = 0;ret <ARRAY_SIZE(tx);ret++) {
        //printf("%.2X",rx[ret]);
        printf("SPI ok");
    }
    printf("\n");
    puts("");
	sleep(1);
	//usleep(100000);
	#if DEBUG
		//printf("spi ok\n");
		//sleep(1);
	#endif
}  
  
  
int spi_heart_thread()
{  
    int ret = 0;  
    int fd;  
   
    sprintf(device, "/dev/spidev0.0");
  
    fd = open(device, O_RDWR);  
    if (fd < 0)  
        pabort("can't open device");  
  
    /* 
     * spi mode 
     */  
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);  
    if (ret == -1)  
        pabort("can't set spi mode");  
  
    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);  
    if (ret == -1)  
        pabort("can't get spi mode");  
  
    /* 
     * bits per word 
     */  
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);  
    if (ret == -1)  
        pabort("can't set bits per word");  
  
    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);  
    if (ret == -1)  
        pabort("can't get bits per word");  
  
    /* 
     * max speed hz 
     */  
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);  
    if (ret == -1)  
        pabort("can't set max speed hz");  
  
    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);  
    if (ret == -1)  
        pabort("can't get max speed hz");  
  
    printf("spi mode: %d\n", mode);  
    printf("bits per word: %d\n", bits);  
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);  
    while(1) 
    {
      transfer(fd);  
    }
  
    close(fd);  
  
    return ret;  
}
