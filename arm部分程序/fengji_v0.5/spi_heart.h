#ifndef __SPI_HEART_H_
#define __SPI_HEART_H_

static void pabort(const char *s);
static void transfer(int fd);
int spi_heart_thread();

#endif