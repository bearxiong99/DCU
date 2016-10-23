#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "net_info_report.h"

#ifdef NET_REPORT_DEBUG_CONSOLE
#	define LOG_REPORT_DEBUG(a)   printf a
#else
#	define LOG_REPORT_DEBUG(a)   (void)0
#endif

static uint16_t pus_json_refs[NUM_MAX_NODES];

//static char spuc_conf_name[20];
//static char spuc_tmp_name[50];
//static char spuc_file_name[50];
//static char spuc_netroutes_tmp_name[] = "/tmp/netroutes.json";
//static char spuc_netroutes_name[] = "/home/DCWeb/public/tables/netroutes.json";

static char spuc_netlist_tmp_name[] = "/tmp/netlist.json";
static char spuc_netlist_name[] = "/home/DCWeb/public/tables/netlist.json";

static char spuc_blacklist_tmp_name[] = "/tmp/blacklist.json";
static char spuc_blacklist_name[] = "/home/DCWeb/public/tables/blacklist.json";

static char spuc_pathlist_tmp_name[] = "/tmp/pathlist.json";
static char spuc_pathlist_name[] = "/home/DCWeb/public/tables/pathlist.json";

static char spuc_numdev_tmp_name[] = "/tmp/numdev.json";
static char spuc_numdev_name[] = "/home/DCWeb/public/tables/numdev.json";

static char spuc_dashboard_tmp_name[] = "/tmp/dashboard.json";
static char spuc_dashboard_name[] = "/home/DCWeb/public/tables/dashboard.json";

static char spuc_roundtime_tmp_name[] = "/tmp/roundtime.json";
static char spuc_roundtime_name[] = "/home/DCWeb/public/tables/roundtime.json";

//static char spuc_nodesinfo_path[] = "/home/DCWeb/public/tables/nodes/";

static const char sc_comillas = '"';

//static uint16_t _extract_u16(void *vptr_value) {
//	uint16_t us_val_swap;
//	uint8_t uc_val_tmp;
//
//	uc_val_tmp = *(uint8_t *)vptr_value;
//	us_val_swap = (uint16_t)uc_val_tmp;
//
//	uc_val_tmp = *((uint8_t *)vptr_value + 1);
//	us_val_swap += ((uint16_t)uc_val_tmp) << 8;
//
//	return us_val_swap;
//}

static void _mov_file(char *_src_path, char *_dst_path)
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

//static void _add_to_file(char *_src_name, int dst_fd)
//{
//	char puc_ln_path[50];
//	int i_ln_len;
//	int src_fd;
//	char c;
//
//	i_ln_len = sprintf(puc_ln_path, "%s%s", spuc_nodesinfo_path, _src_name);
//
//	src_fd = open(puc_ln_path, O_RDONLY);
//	while(read(src_fd, &c, sizeof(c) != 0)){
//		write(dst_fd, &c ,sizeof(c));
//	}
//	close(src_fd);
//}
//
//static void _build_pathinfo(void)
//{
//	char puc_ln_buf[96];
//	int i_ln_len, i_size_fd;
//	DIR *dir;
//	struct dirent *ent;
//	int dst_fd;
//
//	dir = opendir (spuc_nodesinfo_path);
//
//	dst_fd = creat(spuc_pathlist_tmp_name, O_RDWR);
//
//	if (dir == NULL) {
//		LOG_REPORT_DEBUG(("No puedo abrir el directorio"));
//		return;
//	}
//
//	/* Append Nodes data */
//	i_ln_len = sprintf(puc_ln_buf, "{\r\n%cnodes%c:[\r\n", sc_comillas, sc_comillas);
//	i_size_fd = write(dst_fd, puc_ln_buf, i_ln_len);
//
//	while ((ent = readdir (dir)) != NULL) {
//		if (strncmp(ent->d_name, "node_", 5) == 0) {
//			_add_to_file(ent->d_name, dst_fd);
//		}
//	}
//	closedir (dir);
//
//	 // remove last semicolon
//	close(dst_fd);
//	dst_fd = open(spuc_pathlist_tmp_name, O_RDWR);
//	lseek(dst_fd, 4, SEEK_END);
//	i_ln_len = sprintf(puc_ln_buf, "\r\n]\r\n%clinks%c:[\r\n", sc_comillas, sc_comillas);
//	i_size_fd = write(dst_fd, puc_ln_buf, i_ln_len);
//
//	dir = opendir (spuc_nodesinfo_path);
//	/* Append NodeLinks data */
//	while ((ent = readdir (dir)) != NULL) {
//		if (strncmp(ent->d_name, "nodelink_", 5) == 0) {
//			_add_to_file(ent->d_name, dst_fd);
//		}
//	}
//	closedir (dir);
//
//	// remove last semicolon
//	close(dst_fd);
//	dst_fd = open(spuc_pathlist_tmp_name, O_RDWR);
//	lseek(dst_fd, 4, SEEK_END);
//	i_ln_len = sprintf(puc_ln_buf, "\r\n]\r\n}");
//	i_size_fd = write(dst_fd, puc_ln_buf, i_ln_len);
//
//	close(dst_fd);
//}

void net_info_report_init(void)
{

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
//		_mov_file(spuc_tmp_name, spuc_file_name);
//	}
//}
//

int net_info_report_pathlist(x_net_info_t *net_info, x_routes_info_t *routes_info)
{
	int json_fd;
	char puc_ln_buf[100];
	int i_ln_len, i_size_fd;
	uint8_t *ptr_mac;
	x_node_list_t *px_node;
	x_path_info_t *px_path;
	uint8_t uc_node_idx;
	uint16_t us_path_idx;
	uint8_t uc_hops;
	uint8_t puc_ext_addr[24];

//	LOG_REPORT_DEBUG(("PATHLIST creating...\r\n"));

	json_fd = open(spuc_pathlist_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);;

	i_ln_len = sprintf(puc_ln_buf, "{\r\n\t%cnodes%c:[\r\n", sc_comillas, sc_comillas);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	memset(puc_ext_addr, 0, sizeof(puc_ext_addr));
	uc_hops = 0;
	memset(pus_json_refs, INVALID_NODE_ADDRESS, sizeof(pus_json_refs));

	/* First Node is Coordinator */
	ptr_mac = net_info->puc_extended_addr;
	sprintf(puc_ext_addr, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", *(ptr_mac + 7), *(ptr_mac + 6), *(ptr_mac + 5),
					*(ptr_mac + 4), *(ptr_mac + 3), *(ptr_mac + 2), *(ptr_mac + 1), *ptr_mac);

	if (net_info->us_num_nodes) {
		i_ln_len = sprintf(puc_ln_buf, "\t\t{%cu16Addr%c:%c0%c,%cu64Addr%c:%c%s%c,%chops%c:%d},\r\n",
				sc_comillas, sc_comillas, sc_comillas, sc_comillas,
				sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas, sc_comillas, sc_comillas, uc_hops);
	} else {
		i_ln_len = sprintf(puc_ln_buf, "\t\t{%cu16Addr%c:%c0%c,%cu64Addr%c:%c%s%c,%chops%c:%d}\r\n",
						sc_comillas, sc_comillas, sc_comillas, sc_comillas,
						sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas, sc_comillas, sc_comillas, uc_hops);
	}
	i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
	/* Coordinator json ref -> Use index 0 */
	pus_json_refs[0] = 0;


	/* List of Device Nodes */
	for (uc_node_idx = 0; uc_node_idx < net_info->us_num_nodes; uc_node_idx++) {
		/* get node info */
		px_node = &net_info->x_node_list[uc_node_idx];
		/* Node json ref */
		pus_json_refs[px_node->us_short_address] = uc_node_idx + 1;

		/* get hops from routes info using extended address */
		for (us_path_idx = 0; us_path_idx < net_info->us_num_path_nodes; us_path_idx++) {
			uint8_t *puc_ext_addr;

			puc_ext_addr = &routes_info->puc_ext_addr[us_path_idx][0];
			if (memcmp (puc_ext_addr, px_node->puc_extended_address, EXT_ADDRESS_SIZE) == 0) {
				uc_hops = routes_info->x_path_info[us_path_idx].m_u8ForwardHopsCount;
				break;
			}
		}

		ptr_mac = &px_node->puc_extended_address;
		sprintf(puc_ext_addr, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", *ptr_mac, *(ptr_mac + 1), *(ptr_mac + 2),
				*(ptr_mac + 3), *(ptr_mac + 4), *(ptr_mac + 5), *(ptr_mac + 6), *(ptr_mac + 7));

		i_ln_len = sprintf(puc_ln_buf, "\t\t{%cu16Addr%c:%c%d%c,%cu64Addr%c:%c%s%c,%chops%c:%d},\r\n",
				sc_comillas, sc_comillas, sc_comillas, px_node->us_short_address, sc_comillas,
				sc_comillas, sc_comillas, sc_comillas, puc_ext_addr, sc_comillas, sc_comillas, sc_comillas, uc_hops);

		if (uc_node_idx == (net_info->us_num_nodes - 1)) {
			/* Last iteration */
			i_size_fd += write(json_fd, puc_ln_buf, i_ln_len - 3); /* remove the last semicolon, CR and LF */
		} else {
			/* Last iteration */
			i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
		}
	}

	i_ln_len = sprintf(puc_ln_buf, "\r\n\t],\r\n\t%clinks%c:[\r\n", sc_comillas, sc_comillas);
	write(json_fd, puc_ln_buf, i_ln_len);

	/* List Links -> Use JSON refs instead of short address */
	for (us_path_idx = 0; us_path_idx < net_info->us_num_path_nodes; us_path_idx++) {
		uint16_t us_src, us_dst;
		uint8_t uc_hope_idx, uc_link_cost;

		px_path = &routes_info->x_path_info[us_path_idx];

		/* Forward hopes */
		uc_hope_idx = px_path->m_u8ForwardHopsCount - 1;
		if (px_path->m_u8ForwardHopsCount == 1) {
			/* Only one hop -> Src is coordinator address */
			us_src = 0;
		} else {
			/* More than one hop */
			us_src = px_path->m_aForwardPath[uc_hope_idx - 1].m_u16HopAddress;
		}
		us_dst = px_path->m_aForwardPath[uc_hope_idx].m_u16HopAddress;
		uc_link_cost = px_path->m_aForwardPath[uc_hope_idx].m_u8LinkCost;
		i_ln_len = sprintf(puc_ln_buf, "\t\t{%csource%c:%d,%ctarget%c:%d,%cvalue%c:%d},\r\n",
							sc_comillas, sc_comillas, pus_json_refs[us_src],
							sc_comillas, sc_comillas, pus_json_refs[us_dst],
							sc_comillas, sc_comillas, uc_link_cost);

		if (us_path_idx == (net_info->us_num_path_nodes - 1)) {
			/* Last iteration */
			i_size_fd += write(json_fd, puc_ln_buf, i_ln_len - 3); /* remove the last semicolon, CR and LF */
		} else {
			/* Next iteration */
			i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
		}
	}

		/* Print every paths */
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

		/* !!! NO NEED BECAUSE PATH ARE ALWAYS SYMETRIC */

		/* Print every paths */
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

	i_ln_len = sprintf(puc_ln_buf, "\r\n\t]\r\n}");
	i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);
	close(json_fd);

	/* Move temporal file to WebServer folder */
	_mov_file(spuc_pathlist_tmp_name, spuc_pathlist_name);

	LOG_REPORT_DEBUG(("PATHLIST created\r\n"));

	return (i_size_fd);
}

int net_info_report_netlist(x_net_info_t *net_info)
{
	x_node_list_t *px_node;
	uint8_t *ptr_mac;
	uint16_t us_node_idx;
	uint8_t puc_ext_addr[24];
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;

//	LOG_REPORT_DEBUG(("NETLIST creating...\r\n"));

	json_fd = open(spuc_netlist_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	us_node_idx = 0;

	if (net_info->us_num_nodes) {
		i_ln_len = sprintf(puc_ln_buf, "[\r\n");
	} else {
		i_ln_len = sprintf(puc_ln_buf, "[\r\n]");
	}
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
	_mov_file(spuc_netlist_tmp_name, spuc_netlist_name);

	LOG_REPORT_DEBUG(("NETLIST created\r\n"));

	return (i_size_fd);
}

int net_info_report_blacklist(x_net_info_t *net_info)
{
	uint8_t *ptr_mac;
	uint16_t us_node_idx;
	uint8_t puc_ext_addr[24];
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;

//	LOG_REPORT_DEBUG(("BLACKLIST creating...\r\n"));

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
	_mov_file(spuc_blacklist_tmp_name, spuc_blacklist_name);

	LOG_REPORT_DEBUG(("BLACKLIST created\r\n"));

	return (i_size_fd);
}

int net_info_report_dev_num(x_net_info_t *net_info)
{
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;

//	LOG_REPORT_DEBUG(("DEV NUM creating...\r\n"));

	json_fd = open(spuc_numdev_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	i_ln_len = sprintf(puc_ln_buf, "%u", net_info->us_num_nodes);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	close(json_fd);

	/* Move temporal file to WebServer folder */
	_mov_file(spuc_numdev_tmp_name, spuc_numdev_name);

	LOG_REPORT_DEBUG(("DEV NUM created\r\n"));

	return (i_size_fd);
}

int net_info_report_dashboard(x_net_info_t *net_info, x_net_statistics_t *net_stats)
{
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;
	uint32_t ul_tmp;

//	LOG_REPORT_DEBUG(("DASHBOARD creating...\r\n"));

	json_fd = open(spuc_dashboard_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	i_ln_len = sprintf(puc_ln_buf, "[{");
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	/* Number of devices */
	i_ln_len = sprintf(puc_ln_buf, "%cnumdevs%c:%u,", sc_comillas, sc_comillas, net_info->us_num_nodes);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);
	/* Number of data request */
	i_ln_len = sprintf(puc_ln_buf, "%cnummsgs%c:%u,", sc_comillas, sc_comillas, net_stats->us_num_data_tx);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);
	/* Number of ping request */
	i_ln_len = sprintf(puc_ln_buf, "%cnumpings%c:%u,", sc_comillas, sc_comillas, net_stats->us_num_ping_tx);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);
	/* Number of path request */
	i_ln_len = sprintf(puc_ln_buf, "%cnumpaths%c:%u,", sc_comillas, sc_comillas, net_info->us_num_path_nodes);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	/* Network coverage ?????? */
	if (net_info->us_num_nodes) {
		ul_tmp = net_info->us_num_path_nodes * 100 / net_info->us_num_nodes;
	} else {
		ul_tmp = 0;
	}
	i_ln_len = sprintf(puc_ln_buf, "%cnet_cov%c:%u,", sc_comillas, sc_comillas, ul_tmp);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	/* Data coverage */
	if (net_stats->us_num_data_tx) {
		ul_tmp = (net_stats->us_num_data_rx * 100) / net_stats->us_num_data_tx;
	} else {
		ul_tmp = 0;
	}
	i_ln_len = sprintf(puc_ln_buf, "%cdata_cov%c:%u,", sc_comillas, sc_comillas, ul_tmp);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	/* Ping success */
	if (net_stats->us_num_ping_tx) {
		ul_tmp = (net_stats->us_num_ping_rx * 100) / net_stats->us_num_ping_tx;
	} else {
		ul_tmp = 0;
	}
	i_ln_len = sprintf(puc_ln_buf, "%cping_cov%c:%u,", sc_comillas, sc_comillas, ul_tmp);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	/* Routing errors */
	if (net_stats->us_num_path_req) {
		ul_tmp = (net_stats->us_num_path_succ * 100) / net_stats->us_num_path_req;
		i_ln_len = sprintf(puc_ln_buf, "%croute_errors%c:%u", sc_comillas, sc_comillas, 100 - ul_tmp);
	} else {
		ul_tmp = 0;
		i_ln_len = sprintf(puc_ln_buf, "%croute_errors%c:%u", sc_comillas, sc_comillas, ul_tmp);
	}
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);


	i_ln_len = sprintf(puc_ln_buf, "}]");
	i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

	close(json_fd);

	/* Move temporal file to WebServer folder */
	_mov_file(spuc_dashboard_tmp_name, spuc_dashboard_name);

	LOG_REPORT_DEBUG(("DASHBOARD created\r\n"));

	return (i_size_fd);
}

int net_info_report_round_time(int i_round_time, uint16_t us_short_addr)
{
	char puc_ln_buf[20];
	int i_ln_len, i_size_fd;
	int json_fd;

//	char puc_date_buf[20];
//	time_t t = time(NULL);
//
//	struct tm *tm = localtime(&t);
//	sprintf(puc_date_buf, "%c%u:%u:%u%c", sc_comillas, tm->tm_hour, tm->tm_min, tm->tm_sec, sc_comillas);

//	LOG_REPORT_DEBUG(("Round Time creating...\r\n"));

	json_fd = open(spuc_roundtime_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	/* Number of devices */
	i_ln_len = sprintf(puc_ln_buf, "{%clabel%c:%c0x%04x%c,", sc_comillas, sc_comillas, sc_comillas, us_short_addr, sc_comillas);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);
	/* Number of data request */
	i_ln_len = sprintf(puc_ln_buf, "%cdata%c:%u}", sc_comillas, sc_comillas, i_round_time);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	close(json_fd);

	/* Move temporal file to WebServer folder */
	_mov_file(spuc_roundtime_tmp_name, spuc_roundtime_name);

	LOG_REPORT_DEBUG(("Round Time created\r\n"));

	return (i_size_fd);
}


