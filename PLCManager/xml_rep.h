#ifndef XML_REP_H_INCLUDED
#define XML_REP_H_INCLUDED

#define NODELIST_TMP_FILE  "/tmp/nodelist.xml"
#define NODELIST_END_FILE  "/home/DCWS/public/nodelist.xml"

#define PRIME_INFO_FILE    "/home/DCWS/public/prime_info.xml"

int xml_rep_open_nodelist(void);
void xml_rep_close_nodelist(int xml_fd);
void xml_rep_add_node(int xml_fd, x_cli_node_reg_t *px_node, x_cli_node_vs_t *px_node_vs);
void xml_rep_update_node_version(x_cli_node_vs_t *px_node_vs);

void xml_rep_prime_info(x_cli_prime_info_t *x_info);

#endif /* XML_REP_H_INCLUDED */
