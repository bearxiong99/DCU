#include "hal_utils.h"
#include "ifacePrimeSniffer.h"


/* Pointer to callback function to be establish*/
static prime_sniffer_msg_cb_t pf_prime_sniffer_cb;

/* buffer used to serialization */
static uint8_t uc_serial_buf[8];

/* USI CmdParams*/
x_usi_cmd_t sx_sniffer_msg;

void prime_sniffer_set_cb(void (*sap_handler)(uint8_t* msg, uint16_t len))
{
	pf_prime_sniffer_cb = sap_handler;
}

void prime_sniffer_set_channel(uint8_t uc_channel)
{
    uint8_t *puc_msg;
    x_usi_cmd_t x_sniffer_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = SNIFFER_IF_PHY_COMMAND_SET_CHANNEL;
    *puc_msg++ = uc_channel;

    /* Send to USI */
    x_sniffer_msg.uc_p_type = PROTOCOL_SNIF_PRIME;
    x_sniffer_msg.puc_buf = uc_serial_buf;
    x_sniffer_msg.us_len = puc_msg - uc_serial_buf;
    x_sniffer_msg._fd = -1;

    hal_usi_send_cmd(&x_sniffer_msg);
}

uint8_t prime_sniffer_receivedCmd(uint8_t* ptrMsg, uint16_t len)
{
    if (pf_prime_sniffer_cb) {
    	pf_prime_sniffer_cb(ptrMsg,len);
    }
    return 1;
}

void prime_sniffer_init(void)
{
	pf_prime_sniffer_cb = 0;

	/* register to USI callback */
	hal_usi_set_callback(PROTOCOL_SNIF_PRIME, prime_sniffer_receivedCmd);
}
