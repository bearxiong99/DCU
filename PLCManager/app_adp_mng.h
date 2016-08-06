#ifndef APP_ADP_MNG_H_INCLUDED
#define APP_ADP_MNG_H_INCLUDED

#include "socket_handler.h"

#define MAX_APP_ADP_SOCKET_SIZE      1000

enum dlms_emu_commands {
	DLMS_EMU_CMD_STOP_CYCLES = 0x30,
	DLMS_EMU_CMD_START_CYCLES = 0x31,
	DLMS_EMU_CMD_UPDATE_LIST = 0x32,
	DLMS_EMU_CMD_INVALD
};

void adp_mng_init(int _app_id);
void adp_mng_process(void);
void adp_mng_callback(socket_ev_info_t *_ev_info);

uint16_t adp_mng_update_registered_nodes(void *pxNodeList);

#endif /* APP_ADP_MNG_H_INCLUDED */
