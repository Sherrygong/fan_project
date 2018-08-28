#ifndef __MODBUS_DEMO_H_
#define __MODBUS_DEMO_H_
#include "stdint.h"

void rtu_data_anlys(unsigned char *dst, uint16_t *src, int start, int len);
void modbus_rtu_thread();

#endif
