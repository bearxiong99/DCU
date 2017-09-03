#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "config.h"
#include "net_info_mng.h"
#include "net_info_report.h"

#ifdef NET_REPORT_DEBUG_CONSOLE
#	define LOG_REPORT_DEBUG(a)   printf a
#else
#	define LOG_REPORT_DEBUG(a)   (void)0
#endif

static char spuc_devlist_tmp_name[] = "/tmp/devlist.json";
static char spuc_devlist_name[] = "/home/DCWeb/public/tables/devlist.json";

static char spuc_numdev_tmp_name[] = "/tmp/numdev.json";
static char spuc_numdev_name[] = "/home/DCWeb/public/tables/numdev.json";

static const char sc_comillas = '"';

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

void net_info_report_init(void)
{

}

int net_info_report_devlist(x_dev_addr_t *dev_list, uint16_t us_num_nodes)
{
	x_dev_addr_t *px_node;
	uint8_t *ptr_mac;
	uint16_t us_node_idx;
	uint8_t puc_ext_addr_ascii[24];
	char puc_ln_buf[96];
	int i_ln_len, i_size_fd;
	int json_fd;

	/* Create DEVICE LIST */
//	LOG_REPORT_DEBUG(("DEVICE LIST creating...\r\n"));
	json_fd = open(spuc_devlist_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	us_node_idx = 0;

	if (us_num_nodes) {
		i_ln_len = sprintf(puc_ln_buf, "[\r\n");
	} else {
		i_ln_len = sprintf(puc_ln_buf, "[\r\n]");
	}
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	for (us_node_idx = 0; us_node_idx < us_num_nodes; us_node_idx++) {
		i_ln_len = sprintf(puc_ln_buf, "\t{\r\n");
		i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cid%c: %u,\r\n", sc_comillas, sc_comillas, us_node_idx);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		px_node = &dev_list[us_node_idx];

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cshort_addr%c: %c0x%04X%c,\r\n", sc_comillas, sc_comillas, sc_comillas, px_node->us_short_addr, sc_comillas);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		ptr_mac = px_node->puc_ext_addr;
		sprintf(puc_ext_addr_ascii, "0x%02X%02X%02X%02X%02X%02X%02X%02X", *ptr_mac, *(ptr_mac + 1), *(ptr_mac + 2),
				*(ptr_mac + 3), *(ptr_mac + 4), *(ptr_mac + 5), *(ptr_mac + 6), *(ptr_mac + 7));

		i_ln_len = sprintf(puc_ln_buf, "\t\t%cextended_addr%c: %c%s%c\r\n", sc_comillas, sc_comillas, sc_comillas, puc_ext_addr_ascii, sc_comillas);
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

		if (us_node_idx == (us_num_nodes - 1)) {
			/* last one -> close json object */
			i_ln_len = sprintf(puc_ln_buf, "\t}\r\n]");
		} else {
			i_ln_len = sprintf(puc_ln_buf, "\t},\r\n");
		}
		i_size_fd += write(json_fd, puc_ln_buf, i_ln_len);

	}

	close(json_fd);

	/* Move temporal file to WebServer folder */
	_mov_file(spuc_devlist_tmp_name, spuc_devlist_name);

//	LOG_REPORT_DEBUG(("DEVICE LIST created\r\n"));

	/* Create NUMBER OF DEVICES object */
//	LOG_REPORT_DEBUG(("DEV NUM creating...\r\n"));
	json_fd = open(spuc_numdev_tmp_name, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	i_ln_len = sprintf(puc_ln_buf, "%u", us_num_nodes);
	i_size_fd = write(json_fd, puc_ln_buf, i_ln_len);

	close(json_fd);

	/* Move temporal file to WebServer folder */
	_mov_file(spuc_numdev_tmp_name, spuc_numdev_name);

//	LOG_REPORT_DEBUG(("DEV NUM created\r\n"));

	return (i_size_fd);
}
