
#include "net_config.h"
#include "network_adapter_g3.h"

#define G3_ADP_MAX_DATA_LENGTH 1280

/* Global variables */
const NetBuffer*  mp_buffer = NULL;
size_t                mp_offset = 0;
uint8_t adp_sdu[G3_ADP_MAX_DATA_LENGTH];

/* Structure to define the G3 network adapter */
NicDriver g3_adapter = { NIC_TYPE_6LOWPAN,
						 1500,
						 Init,
						 Tick,
						 EnableIrq,
						 DisableIrq,
						 RxEventHandler,
						 ipv6_send_packet,
						 SetMacFilter,
						 NULL,
						 WritePhyReg,
						 ReadPhyReg,
						 TRUE,
						 TRUE,
						 TRUE };

/* G3 driver initialization */
error_t Init (NetInterface* interface)
{
	if (interface != NULL)
	{
		/* Force the TCP/IP stack to check the link state */
		interface->nicEvent = TRUE;
		osSetEvent(&netEvent);
		/* G3 is now ready to send */
		osSetEvent(&interface->nicTxEvent);
	}
	else
	{
		/* Only for MISRA C++ compliance */
	}

	return NO_ERROR;
}

/* RX routine */
void ipv6_receive_packet (struct TAdpDataIndication *pDataIndication)
{
	//osAcquireMutex(&netInterface[0U].nicDriverMutex);
	nicProcessPacket(&netInterface[0U], (uint8_t *)pDataIndication->m_pNsdu, pDataIndication->m_u16NsduLength);
	//osReleaseMutex(&netInterface[0U].nicDriverMutex);
}

/* TX routine */
error_t ipv6_send_packet (NetInterface*        interface,
		            		const NetBuffer* buffer,
		            		size_t               offset)
{
	static uint8_t uc_nsdu_handle = 0;
	error_t  sendPacketError;
	uint16_t us_data_length = netBufferGetLength(buffer) - offset;

	if(us_data_length <= G3_ADP_MAX_DATA_LENGTH) {
		(void)netBufferRead((uint8_t *)adp_sdu, buffer, offset, us_data_length);
		AdpDataRequest(us_data_length, adp_sdu, uc_nsdu_handle++, true, 0x00);
		sendPacketError = NO_ERROR;
	} else {
		sendPacketError = ERROR_WRONG_LENGTH;
	}

	osSetEvent(&interface->nicTxEvent);
	return sendPacketError;
}

/* Empty callbacks (functionality not needed within G3 network adapter) */
void Tick (NetInterface* interface)
{
	(void)interface;
}

void RxEventHandler (NetInterface* interface)
{
	(void)interface;
}

void EnableIrq (NetInterface* interface)
{
	(void)interface;
}

void DisableIrq (NetInterface* interface)
{
	(void)interface;
}

error_t SetMacFilter (NetInterface* interface)
{
	(void)interface;
	return NO_ERROR;
}

void WritePhyReg (uint8_t  phyAddr, uint8_t  regAddr, uint16_t data)
{
	(void)phyAddr;
	(void)regAddr;
	(void)data;
}

uint16_t ReadPhyReg (uint8_t phyAddr, uint8_t regAddr)
{
	(void)phyAddr;
	(void)regAddr;
	return 0U;
}
