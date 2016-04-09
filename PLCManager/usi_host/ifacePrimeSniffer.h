#ifndef IFACEPRIMESNIFFER_H
#define IFACEPRIMESNIFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define SNIFFER_IF_PHY_COMMAND_SET_CHANNEL               2     /* SET PLC channel (1 = CENELEC- A) */

typedef void (*prime_sniffer_msg_cb_t)(uint8_t* ptrMsg, uint16_t len);

void prime_sniffer_init(void);
void prime_sniffer_set_cb(void (*sap_handler)(uint8_t* msg, uint16_t len));
void prime_sniffer_set_channel(uint8_t uc_channel);
uint8_t prime_sniffer_receivedCmd(uint8_t* ptrMsg, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // IFACEPRIMESNIFFER_H
