#ifndef DLMS_EMU_REPORT_H_INCLUDED
#define DLMS_EMU_REPORT_H_INCLUDED

typedef struct dlms_report_info {
	char pc_dev_sn[20];
	char pc_operation[10];
	char pc_obis[20];
	int i_class_id;
	int i_element_id;
	char pc_result[30];
} dlms_report_info_t;

void dlms_emu_report_init(void);
void dlms_emu_report_start_cycle(void);
void dlms_emu_report_end_cycle(void);
void dlms_emu_report_node_cycle(dlms_report_info_t *x_node_info);

int dlms_emu_report_xml_init(void);
void dlms_emu_report_xml_end(int xml_fd);
void dlms_emu_report_xml_add_node(int xml_fd, x_node_list_t *px_node);

#endif /* DLMS_EMU_REPORT_H_INCLUDED */
