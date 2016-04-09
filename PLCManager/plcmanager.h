#ifndef __PLCMANAGER__
#define __PLCMANAGER__

#include <unistd.h>
#include <stdint.h>

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
	PLC_MNG_PRIMEMNG_APP_ID,
	PLC_MNG_CLI_APP_ID,
	PLC_MNG_DLMSoTCP_APP_ID,
	PLC_MNG_DLMS_APP_ID,
	PLC_MNG_PYSCRIPTS_APP_ID,
	PLC_MNG_USI_APP_ID,
	PLC_MNG_MAX_APP_ID
};

enum {
	PLC_MNG_SNIFFER_APP_PORT      = 9000,
	PLC_MNG_PRIMEMNG_APP_PORT     = 9010,
	PLC_MNG_CLI_APP_PORT          = 9020,
	PLC_MNG_DLMSoTCP_APP_PORT     = 9030,
	PLC_MNG_DLMS_APP_PORT         = 9040,
	PLC_MNG_PYSCRIPTS_APP_PORT    = 9050
};

typedef void (*pf_app_callback_t) (socket_ev_info_t *_ev_info);

#endif /* __PLCMANAGER__ */
