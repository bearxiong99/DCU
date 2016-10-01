/**
 * \file
 *
 * \brief HAL_USI_HOST: PLC Service Universal Serial Host Interface
 *
 *
 */

#ifndef USI_HOST_H_INCLUDE
#define USI_HOST_H_INCLUDE

#include "hal_utils.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

void usi_host_init();
int usi_host_open(char *sz_tty_name, unsigned ui_baudrate);
void usi_host_process(void);
usi_status_t usi_host_set_callback(usi_protocol_t protocol_id, uint8_t (*p_handler)(uint8_t *puc_rx_msg, uint16_t us_len));
usi_status_t usi_host_send_cmd(void *msg);
void usi_host_loopback(int _fd_redirect);
void usi_host_set_sniffer_cb(void (*sap_handler)(uint8_t* msg, uint16_t len));

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
#endif /* USI_HOST_H_INCLUDE */
