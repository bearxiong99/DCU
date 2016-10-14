#ifndef NET_INFO_MNG_H_INCLUDED
#define NET_INFO_MNG_H_INCLUDED

#include "socket_handler.h"

//#define NET_INFO_DEBUG_CONSOLE

#define MAX_NET_INFO_SOCKET_SIZE      1000

enum net_info_mng_webcmds {
	WEBCMD_UPDATE_DASHBOARD = 0x30,
	WEBCMD_INVALD
};

enum net_info_mng_lnxcmds {
	LNXCMS_UPDATE_DASHBOARD = 0x40,
	LNXCMS_UPDATE_PATHLIST,
	LNXCMS_INVALD
};

void net_info_mng_init(int _app_id);
void net_info_mng_process(void);
//void net_info_mng_callback(socket_ev_info_t *_ev_info);
void net_info_webcmd_process(uint8_t* buf);

#endif /* NET_INFO_MNG_H_INCLUDED */
