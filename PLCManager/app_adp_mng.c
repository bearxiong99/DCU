#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "socket_handler.h"

#include "AdpApi.h"
#include "AdpApiTypes.h"

#include "Config.h"
#include "app_debug.h"

#include "drivers/g3/network_adapter_g3.h"
#include "app_adp_mng.h"
#include "dlms_emu_coord.h"
#include "bs_api.h"

/* MAC include */
#include "MacMib.h"

#ifdef APP_DEBUG_CONSOLE
#	define LOG_APP_DEBUG(a)   printf a
#else
#	define LOG_APP_DEBUG(a)   (void)0
#endif

static int si_app_adp_id;
static int si_app_adp_link_fd;
static int si_app_adp_data_fd;
static unsigned char suc_app_adp_buf[MAX_APP_ADP_SOCKET_SIZE];

/* Exteneded Address by default */
uint8_t CONF_EXTENDED_ADDRESS[8] = {0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};

/* MIB types and definitions */
enum MibType
{
	MIB_MAC = 0,
	MIB_ADP
};

struct MibData
{
	uint8_t m_u8Type;
	const char *m_szName;
	uint32_t m_u32Id;
	uint16_t m_u16Index;
	uint8_t m_u8ValueLength;
	const uint8_t *m_pu8Value;
};

uint8_t g_u8MibInitIndex = 0;
const uint8_t g_u8MibTableSize = 9;
uint8_t g_CoordInitComplete;
struct MibData g_MibSettings[] =
{
	{ MIB_MAC, "MAC_PIB_MANUF_EXTENDED_ADDRESS", MAC_PIB_MANUF_EXTENDED_ADDRESS, 0, 8, CONF_EXTENDED_ADDRESS },
	{ MIB_MAC, "MAC_PIB_SHORT_ADDRESS", MAC_PIB_SHORT_ADDRESS, 0, 2, CONF_SHORT_ADDRESS },
	{ MIB_ADP, "ADP_IB_CONTEXT_INFORMATION_TABLE", ADP_IB_CONTEXT_INFORMATION_TABLE, 0, 14, CONF_CONTEXT_INFORMATION_TABLE_0 },
	{ MIB_ADP, "ADP_IB_CONTEXT_INFORMATION_TABLE", ADP_IB_CONTEXT_INFORMATION_TABLE, 1, 10, CONF_CONTEXT_INFORMATION_TABLE_1 },
	{ MIB_ADP, "ADP_IB_GROUP_TABLE", ADP_IB_GROUP_TABLE, 0, 2, CONF_GROUP_TABLE_0 },
	{ MIB_ADP, "ADP_IB_ROUTING_TABLE_ENTRY_TTL", ADP_IB_ROUTING_TABLE_ENTRY_TTL, 0, 2, CONF_ROUTING_TABLE_ENTRY_TTL },
	{ MIB_ADP, "ADP_IB_MAX_JOIN_WAIT_TIME", ADP_IB_MAX_JOIN_WAIT_TIME, 0, 2, CONF_MAX_JOIN_WAIT_TIME },
	{ MIB_ADP, "ADP_IB_MAX_HOPS", ADP_IB_MAX_HOPS, 0, 1, CONF_MAX_HOPS },
	{ MIB_ADP, "ADP_IB_MANUF_EAP_PRESHARED_KEY", ADP_IB_MANUF_EAP_PRESHARED_KEY, 0, 16, CONF_PSK_KEY }
};

static void SetConfirm(uint8_t u8Status, uint32_t u32AttributeId, uint16_t u16AttributeIndex)
{
	bool b_send_pib;

	b_send_pib = true;

	if (u8Status == G3_SUCCESS)	{
		// initialization phase?
		if (g_u8MibInitIndex < g_u8MibTableSize) {
			if ((g_MibSettings[g_u8MibInitIndex].m_u32Id == u32AttributeId) && (g_MibSettings[g_u8MibInitIndex].m_u16Index == u16AttributeIndex)) {
				g_u8MibInitIndex++;
				if (g_u8MibInitIndex == g_u8MibTableSize) {
					// Stop
					b_send_pib = false;
#ifdef DLMS_REPORT
					printf("Modem fully initialized.\r\n");
#endif
				}
			} else {
				LOG_APP_DEBUG(("ERR[AppAdpSetConfirm] Invalid SetConfirm received during initialization. Expecting 0x%08X/%u but received 0x%08X/%u\r\n",
					g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index,
					u32AttributeId, u16AttributeIndex));
			}
		} else {
			/* out of range */
			b_send_pib = false;
		}
	} else {
		LOG_APP_DEBUG(("ERR[AppAdpSetConfirm] status: %u\r\n", u8Status));
	}

	if (b_send_pib) {
		/* Next value */
		if (g_MibSettings[g_u8MibInitIndex].m_u8Type == MIB_ADP) {
			AdpSetRequest(g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index, g_MibSettings[g_u8MibInitIndex].m_u8ValueLength, g_MibSettings[g_u8MibInitIndex].m_pu8Value);
		} else {
			AdpMacSetRequest(g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index, g_MibSettings[g_u8MibInitIndex].m_u8ValueLength, g_MibSettings[g_u8MibInitIndex].m_pu8Value);
		}
	}
}

static void AppAdpMsgStatus(uint8_t u8MsgStatus)
{
	(void)u8MsgStatus;
}

static void AppAdpDataConfirm(struct TAdpDataConfirm *pDataConfirm)
{
	(void)pDataConfirm;
}

static void AppAdpDataIndication(struct TAdpDataIndication *pDataIndication)
{
	if (pDataIndication->m_u16NsduLength) {
		dlms_emu_data_ind_handler((void *)pDataIndication);
		LOG_APP_DEBUG(("ERR[AppAdpDataIndication] Len: %u LQI: %u\r\n", pDataIndication->m_u16NsduLength, pDataIndication->m_u8LinkQualityIndicator));
	}
}

static void AppAdpDiscoveryConfirm(uint8_t uc_status)
{
	(void)uc_status;
}

static void AppAdpDiscoveryIndication(struct TAdpPanDescriptor *pPanDescriptor)
{
	(void)pPanDescriptor;
}

static void AppAdpNetworkStartConfirm(struct TAdpNetworkStartConfirm *pNetworkStartConfirm)
{
	(void)pNetworkStartConfirm;
}

static void AppAdpNetworkJoinConfirm(struct TAdpNetworkJoinConfirm *pNetworkJoinConfirm)
{
	(void)pNetworkJoinConfirm;
}

static void AppAdpNetworkLeaveIndication(void)
{

}

static void AppAdpNetworkLeaveConfirm(struct TAdpNetworkLeaveConfirm *pLeaveConfirm)
{
	(void)pLeaveConfirm;
}

static void AppAdpResetConfirm(struct TAdpResetConfirm *pResetConfirm)
{
	(void)pResetConfirm;
}

static void AppAdpSetConfirm(struct TAdpSetConfirm *pSetConfirm)
{
	SetConfirm(pSetConfirm->m_u8Status, pSetConfirm->m_u32AttributeId, pSetConfirm->m_u16AttributeIndex);
}

static void AppAdpMacSetConfirm(struct TAdpMacSetConfirm *pSetConfirm)
{
	SetConfirm(pSetConfirm->m_u8Status, pSetConfirm->m_u32AttributeId, pSetConfirm->m_u16AttributeIndex);
}

static void AppAdpGetConfirm(struct TAdpGetConfirm *pGetConfirm)
{
	(void)pGetConfirm;
}

static void AppAdpMacGetConfirm(struct TAdpMacGetConfirm *pGetConfirm)
{
	(void)pGetConfirm;
}

static void AppAdpRouteDiscoveryConfirm(struct TAdpRouteDiscoveryConfirm *pRouteDiscoveryConfirm)
{
	(void)pRouteDiscoveryConfirm;
}

static void AppAdpPathDiscoveryConfirm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm)
{
	(void)pPathDiscoveryConfirm;

	dlms_emu_path_node_cfm(pPathDiscoveryConfirm);
}

static void AppAdpNetworkStatusIndication(struct TAdpNetworkStatusIndication *pNetworkStatusIndication)
{
	(void)pNetworkStatusIndication;
}

static void AppAdpBufferIndication(struct TAdpBufferIndication *pBufferIndication)
{
	(void)pBufferIndication;
}

static void AppBsJoinIndication(uint8_t *puc_extended_address, uint16_t us_short_address)
{
	(void)puc_extended_address;
	(void)us_short_address;

	dlms_emu_join_node(puc_extended_address, us_short_address);
}

static void AppBsLeaveIndication(uint16_t u16SrcAddr, bool bSecurityEnabled, uint8_t u8LinkQualityIndicator, uint8_t *pNsdu, uint16_t u16NsduLength)
{
	(void)bSecurityEnabled;
	(void)u8LinkQualityIndicator;
	(void)pNsdu;
	(void)u16NsduLength;

	dlms_emu_leave_node(u16SrcAddr);
}

static void AppAdpNotification_UpdNonVolatileDataIndication(void)
{

}

static void InitializeStack(void)
{
	struct TAdpNotifications notifications;
	TBootstrapAdpNotifications *ps_bs_notifications;

	/* Set Bootstrap managment */
	ps_bs_notifications = bs_get_not_handlers();

	/* Set ADP callbacks */
	notifications.fnctAdpMsgStatus = AppAdpMsgStatus;
	notifications.fnctAdpDataConfirm = AppAdpDataConfirm;
	notifications.fnctAdpDataIndication = AppAdpDataIndication;
	notifications.fnctAdpDiscoveryConfirm = AppAdpDiscoveryConfirm;
	notifications.fnctAdpDiscoveryIndication = AppAdpDiscoveryIndication;
	notifications.fnctAdpNetworkStartConfirm = AppAdpNetworkStartConfirm;
	notifications.fnctAdpNetworkJoinConfirm = AppAdpNetworkJoinConfirm;
	notifications.fnctAdpNetworkLeaveIndication = AppAdpNetworkLeaveIndication;
	notifications.fnctAdpNetworkLeaveConfirm = AppAdpNetworkLeaveConfirm;
	notifications.fnctAdpResetConfirm = AppAdpResetConfirm;
	notifications.fnctAdpSetConfirm = AppAdpSetConfirm;
	notifications.fnctAdpMacSetConfirm = AppAdpMacSetConfirm;
	notifications.fnctAdpGetConfirm = AppAdpGetConfirm;
	notifications.fnctAdpMacGetConfirm = AppAdpMacGetConfirm;
	notifications.fnctAdpLbpConfirm = ps_bs_notifications->fnctAdpLbpConfirm;
	notifications.fnctAdpLbpIndication = ps_bs_notifications->fnctAdpLbpIndication;
	notifications.fnctAdpRouteDiscoveryConfirm = AppAdpRouteDiscoveryConfirm;
	notifications.fnctAdpPathDiscoveryConfirm = AppAdpPathDiscoveryConfirm;
	notifications.fnctAdpNetworkStatusIndication = AppAdpNetworkStatusIndication;
	notifications.fnctAdpBufferIndication = AppAdpBufferIndication;
	notifications.fnctAdpUpdNonVolatileDataIndication = AppAdpNotification_UpdNonVolatileDataIndication;
	notifications.fnctAdpPREQIndication = NULL;

	AdpInitialize(&notifications, ADP_BAND_CENELEC_A);
}

static void InitializeModemParameters(void)
{
	g_u8MibInitIndex = 0;

#ifdef DLMS_REPORT
	printf("Start modem initialization.\r\n");
#endif

	if (g_MibSettings[g_u8MibInitIndex].m_u8Type == MIB_ADP) {
		AdpSetRequest(g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index, g_MibSettings[g_u8MibInitIndex].m_u8ValueLength, g_MibSettings[g_u8MibInitIndex].m_pu8Value);
	} else {
		AdpMacSetRequest(g_MibSettings[g_u8MibInitIndex].m_u32Id, g_MibSettings[g_u8MibInitIndex].m_u16Index, g_MibSettings[g_u8MibInitIndex].m_u8ValueLength, g_MibSettings[g_u8MibInitIndex].m_pu8Value);
	}
}

/**
 * \brief Periodic task to process Cycles App. Initialize and start Cycles Application and launch timer
 * to update internal counters.
 *
 */
void adp_mng_process(void)
{
	if (g_CoordInitComplete) {
		/* Init complete */
		dlms_emu_process();
	} else {
		struct t_bs_lbp_set_param_confirm lbp_set_confirm;

		/* Wait until initialization */
		if (g_u8MibInitIndex < g_u8MibTableSize) {
			return;
		}

		g_CoordInitComplete = true;

		/* Init Bootstrap module */
		bs_init();
		bs_lbp_join_ind_set_cb(AppBsJoinIndication);
		bs_lbp_leave_ind_set_cb(AppBsLeaveIndication);

		/* Set PSK */
		bs_lbp_set_param(LBP_IB_PSK, 0, 16, CONF_PSK_KEY, &lbp_set_confirm);

		/* Set GMK */
		bs_lbp_set_param(LBP_IB_GMK, 0, 16, CONF_GMK_KEY, &lbp_set_confirm);

		/* Start G3 Network */
		AdpNetworkStartRequest(G3_COORDINATOR_PAN_ID);

		/* Init DLMS App */
		dlms_emu_init(CONF_EXTENDED_ADDRESS);
	}
}

/**
 * \brief Update the list of registered nodes from Bootstrap module.
 *
 */
uint16_t app_update_registered_nodes(void *pxNodeList)
{
	x_node_list_t *px_list_ptr;
	struct t_bs_lbp_get_param_confirm p_get_confirm;
	uint16_t us_idx, us_num_devices, us_device_cnt;

	px_list_ptr = pxNodeList;

	/* Get the number of devices from Bootstrap module */
	us_num_devices = bs_lbp_get_lbds_counter();
	us_device_cnt = 0;

	/* If no devices found, return */
	if (us_num_devices == 0) {
		return 0;
	}

	/* Update Device Addresses from Bootstrap module */
	for (us_idx = 0; us_idx < MAX_LBDS; us_idx++) {
		bs_lbp_get_param(LBP_IB_DEVICE_LIST, us_idx, &p_get_confirm);
		if (p_get_confirm.uc_status == LBP_STATUS_OK) {
			px_list_ptr->us_short_address = ((uint16_t)p_get_confirm.uc_attribute_value[1]) << 8;
			px_list_ptr->us_short_address += ((uint16_t)p_get_confirm.uc_attribute_value[0]);
			memcpy(px_list_ptr->puc_extended_address, &p_get_confirm.uc_attribute_value[2], sizeof(px_list_ptr->puc_extended_address));
			px_list_ptr++;
			us_device_cnt++;
			if (us_device_cnt == us_num_devices) {
				break;
			}
		}
	}

	return us_num_devices;
}

/*
 * \brief App initialization function.
 *
 *
 */
void adp_mng_init(int _app_id)
{
	si_app_adp_id = _app_id;

	/* Init modules */
	InitializeStack();

	/* Set configuration values for user-specified ADP parameters */
	g_CoordInitComplete = false;
	InitializeModemParameters();
}

/**
 * \brief Process messages received from External Interface.
 * Unpack External Interface protocol command
 */
static void _app_adp_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf++) {
		case DLMS_EMU_CMD_START_CYCLES:
		{
			PRINTF("DLMS EMU: start cycles\n");
		}
    	break;

    case DLMS_EMU_CMD_STOP_CYCLES:
		{
			PRINTF("DLMS EMU: stop cycles\n");
		}
    	break;
    }
}


void adp_mng_callback(socket_ev_info_t *_ev_info)
{
	if (_ev_info->i_socket_fd >= 0) {
		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
			/* Manage LINK */
			si_app_adp_link_fd = _ev_info->i_socket_fd;
			socket_accept_conn(_ev_info);
		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
			/* Receive DATA */
			ssize_t i_bytes;
			si_app_adp_data_fd = _ev_info->i_socket_fd;
			/* Read data from Socket */
			i_bytes = read(_ev_info->i_socket_fd, suc_app_adp_buf, MAX_APP_ADP_SOCKET_SIZE);
			if (i_bytes > 0) {
				_app_adp_rcv_cmd(suc_app_adp_buf, i_bytes);
			} else {
				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
			}
		}
	}
}
