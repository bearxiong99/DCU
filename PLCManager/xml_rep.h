#ifndef XML_REP_H_INCLUDED
#define XML_REP_H_INCLUDED

int xml_rep_open(void);
void xml_rep_close(int xml_fd);
void xml_rep_add_node(int xml_fd, x_cli_node_reg_t *px_node);

#endif /* XML_REP_H_INCLUDED */
