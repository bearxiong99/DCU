#include "hal_utils.h"
#include "ifaceSniffer.h"

/* Pointer to callback function to be establish*/
static sniffer_msg_cb_t pf_sniffer_cb;

/* USI CmdParams*/
x_usi_cmd_t sx_sniffer_msg;

void ifaceSniffer_set_cb(void (*sap_handler)(uint8_t* msg, uint16_t len))
{
	pf_sniffer_cb = sap_handler;
}


uint8_t ifaceSniffer_receivedCmd(uint8_t* ptrMsg, uint16_t len)
{
    if (pf_sniffer_cb) {
    	pf_sniffer_cb(ptrMsg,len);
    }
    return 1;
}

void ifaceSniffer_init(void)
{
	pf_sniffer_cb = 0;

	/* register to USI callback */
	hal_usi_set_callback(PROTOCOL_SNIF_G3, ifaceSniffer_receivedCmd);
}
