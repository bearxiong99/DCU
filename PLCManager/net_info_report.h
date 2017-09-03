#ifndef NET_INFO_REPORT_H_INCLUDED
#define NET_INFO_REPORT_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

//#define NET_REPORT_DEBUG_CONSOLE

#define LE16_TO_CPU_ENDIAN(x)   ((((uint16_t)x & 0x00FF) << 8) + (((uint16_t)x & 0xFF00) >> 8))

int net_info_report_devlist(x_dev_addr_t *dev_list, uint16_t us_num_nodes);

#endif /* NET_INFO_REPORT_H_INCLUDED */
