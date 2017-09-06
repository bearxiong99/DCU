#ifndef __PLCMANAGER__
#define __PLCMANAGER__

#include <unistd.h>
#include <stdint.h>

#define BOARD_ARIA          1
#define BOARD_SAMA5EK       2
#define BOARD               BOARD_ARIA

#define PLC_MNG_VERSION_HI	      0x01
#define PLC_MNG_VERSION_LO        0x01

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

enum {
	PLC_MNG_SNIFFER_APP_ID = 0,
	PLC_MNG_NET_INFO_APP_ID,
	PLC_MNG_USI_APP_ID,
	PLC_MNG_HTTP_MNG_APP_ID,
	PLC_MNG_MAX_APP_ID
};

enum {
	PLC_MNG_SNIFFER_APP_PORT      = 5000,
	PLC_MNG_NET_INFO_APP_PORT     = 5040,
	PLC_MNG_HTTP_MNG_APP_PORT     = 5060
};

typedef void (*pf_app_callback_t) (socket_ev_info_t *_ev_info);

#endif /* __PLCMANAGER__ */
