#ifndef IFACESNIFFER_H
#define IFACESNIFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void (*sniffer_msg_cb_t)(uint8_t* ptrMsg, uint16_t len);

void ifaceSniffer_init(void);
void ifaceSniffer_set_cb(void (*sap_handler)(uint8_t* msg, uint16_t len));
uint8_t ifaceSniffer_receivedCmd(uint8_t* ptrMsg, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // IFACESNIFFER_H
