#ifndef NET_INFO_REPORT_H_INCLUDED
#define NET_INFO_REPORT_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

//#define NET_REPORT_DEBUG_CONSOLE

#define EXT_ADDRESS_SIZE         8
#define INVALID_NODE_ADDRESS     0xFFFF

#define LE16_TO_CPU_ENDIAN(x)   ((((uint16_t)x & 0x00FF) << 8) + (((uint16_t)x & 0xFF00) >> 8))

//typedef struct x_node_list{
//	uint16_t us_short_address;
//	uint8_t puc_extended_address[EXT_ADDRESS_SIZE];
//} x_node_list_t;

//void net_info_report_init(void);
//void net_info_report_start_cycle(void);
//void net_info_report_end_cycle(void);
//void net_info_report_node_cycle(dlms_report_info_t *x_node_info);
//
//int net_info_report_xml_init(void);
//void net_info_report_xml_end(int xml_fd);
//void net_info_report_xml_add_node(int xml_fd, uint16_t u16Addr, uint8_t *pu8Addr);
//
//int net_info_report_pathlist(x_net_info_t *net_info, x_routes_info_t *routes_info);
//int net_info_report_netlist(x_net_info_t *net_info);
//int net_info_report_blacklist(x_net_info_t *net_info);
//int net_info_report_path_info(uint16_t us_node_addr, uint8_t *puc_ext_addr, x_path_info_t *path_info);
//
//int net_info_report_dev_num(x_net_info_t *net_info);
//int net_info_report_dashboard(x_net_info_t *net_info, x_net_statistics_t *net_stats);
//int net_info_report_round_time(int i_round_time, uint16_t us_short_addr);
//int net_info_report_data_throughput(int i_data_throughput, uint16_t us_short_addr);

#endif /* NET_INFO_REPORT_H_INCLUDED */
