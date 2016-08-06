#ifndef L2_IPV6_IMPL_L2_NETWORK_ADAPTER_H
#define L2_IPV6_IMPL_L2_NETWORK_ADAPTER_H

#include "core/net.h"
#include "AdpApi.h"

/* Exported G3 network adapter */
extern NicDriver g3_adapter;

void ipv6_receive_packet (struct TAdpDataIndication *pDataIndication);
error_t ipv6_send_packet (NetInterface* interface, const NetBuffer* buffer, size_t offset);
error_t Init (NetInterface* interface);

void 		Tick (NetInterface* interface);
void        RxEventHandler (NetInterface* interface);
void        EnableIrq (NetInterface* interface);
void        DisableIrq (NetInterface* interface);
error_t     SetMacFilter (NetInterface* interface);
void        WritePhyReg (uint8_t  phyAddr, uint8_t  regAddr, uint16_t data);
uint16_t 	ReadPhyReg (uint8_t phyAddr, uint8_t regAddr);

#endif // L2_IPV6_IMPL_L2_NETWORK_ADAPTER_H
