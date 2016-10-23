#ifndef NET_INFO_MNG_H_INCLUDED
#define NET_INFO_MNG_H_INCLUDED

#include "socket_handler.h"

//#define NET_INFO_DEBUG_CONSOLE

#define MAX_NET_INFO_SOCKET_SIZE      1000

#define RESET_GPIO_ID          4  // S19:PA4
#define ERASE_GPIO_ID          2  // S21:PA2

#define RESET_GPIO_ENABLE      0
#define RESET_GPIO_DISABLE     1

#define ERASE_GPIO_ENABLE      1
#define ERASE_GPIO_DISABLE     0

enum net_info_mng_webcmds {
	WEBCMD_UPDATE_DASHBOARD = 0x30,
	WEBCMD_INVALD
};

enum net_info_mng_lnxcmds {
	LNXCMS_UPDATE_DASHBOARD = 0x40,
	LNXCMS_UPDATE_PATHLIST,
	LNXCMS_UPDATE_ROUNDTIME,
	LNXCMS_INVALD
};

void net_info_mng_init(int _app_id);
void net_info_mng_process(void);
//void net_info_mng_callback(socket_ev_info_t *_ev_info);
void net_info_webcmd_process(uint8_t* buf);

#endif /* NET_INFO_MNG_H_INCLUDED */
