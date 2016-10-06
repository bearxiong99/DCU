#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "net_info_report.h"

static char spuc_conf_name[20];
static char spuc_tmp_name[50];
static char spuc_file_name[50];
static char spuc_netroutes_tmp_name[] = "/tmp/netroutes.json";
static char spuc_netroutes_name[] = "/home/DCWeb/public/tables/netroutes.json";

static char spuc_netlist_tmp_name[] = "/tmp/netlist.json";
static char spuc_netlist_name[] = "/home/DCWeb/public/tables/netlist.json";

static char spuc_blacklist_tmp_name[] = "/tmp/blacklist.json";
static char spuc_blacklist_name[] = "/home/DCWeb/public/tables/blacklist.json";

static char spuc_pathinfo_tmp_name[] = "/tmp/pathinfo.json";
static char spuc_pathinfo_name[] = "/home/DCWeb/public/tables/pathinfo.json";

static const char sc_comillas = '"';

static void mov_file(char *_src_path, char *_dst_path)
{
	int src_fd, dst_fd;
	char c;

	src_fd = open(_src_path, O_RDONLY);
	dst_fd = creat(_dst_path, O_RDWR);
	while(read(src_fd, &c, sizeof(c) != 0)){
		write(dst_fd, &c ,sizeof(c));
	}
	close(src_fd);
	close(dst_fd);

	remove(_src_path);
	chmod(_dst_path, 0777);
}

void net_info_report_init(void)
{
	sprintf(spuc_conf_name, "top_name");
}

//void net_info_report_start_cycle(void)
//{
//	int csv_fd;
//	char puc_ln_buf[100];
//	int i_ln_len;
//
//	time_t t = time(NULL);
//	struct tm *tm = localtime(&t);
//
//	sprintf(spuc_tmp_name, "/tmp/%s_%u%02u%02u_%02u%02u%02u.csv", spuc_conf_name,
//				(tm->tm_year + 1900), tm->tm_mon, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec);
//	sprintf(spuc_file_name, "/home/ftp/Cycles/%s_%u%02u%02u_%02u%02u%02u.csv", spuc_conf_name,
//			(tm->tm_year + 1900), tm->tm_mon, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec);
//	csv_fd = open(spuc_tmp_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);
//
//	if (csv_fd != -1) {
//		i_ln_len = sprintf(puc_ln_buf, "starting cycle %s at %s\r\n", spuc_conf_name, asctime(tm));
//		write(csv_fd, puc_ln_buf, i_ln_len);
//		i_ln_len = sprintf(puc_ln_buf, "time;dev_sn;operation;element_id;result\r\n");
//		write(csv_fd, puc_ln_buf, i_ln_len);
//		close(csv_fd);
//	}
//}
//
//
//void net_info_report_node_cycle(dlms_report_info_t *x_node_info)
//{
//	int csv_fd;
//	char puc_ln_buf[100];
//	int i_ln_len;
//
//	time_t t = time(NULL);
//	struct tm *tm = localtime(&t);
//
//	csv_fd = open(spuc_tmp_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);
//
//	if (csv_fd != -1) {
//		i_ln_len = sprintf(puc_ln_buf, "%u/%02u/%02u_%02u:%02u:%02u;%s;%s;%u;%s\r\n",
//				(tm->tm_year + 1900), tm->tm_mon, tm->tm_mday,tm->tm_hour, tm->tm_min, tm->tm_sec,
//				x_node_info->pc_dev_sn, x_node_info->pc_operation, x_node_info->i_element_id, x_node_info->pc_result);
//		write(csv_fd, puc_ln_buf, i_ln_len);
//		close(csv_fd);
//	}
//}
//
//
//void net_info_report_end_cycle(void)
//{
//	int csv_fd, csv_fd2;
//	char puc_ln_buf[100];
//	int i_ln_len;
//	char c;
//
//	time_t t = time(NULL);
//	struct tm *tm = localtime(&t);
//
//	csv_fd = open(spuc_tmp_name, O_RDWR|O_CREAT|O_APPEND, S_IROTH|S_IWOTH|S_IXOTH);
//
//	if (csv_fd != -1) {
//		i_ln_len = sprintf(puc_ln_buf, "ending cycle %s at %s\r\n", spuc_conf_name, asctime(tm));
//		write(csv_fd, puc_ln_buf, i_ln_len);
//		close(csv_fd);
//
//		/* Move temporal file to FTP folder */
//		mov_file(spuc_tmp_name, spuc_file_name);
//	}
//}
//

//int net_info_report_net_topology(x_net_info_t *net_info)
//{
//	int json_fd;
//	char puc_ln_buf[100];
//	int i_ln_len, i_size_fd;
//	uint8_t *ptr_mac;
//	x_node_list_t *px_node;
//	struct TAdpPathDiscoveryConfirm *px_path;
//	uint8_t uc_idx;
//	uint16_t us_path_idx;
//	uint8_t uc_hops;
//	uint8_t puc_ext_addr[24];
//
//	printf ("JSON obj creating...\r\n");
//
//	json_fd = open(spuc_json_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);;
//
//	i_ln_len = sprintf(puc_ln_buf, "{\r\n  %cnodes%c:[\r\n", sc_comillas, sc_comillas);
//	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);
//
//	memset(puc_ext_addr, 0, sizeof(puc_ext_addr));
//	uc_hops = 0;
//
//	/* First Node is Coordinator */
//	ptr_mac = net_info->puc_extended_addr;
//	sprintf(puc_ext_addr, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", *(ptr_mac + 7), *(ptr_mac + 6), *(ptr_mac + 5),
//					*(ptr_mac + 4), *(ptr_mac + 3), *(ptr_mac + 2), *(ptr_mac + 1), *ptr_mac);
//
//	i_ln_len = sprintf(puc_ln_buf, "    {%cu16Addr%c:%c0%c,%cu64Addr%c:%c%s%c,%chops%c:%d},\r\n",
//			sc_comillas, sc_comillas, sc_comillas, sc_comillas,
//			sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas, sc_comillas, sc_comillas, uc_hops);
//	i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
//
//
//	/* List of Device Nodes */
//	for (uc_idx = 0; uc_idx < net_info->us_num_nodes; uc_idx++) {
//
//		px_node = (x_node_list_t *)&net_info->px_node_list[uc_idx];
//		for (us_path_idx = 0; us_path_idx < net_info->us_num_path_nodes; us_path_idx++) {
//			px_path = (struct TAdpPathDiscoveryConfirm *)&net_info->px_path_nodes[us_path_idx];
//			if (px_path->m_u16DstAddr == px_node->us_short_address) {
//				uc_hops = px_path->m_u8ForwardHopsCount;
//				break;
//			}
//		}
//
//		ptr_mac = &px_node->puc_extended_address;
//		sprintf(puc_ext_addr, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", *ptr_mac, *(ptr_mac + 1), *(ptr_mac + 2),
//				*(ptr_mac + 3), *(ptr_mac + 4), *(ptr_mac + 5), *(ptr_mac + 6), *(ptr_mac + 7));
//
//		i_ln_len = sprintf(puc_ln_buf, "    {%cu16Addr%c:%c%d%c,%cu64Addr%c:%c%s%c,%chops%c:%d},\r\n",
//				sc_comillas, sc_comillas, sc_comillas, px_node->us_short_address, sc_comillas,
//				sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas, sc_comillas, sc_comillas, uc_hops);
//
//		if (uc_idx == (net_info->us_num_path_nodes - 1)) {
//			/* Last iteration */
//			i_size_fd += write(json_fd, puc_ln_buf, i_ln_len - 3); /* remove the last semicolon, CR and LF */
//		} else {
//			/* Last iteration */
//			i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
//		}
//	}
//
//	i_ln_len = sprintf(puc_ln_buf, "\r\n  ],\r\n  %clinks%c:[\r\n", sc_comillas, sc_comillas);
//	write(json_fd, puc_ln_buf, i_ln_len);
//
//	/* List Links */
//	for (us_path_idx = 0; us_path_idx < net_info->us_num_path_nodes; us_path_idx++) {
//		uint16_t us_src, us_dst;
//		uint8_t uc_hope_idx;
//
//		px_path = &net_info->px_path_nodes[us_path_idx];
//
//		/* Forward hopes */
//		us_src = 0;
//		for (uc_hope_idx = 0; uc_hope_idx < px_path->m_u8ForwardHopsCount; uc_hope_idx++) {
//			us_dst = px_path->m_aForwardPath[uc_hope_idx].m_u16HopAddress;
//			i_ln_len = sprintf(puc_ln_buf, "    {%csource%c:%c%d%c,%ctarget%c:%c%d%c,%cvalue%c:%d},\r\n",
//					sc_comillas, sc_comillas, sc_comillas, us_src, sc_comillas,
//					sc_comillas, sc_comillas, sc_comillas, us_dst, sc_comillas,
//					sc_comillas, sc_comillas, px_path->m_aForwardPath[uc_hope_idx].m_u8LinkCost);
//			i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
//
//			us_src = us_dst;
//		}
//
//		/* Reverse hopes */
//		us_src = px_path->m_u16OrigAddr;
//		for (uc_hope_idx = 0; uc_hope_idx < px_path->m_u8ReverseHopsCount; uc_hope_idx++) {
//			us_dst = px_path->m_aReversePath[uc_hope_idx].m_u16HopAddress;
//			i_ln_len = sprintf(puc_ln_buf, "    {%csource%c:%c%d%c,%ctarget%c:%c%d%c,%cvalue%c:%d},\r\n",
//					sc_comillas, sc_comillas, sc_comillas, us_src, sc_comillas,
//					sc_comillas, sc_comillas, sc_comillas, us_dst, sc_comillas,
//					sc_comillas, sc_comillas, px_path->m_aForwardPath[uc_hope_idx].m_u8LinkCost);
//
//			if (uc_hope_idx == (px_path->m_u8ReverseHopsCount - 1)) {
//				/* Last iteration */
//				i_size_fd += write(json_fd, puc_ln_buf, i_ln_len - 3); /* remove the last semicolon, CR and LF */
//			} else {
//				/* Last iteration */
//				i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
//			}
//
//			us_src = us_dst;
//		}
//	}
//
//	i_ln_len = sprintf(puc_ln_buf, "\r\n  ]\r\n}");
//	i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
//	close(json_fd);
//
//	/* Move temporal file to WebServer folder */
//	mov_file(spuc_json_tmp_name, spuc_json_name);
//
//	printf ("JSON obj created\r\n");
//
//	return (i_size_fd);
//}

int net_info_report_netlist(x_net_info_t *net_info)
{
	x_node_list_t *px_node;
	uint8_t *ptr_mac;
	uint16_t us_node_idx;
	uint8_t puc_ext_addr[24];
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;

	printf ("NETLIST creating...\r\n");

	json_fd = open(spuc_netlist_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	us_node_idx = 0;

	i_ln_len = sprintf(puc_ln_buf, "[\r\n");
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	for (us_node_idx = 0; us_node_idx < net_info->us_num_nodes; us_node_idx++) {
		i_ln_len = sprintf(puc_ln_buf, "\t{\r\n");
		i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cid%c: %u,\r\n", sc_comillas, sc_comillas, us_node_idx);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		px_node = (x_node_list_t *)&net_info->x_node_list[us_node_idx];

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cshort_addr%c: %c0x%04X%c,\r\n", sc_comillas, sc_comillas, sc_comillas, px_node->us_short_address, sc_comillas);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		ptr_mac = &px_node->puc_extended_address[0];
		sprintf(puc_ext_addr, "0x%02X%02X%02X%02X%02X%02X%02X%02X", *ptr_mac, *(ptr_mac + 1), *(ptr_mac + 2),
				*(ptr_mac + 3), *(ptr_mac + 4), *(ptr_mac + 5), *(ptr_mac + 6), *(ptr_mac + 7));

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cextended_addr%c: %c%s%c\r\n", sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		if (us_node_idx == (net_info->us_num_nodes - 1)) {
			/* last one -> close json object */
			i_ln_len = sprintf(puc_ln_buf, "\t}\r\n]");
		} else {
			i_ln_len = sprintf(puc_ln_buf, "\t},\r\n");
		}
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

	}

	close(json_fd);

	/* Move temporal file to WebServer folder */
	mov_file(spuc_netlist_tmp_name, spuc_netlist_name);

	printf ("NETLIST created\r\n");

	return (i_size_fd);
}

int net_info_report_blacklist(x_net_info_t *net_info)
{
	x_node_list_t *px_node;
	uint8_t *ptr_mac;
	uint16_t us_node_idx;
	uint8_t puc_ext_addr[24];
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;

	printf ("BLACKLIST creating...\r\n");

	json_fd = open(spuc_blacklist_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	us_node_idx = 0;

	i_ln_len = sprintf(puc_ln_buf, "[\r\n");
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	for (us_node_idx = 0; us_node_idx < net_info->us_black_nodes; us_node_idx++) {
		i_ln_len = sprintf(puc_ln_buf, "\t{\r\n");
		i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cid%c: %u,\r\n", sc_comillas, sc_comillas, us_node_idx);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		ptr_mac = (uint8_t *)&net_info->puc_black_list[us_node_idx][0];

		sprintf(puc_ext_addr, "0x%02X%02X%02X%02X%02X%02X%02X%02X", *ptr_mac, *(ptr_mac + 1), *(ptr_mac + 2),
				*(ptr_mac + 3), *(ptr_mac + 4), *(ptr_mac + 5), *(ptr_mac + 6), *(ptr_mac + 7));

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cextended_addr%c: %c%s%c\r\n", sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		if (us_node_idx == (net_info->us_black_nodes - 1)) {
			/* last one -> close json object */
			i_ln_len = sprintf(puc_ln_buf, "\t}\r\n]");
		} else {
			i_ln_len = sprintf(puc_ln_buf, "\t},\r\n");
		}
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

	}

	close(json_fd);

	/* Move temporal file to WebServer folder */
	mov_file(spuc_blacklist_tmp_name, spuc_blacklist_name);

	printf ("BLACKLIST created\r\n");

	return (i_size_fd);
}


int net_info_report_path_info(uint16_t us_node_addr, uint8_t *path_info)
{
	x_node_list_t *px_node;
	uint8_t *ptr_mac;
	uint16_t us_node_idx;
	uint8_t puc_ext_addr[24];
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;

	printf ("PATH INFO updating...\r\n");

	json_fd = open(spuc_pathinfo_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	us_node_idx = 0;

	i_ln_len = sprintf(puc_ln_buf, "[\r\n");
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	for (us_node_idx = 0; us_node_idx < net_info->us_black_nodes; us_node_idx++) {
		i_ln_len = sprintf(puc_ln_buf, "\t{\r\n");
		i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cid%c: %u,\r\n", sc_comillas, sc_comillas, us_node_idx);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		ptr_mac = (uint8_t *)&net_info->puc_black_list[us_node_idx][0];

		sprintf(puc_ext_addr, "0x%02X%02X%02X%02X%02X%02X%02X%02X", *ptr_mac, *(ptr_mac + 1), *(ptr_mac + 2),
				*(ptr_mac + 3), *(ptr_mac + 4), *(ptr_mac + 5), *(ptr_mac + 6), *(ptr_mac + 7));

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cextended_addr%c: %c%s%c\r\n", sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		if (us_node_idx == (net_info->us_black_nodes - 1)) {
			/* last one -> close json object */
			i_ln_len = sprintf(puc_ln_buf, "\t}\r\n]");
		} else {
			i_ln_len = sprintf(puc_ln_buf, "\t},\r\n");
		}
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

	}

	close(json_fd);

	/* Move temporal file to WebServer folder */
	mov_file(spuc_blacklist_tmp_name, spuc_pathinfo_tmp_name);

	printf ("PATH INFO updated\r\n");

	return (i_size_fd);
}


