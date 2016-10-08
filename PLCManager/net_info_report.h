#ifndef NET_INFO_REPORT_H_INCLUDED
#define NET_INFO_REPORT_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#define NUM_MAX_NODES            10
#define NUM_MAX_BLACK_NODES      10
#define EXT_ADDRESS_SIZE         8
#define INVALID_NODE_ADDRESS     0xFFFF

#define LE16_TO_CPU_ENDIAN(x)   ((((uint16_t)x & 0x00FF) << 8) + (((uint16_t)x & 0xFF00) >> 8))

typedef struct dlms_report_info {
	char pc_dev_sn[20];
	char pc_operation[10];
	char pc_obis[20];
	int i_class_id;
	int i_element_id;
	char pc_result[30];
} dlms_report_info_t;

typedef struct x_node_list{
	uint16_t us_short_address;
	uint8_t puc_extended_address[EXT_ADDRESS_SIZE];
} x_node_list_t;

struct THopDescriptor {
  uint16_t m_u16HopAddress;
  uint8_t m_u8Mns;
  uint8_t m_u8LinkCost;
};

typedef struct x_path_info {
	uint8_t m_u8Status;
	uint16_t m_u16DstAddr;
	uint16_t m_u16OrigAddr;
	uint8_t m_u8MetricType;
	uint8_t m_u8ForwardHopsCount;
	uint8_t m_u8ReverseHopsCount;
	struct THopDescriptor m_aForwardPath[16];
	struct THopDescriptor m_aReversePath[16];
} x_path_info_t;

typedef struct x_routes_info {
	bool b_path_is_valid[NUM_MAX_NODES];
	uint8_t puc_ext_addr[NUM_MAX_NODES][EXT_ADDRESS_SIZE];
	x_path_info_t x_path_info[NUM_MAX_NODES];
} x_routes_info_t;

typedef struct x_net_info {
	uint16_t us_num_nodes;
	uint16_t us_black_nodes;
	uint16_t us_num_path_nodes;
	uint8_t puc_extended_addr[EXT_ADDRESS_SIZE];
	uint8_t puc_black_list[NUM_MAX_BLACK_NODES][EXT_ADDRESS_SIZE];
	x_node_list_t x_node_list[NUM_MAX_NODES];
} x_net_info_t;



void net_info_report_init(void);
void net_info_report_start_cycle(void);
void net_info_report_end_cycle(void);
void net_info_report_node_cycle(dlms_report_info_t *x_node_info);

int net_info_report_xml_init(void);
void net_info_report_xml_end(int xml_fd);
void net_info_report_xml_add_node(int xml_fd, uint16_t u16Addr, uint8_t *pu8Addr);

int net_info_report_pathlist(x_net_info_t *net_info, x_routes_info_t *routes_info);
int net_info_report_netlist(x_net_info_t *net_info);
int net_info_report_blacklist(x_net_info_t *net_info);
int net_info_report_path_info(uint16_t us_node_addr, uint8_t *puc_ext_addr, x_path_info_t *path_info);

#endif /* NET_INFO_REPORT_H_INCLUDED */
