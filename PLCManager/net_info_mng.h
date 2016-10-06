#ifndef NET_INFO_MNG_H_INCLUDED
#define NET_INFO_MNG_H_INCLUDED

#include "socket_handler.h"

#define MAX_NET_INFO_SOCKET_SIZE      1000

enum net_info_mng_cmds {
	NET_INFO_WCMD_STOP_CYCLES = 0x01,
	NET_INFO_WCMD_START_CYCLES,
	NET_INFO_WCMD_UPDATE_LIST,
	NET_INFO_WCMD_GET_ID,
	NET_INFO_WCMD_INVALD
};

enum net_info_mng_rsps {
	NET_INFO_WRSP_STOP_OK = 0x80,
	NET_INFO_WRSP_START_OK,
	NET_INFO_WRSP_UPD_LIST,
	NET_INFO_WRSP_GET_ID,
	NET_INFO_WRSP_INVALD
};

void net_info_mng_init(int _app_id);
void net_info_mng_process(void);
void net_info_mng_callback(socket_ev_info_t *_ev_info);

#endif /* NET_INFO_MNG_H_INCLUDED */
