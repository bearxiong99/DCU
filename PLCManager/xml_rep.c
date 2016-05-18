#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cli.h"
#include "xml_rep.h"

#define NODELIST_TMP_FILE  "/tmp/nodelist.xml"
#define NODELIST_END_FILE  "/home/DCWS/public/nodelist.xml"

#define PRIME_INFO_FILE    "/home/DCWS/public/prime_info.xml"

void xml_rep_prime_info(x_cli_prime_info_t *x_info)
{
	int xml_fd, i_ln_len;
	char puc_aux_buf[50];

	remove(PRIME_INFO_FILE);

	/* Create XML file with the list of connected nodes */
	xml_fd = open(PRIME_INFO_FILE, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_aux_buf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<pinfo>\r\n");
	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<mac>%02x%02x%02x%02x%02x%02x</mac>\r\n", x_info->mac_addr[0], x_info->mac_addr[1], x_info->mac_addr[2],
			x_info->mac_addr[3], x_info->mac_addr[4], x_info->mac_addr[5]);
	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<vendor_id>0x%04x</vendor_id>\r\n", x_info->us_vendor_id);
	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<product_id>0x%04x</product_id>\r\n", x_info->us_product_id);
	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<fw_version>%s</fw_version>\r\n", x_info->fw_version);
	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "</pinfo>");
	write(xml_fd, puc_aux_buf, i_ln_len);

	close(xml_fd);

	chmod((const char *)xml_fd, 0777);
}

int xml_rep_open_nodelist(void)
{
	int xml_fd, i_ln_len;
	char puc_aux_buf[50];

	remove(NODELIST_TMP_FILE);

	/* Create XML file with the list of connected nodes */
	xml_fd = open(NODELIST_TMP_FILE, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_aux_buf, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n<nodelist>\r\n");
	write(xml_fd, puc_aux_buf, i_ln_len);

	return xml_fd;
}

void xml_rep_close_nodelist(int xml_fd)
{
	int i_ln_len;
	int xml_fd1, xml_fd2;
	char puc_aux_buf[50];
	char c;

	i_ln_len = sprintf(puc_aux_buf, "</nodelist>");
	write(xml_fd, puc_aux_buf, i_ln_len);
	close(xml_fd);

	remove(NODELIST_END_FILE);

	xml_fd1 = open(NODELIST_TMP_FILE, O_RDONLY);
	xml_fd2 = open(NODELIST_END_FILE, O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);

	/* Copy temporal file to FTP folder */
	while(read(xml_fd1, &c, sizeof(c) != 0)){
		write(xml_fd2, &c ,sizeof(c));
	}
	close(xml_fd1);
	close(xml_fd2);

	chmod((const char *)xml_fd2, 0777);
}

void xml_rep_add_node(int xml_fd, x_cli_node_reg_t *px_node)
{
	int i_ln_len;
	char puc_aux_buf[120];

//	i_ln_len = sprintf(puc_aux_buf, "<node mac=\"%02x%02x%02x%02x%02x%02x\" lnid=\"%u\" state=\"%u\" lsid=\"%u\" sid=\"%u\" level=\"%u\"></node>\r\n",
//			px_node->mac_addr[0], px_node->mac_addr[1], px_node->mac_addr[2], px_node->mac_addr[3], px_node->mac_addr[4], px_node->mac_addr[5],
//			px_node->us_lnid, px_node->uc_state, px_node->us_lsid, px_node->us_sid, px_node->uc_level);
//	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t<node>\r\n");
	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<mac>%02x%02x%02x%02x%02x%02x</mac>\r\n", px_node->mac_addr[0], px_node->mac_addr[1], px_node->mac_addr[2],
			px_node->mac_addr[3], px_node->mac_addr[4], px_node->mac_addr[5]);
	write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<lnid>%u</lnid>\r\n", px_node->us_lnid);
		write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<state>%u</state>\r\n", px_node->uc_state);
		write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<lsid>%u</lsid>\r\n", px_node->us_lsid);
		write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<sid>%u</sid>\r\n", px_node->us_sid);
		write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t\t<level>%u</level>\r\n", px_node->uc_level);
		write(xml_fd, puc_aux_buf, i_ln_len);

	i_ln_len = sprintf(puc_aux_buf, "\t</node>\r\n");
	write(xml_fd, puc_aux_buf, i_ln_len);
}


