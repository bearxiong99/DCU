#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "AdpApiTypes.h"
#include "AdpApi.h"
#include "ifaceAdp_api.h"

#include "hal_utils.h"

#define ADP_API_MAX_NUM           8
#define ADP_API_INVALID           0xFF

static struct TAdpNotifications sx_g3_adp_api;
static uint8_t suc_g3_adp_app_id_req;

/* buffer used to serialization */
static uint8_t spuc_serial_buf[1024];

/* USI Cmd */
x_usi_cmd_t sx_adp_api_msg;

/* Serialize ADP commands */
void AdpInitialize(struct TAdpNotifications *pNotifications, enum TAdpBand band)
{
    uint8_t *puc_msg;

    /* Catch callback functions */
    memcpy ((uint8_t *)&sx_g3_adp_api, (uint8_t *)pNotifications, sizeof(sx_g3_adp_api));

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_INITIALIZE;
    *puc_msg++ = (uint8_t)band;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.puc_buf = spuc_serial_buf;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);
}

void AdpDataRequest(uint16_t u16NsduLength, const uint8_t *pNsdu,
		uint8_t u8NsduHandle, bool bDiscoverRoute, uint8_t u8QualityOfService)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_DATA_REQUEST;
    *puc_msg++ = u8NsduHandle;
    *puc_msg++ = (uint8_t)bDiscoverRoute;
    *puc_msg++ = u8QualityOfService;
    *puc_msg++ = (uint8_t)(u16NsduLength >> 8);
    *puc_msg++ = (uint8_t)u16NsduLength;
    memcpy(puc_msg, pNsdu, u16NsduLength);
    puc_msg += u16NsduLength;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.puc_buf = spuc_serial_buf;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);
}

void AdpDiscoveryRequest(uint8_t u8Duration)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_DISCOVERY_REQUEST;
    *puc_msg++ = u8Duration;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.puc_buf = spuc_serial_buf;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpNetworkStartRequest(uint16_t u16PanId)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_NETWORK_START_REQUEST;
    *puc_msg++ = (uint8_t)(u16PanId >> 8);
    *puc_msg++ = (uint8_t)u16PanId;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.puc_buf = spuc_serial_buf;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpNetworkJoinRequest(uint16_t u16PanId, uint16_t u16LbaAddress)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_NETWORK_JOIN_REQUEST;
    *puc_msg++ = (uint8_t)(u16PanId >> 8);
    *puc_msg++ = (uint8_t)u16PanId;
    *puc_msg++ = (uint8_t)(u16LbaAddress >> 8);
    *puc_msg++ = (uint8_t)u16LbaAddress;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpNetworkLeaveRequest(void)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_NETWORK_LEAVE_REQUEST;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpResetRequest(void)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_RESET_REQUEST;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpGetRequest(uint32_t u32AttributeId, uint16_t u16AttributeIndex)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_GET_REQUEST;
    *puc_msg++ = (uint8_t)(u32AttributeId >> 24);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 16);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 8);
    *puc_msg++ = (uint8_t)u32AttributeId;
    *puc_msg++ = (uint8_t)(u16AttributeIndex >> 8);
    *puc_msg++ = (uint8_t)u16AttributeIndex;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpMacGetRequest(uint32_t u32AttributeId, uint16_t u16AttributeIndex)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_MAC_GET_REQUEST;
    *puc_msg++ = (uint8_t)(u32AttributeId >> 24);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 16);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 8);
    *puc_msg++ = (uint8_t)u32AttributeId;
    *puc_msg++ = (uint8_t)(u16AttributeIndex >> 8);
    *puc_msg++ = (uint8_t)u16AttributeIndex;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpSetRequest(uint32_t u32AttributeId, uint16_t u16AttributeIndex,	uint8_t u8AttributeLength, const uint8_t *pu8AttributeValue)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_SET_REQUEST;
    *puc_msg++ = (uint8_t)(u32AttributeId >> 24);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 16);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 8);
    *puc_msg++ = (uint8_t)u32AttributeId;
    *puc_msg++ = (uint8_t)(u16AttributeIndex >> 8);
    *puc_msg++ = (uint8_t)u16AttributeIndex;
    *puc_msg++ = u8AttributeLength;
    memcpy(puc_msg, pu8AttributeValue, u8AttributeLength);
    puc_msg += u8AttributeLength;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpMacSetRequest(uint32_t u32AttributeId, uint16_t u16AttributeIndex, uint8_t u8AttributeLength, const uint8_t *pu8AttributeValue)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_MAC_SET_REQUEST;
    *puc_msg++ = (uint8_t)(u32AttributeId >> 24);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 16);
    *puc_msg++ = (uint8_t)(u32AttributeId >> 8);
    *puc_msg++ = (uint8_t)u32AttributeId;
    *puc_msg++ = (uint8_t)(u16AttributeIndex >> 8);
    *puc_msg++ = (uint8_t)u16AttributeIndex;
    *puc_msg++ = u8AttributeLength;
    memcpy(puc_msg, pu8AttributeValue, u8AttributeLength);
    puc_msg += u8AttributeLength;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpRouteDiscoveryRequest(uint16_t u16DstAddr, uint8_t u8MaxHops)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_ROUTE_DISCOVERY_REQUEST;
    *puc_msg++ = (uint8_t)(u16DstAddr >> 8);
    *puc_msg++ = (uint8_t)u16DstAddr;
    *puc_msg++ = u8MaxHops;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpPathDiscoveryRequest(uint16_t u16DstAddr, uint8_t u8MetricType)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_PATH_DISCOVERY_REQUEST;
    *puc_msg++ = (uint8_t)(u16DstAddr >> 8);
    *puc_msg++ = (uint8_t)u16DstAddr;
    *puc_msg++ = u8MetricType;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

void AdpLbpRequest(const struct TAdpAddress *pDstAddr, uint16_t u16NsduLength, uint8_t *pNsdu, uint8_t u8NsduHandle,
		uint8_t u8MaxHops, bool bDiscoveryRoute, uint8_t u8QualityOfService, bool bSecurityEnable)
{
    uint8_t *puc_msg;
    uint8_t uc_len_addr;

    uc_len_addr = sizeof(pDstAddr->m_ExtendedAddress.m_au8Value);

    /* Insert parameters */
    puc_msg = spuc_serial_buf;

    *puc_msg++ = SERIAL_MSG_ADP_LBP_REQUEST;
    *puc_msg++ = u8NsduHandle;
    *puc_msg++ = u8MaxHops;
    *puc_msg++ = (uint8_t)bDiscoveryRoute;
    *puc_msg++ = u8QualityOfService;
    *puc_msg++ = (uint8_t)bSecurityEnable;
    *puc_msg++ = uc_len_addr;
    *puc_msg++ = (uint8_t)(u16NsduLength >> 8);
    *puc_msg++ = (uint8_t)u16NsduLength;
    memcpy(puc_msg, pDstAddr->m_ExtendedAddress.m_au8Value, uc_len_addr);
    puc_msg += uc_len_addr;
    memcpy(puc_msg, pNsdu, u16NsduLength);
    puc_msg += u16NsduLength;

    /* Send to USI */
    sx_adp_api_msg.uc_p_type = PROTOCOL_ADP_G3;
    sx_adp_api_msg.us_len = puc_msg - spuc_serial_buf;
    sx_adp_api_msg._fd = -1;

    hal_usi_send_cmd(&sx_adp_api_msg);

}

/* Decode Serial Callbacks */

uint8_t _adp_msg_status(uint8_t* ptrMsg, uint16_t len)
{
	uint8_t uc_msg_status;

	uc_msg_status = *ptrMsg;

    if (sx_g3_adp_api.fnctAdpMsgStatus) {
    	sx_g3_adp_api.fnctAdpMsgStatus(uc_msg_status);
    }

    return true;
}

uint8_t _adp_data_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpDataConfirm data_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	data_cfm.m_u8Status = *ptr_info++;
	data_cfm.m_u8NsduHandle = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpDataConfirm) {
    	sx_g3_adp_api.fnctAdpDataConfirm(&data_cfm);
    }

    return true;
}

uint8_t _adp_data_ind(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpDataIndication data_ind;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	data_ind.m_u8LinkQualityIndicator = *ptr_info++;
	data_ind.m_u16NsduLength = ((uint16_t)(*ptr_info++)) << 8;
	data_ind.m_u16NsduLength += *ptr_info++;
	if (data_ind.m_u16NsduLength > sizeof(spuc_serial_buf)) {
		memcpy(spuc_serial_buf, ptr_info, sizeof(spuc_serial_buf));
	} else {
		memcpy(spuc_serial_buf, ptr_info, data_ind.m_u16NsduLength);
	}
	data_ind.m_pNsdu = spuc_serial_buf;

    if (sx_g3_adp_api.fnctAdpDataIndication) {
    	sx_g3_adp_api.fnctAdpDataIndication(&data_ind);
    }

    return true;
}


uint8_t _adp_network_status_ind(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpNetworkStatusIndication status_ind;
	uint8_t* ptr_info;
	uint8_t uc_len_addr;

	ptr_info = ptrMsg;

	status_ind.m_u16PanId = ((uint16_t)(*ptr_info++)) << 8;
	status_ind.m_u16PanId += *ptr_info++;
	uc_len_addr = *ptr_info++;
	memcpy(&status_ind.m_SrcDeviceAddress, ptr_info, uc_len_addr);
	ptr_info += uc_len_addr;
	uc_len_addr = *ptr_info++;
	memcpy(&status_ind.m_DstDeviceAddress, ptr_info, uc_len_addr);
	ptr_info += uc_len_addr;
	status_ind.m_u8Status  = *ptr_info++;
	status_ind.m_u8SecurityLevel  = *ptr_info++;
	status_ind.m_u8KeyIndex  = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpNetworkStatusIndication) {
    	sx_g3_adp_api.fnctAdpNetworkStatusIndication(&status_ind);
    }

    return true;
}


uint8_t _adp_network_start_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpNetworkStartConfirm start_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	start_cfm.m_u8Status = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpNetworkStartConfirm) {
    	sx_g3_adp_api.fnctAdpNetworkStartConfirm(&start_cfm);
    }

    return true;
}


uint8_t _adp_network_join_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpNetworkJoinConfirm join_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	join_cfm.m_u8Status = *ptr_info++;
	join_cfm.m_u16NetworkAddress = ((uint16_t)(*ptr_info++)) << 8;
	join_cfm.m_u16NetworkAddress += *ptr_info++;
	join_cfm.m_u16PanId = ((uint16_t)(*ptr_info++)) << 8;
	join_cfm.m_u16PanId += *ptr_info++;

    if (sx_g3_adp_api.fnctAdpNetworkJoinConfirm) {
    	sx_g3_adp_api.fnctAdpNetworkJoinConfirm(&join_cfm);
    }

    return true;
}


uint8_t _adp_network_leave_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpNetworkLeaveConfirm leave_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	leave_cfm.m_u8Status = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpNetworkLeaveConfirm) {
    	sx_g3_adp_api.fnctAdpNetworkLeaveConfirm(&leave_cfm);
    }

    return true;
}


uint8_t _adp_network_leave_ind(uint8_t* ptrMsg, uint16_t len)
{
    if (sx_g3_adp_api.fnctAdpNetworkLeaveIndication) {
    	sx_g3_adp_api.fnctAdpNetworkLeaveIndication();
    }

    return true;
}


uint8_t _adp_reset_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpResetConfirm reset_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	reset_cfm.m_u8Status = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpResetConfirm) {
    	sx_g3_adp_api.fnctAdpResetConfirm(&reset_cfm);
    }

    return true;
}


uint8_t _adp_set_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpSetConfirm set_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	set_cfm.m_u8Status = *ptr_info++;
	set_cfm.m_u32AttributeId = ((uint32_t)(*ptr_info++)) << 24;
	set_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 16;
	set_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 8;
	set_cfm.m_u32AttributeId += *ptr_info++;
	set_cfm.m_u16AttributeIndex = ((uint16_t)(*ptr_info++)) << 8;
	set_cfm.m_u16AttributeIndex += *ptr_info++;

    if (sx_g3_adp_api.fnctAdpSetConfirm) {
    	sx_g3_adp_api.fnctAdpSetConfirm(&set_cfm);
    }

    return true;
}


uint8_t _adp_get_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpGetConfirm get_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	get_cfm.m_u8Status = *ptr_info++;
	get_cfm.m_u32AttributeId = ((uint32_t)(*ptr_info++)) << 24;
	get_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 16;
	get_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 8;
	get_cfm.m_u32AttributeId += *ptr_info++;
	get_cfm.m_u16AttributeIndex = ((uint16_t)(*ptr_info++)) << 8;
	get_cfm.m_u16AttributeIndex += *ptr_info++;
	get_cfm.m_u8AttributeLength = *ptr_info++;
	memcpy(get_cfm.m_au8AttributeValue, ptr_info, get_cfm.m_u8AttributeLength);

    if (sx_g3_adp_api.fnctAdpGetConfirm) {
    	sx_g3_adp_api.fnctAdpGetConfirm(&get_cfm);
    }

    return true;
}


uint8_t _adp_lbp_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpLbpConfirm lbp_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	lbp_cfm.m_u8Status = *ptr_info++;
	lbp_cfm.m_u8NsduHandle = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpLbpConfirm) {
    	sx_g3_adp_api.fnctAdpLbpConfirm(&lbp_cfm);
    }

    return true;
}


uint8_t _adp_lbp_ind(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpLbpIndication lbp_ind;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	lbp_ind.m_u16SrcAddr = ((uint16_t)(*ptr_info++)) << 8;
	lbp_ind.m_u16SrcAddr += *ptr_info++;
	lbp_ind.m_u16NsduLength = ((uint16_t)(*ptr_info++)) << 8;
	lbp_ind.m_u16NsduLength += *ptr_info++;
	memcpy(spuc_serial_buf, ptr_info, lbp_ind.m_u16NsduLength);
	ptr_info += lbp_ind.m_u16NsduLength;
	lbp_ind.m_pNsdu = spuc_serial_buf;
	lbp_ind.m_u8LinkQualityIndicator = *ptr_info++;
	lbp_ind.m_bSecurityEnabled = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpLbpIndication) {
    	sx_g3_adp_api.fnctAdpLbpIndication(&lbp_ind);
    }

    return true;
}


uint8_t _adp_route_discovery_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpRouteDiscoveryConfirm route_disc_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	route_disc_cfm.m_u8Status = *ptr_info++;

    if (sx_g3_adp_api.fnctAdpRouteDiscoveryConfirm) {
    	sx_g3_adp_api.fnctAdpRouteDiscoveryConfirm(&route_disc_cfm);
    }

    return true;
}


uint8_t _adp_route_path_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpPathDiscoveryConfirm route_path_cfm;
	uint8_t* ptr_info;
	uint8_t uc_idx;

	ptr_info = ptrMsg;

	route_path_cfm.m_u8Status = *ptr_info++;
	route_path_cfm.m_u16DstAddr = ((uint16_t)(*ptr_info++)) << 8;
	route_path_cfm.m_u16DstAddr += *ptr_info++;
	route_path_cfm.m_u16OrigAddr = ((uint16_t)(*ptr_info++)) << 8;
	route_path_cfm.m_u16OrigAddr += *ptr_info++;
	route_path_cfm.m_u8MetricType = *ptr_info++;
	route_path_cfm.m_u8ForwardHopsCount = *ptr_info++;
	route_path_cfm.m_u8ReverseHopsCount = *ptr_info++;

	for (uc_idx = 0; uc_idx < route_path_cfm.m_u8ForwardHopsCount; uc_idx++) {
		route_path_cfm.m_aForwardPath[uc_idx].m_u16HopAddress = ((uint16_t)(*ptr_info++)) << 8;
		route_path_cfm.m_aForwardPath[uc_idx].m_u16HopAddress += *ptr_info++;
		route_path_cfm.m_aForwardPath[uc_idx].m_u8Mns += *ptr_info++;
		route_path_cfm.m_aForwardPath[uc_idx].m_u8LinkCost += *ptr_info++;
	}

	for (uc_idx = 0; uc_idx < route_path_cfm.m_u8ReverseHopsCount; uc_idx++) {
		route_path_cfm.m_aReversePath[uc_idx].m_u16HopAddress = ((uint16_t)(*ptr_info++)) << 8;
		route_path_cfm.m_aReversePath[uc_idx].m_u16HopAddress += *ptr_info++;
		route_path_cfm.m_aReversePath[uc_idx].m_u8Mns += *ptr_info++;
		route_path_cfm.m_aReversePath[uc_idx].m_u8LinkCost += *ptr_info++;
	}

    if (sx_g3_adp_api.fnctAdpPathDiscoveryConfirm) {
    	sx_g3_adp_api.fnctAdpPathDiscoveryConfirm(&route_path_cfm);
    }

    return true;
}


uint8_t _adp_mac_set_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpMacSetConfirm mac_set_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	mac_set_cfm.m_u8Status = *ptr_info++;
	mac_set_cfm.m_u32AttributeId = ((uint32_t)(*ptr_info++)) << 24;
	mac_set_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 16;
	mac_set_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 8;
	mac_set_cfm.m_u32AttributeId += *ptr_info++;
	mac_set_cfm.m_u16AttributeIndex = ((uint16_t)(*ptr_info++)) << 8;
	mac_set_cfm.m_u16AttributeIndex += *ptr_info++;

    if (sx_g3_adp_api.fnctAdpMacSetConfirm) {
    	sx_g3_adp_api.fnctAdpMacSetConfirm(&mac_set_cfm);
    }

    return true;
}


uint8_t _adp_mac_get_cfm(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpMacGetConfirm mac_get_cfm;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	mac_get_cfm.m_u8Status = *ptr_info++;
	mac_get_cfm.m_u32AttributeId = ((uint32_t)(*ptr_info++)) << 24;
	mac_get_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 16;
	mac_get_cfm.m_u32AttributeId += ((uint32_t)(*ptr_info++)) << 8;
	mac_get_cfm.m_u32AttributeId += *ptr_info++;
	mac_get_cfm.m_u16AttributeIndex = ((uint16_t)(*ptr_info++)) << 8;
	mac_get_cfm.m_u16AttributeIndex += *ptr_info++;
	mac_get_cfm.m_u8AttributeLength = *ptr_info++;
	memcpy(mac_get_cfm.m_au8AttributeValue, ptr_info, mac_get_cfm.m_u8AttributeLength);

    if (sx_g3_adp_api.fnctAdpMacGetConfirm) {
    	sx_g3_adp_api.fnctAdpMacGetConfirm(&mac_get_cfm);
    }

    return true;
}


uint8_t _adp_buffer_ind(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpBufferIndication buf_ind;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	buf_ind.m_bBufferReady = *ptr_info;

    if (sx_g3_adp_api.fnctAdpBufferIndication) {
    	sx_g3_adp_api.fnctAdpBufferIndication(&buf_ind);
    }

    return true;
}


uint8_t _adp_discovery_ind(uint8_t* ptrMsg, uint16_t len)
{
	struct TAdpPanDescriptor pan_desc;
	uint8_t* ptr_info;

	ptr_info = ptrMsg;

	pan_desc.m_u16PanId = ((uint16_t)(*ptr_info++)) << 8;
	pan_desc.m_u16PanId += *ptr_info++;
	pan_desc.m_u8LinkQuality = *ptr_info++;
	pan_desc.m_u16LbaAddress = ((uint16_t)(*ptr_info++)) << 8;
	pan_desc.m_u16LbaAddress += *ptr_info++;
	pan_desc.m_u16RcCoord = ((uint16_t)(*ptr_info++)) << 8;
	pan_desc.m_u16RcCoord += *ptr_info++;

    if (sx_g3_adp_api.fnctAdpDiscoveryIndication) {
    	sx_g3_adp_api.fnctAdpDiscoveryIndication(&pan_desc);
    }

    return true;
}


uint8_t _adp_discovery_cfm(uint8_t* ptrMsg, uint16_t len)
{
	uint8_t m_u8Status;

	m_u8Status= *ptrMsg;

    if (sx_g3_adp_api.fnctAdpDiscoveryConfirm) {
    	sx_g3_adp_api.fnctAdpDiscoveryConfirm(m_u8Status);
    }

    return true;
}


uint8_t ifaceAdp_api_ReceivedCmd(uint8_t* ptrMsg, uint16_t len)
{
	enum EAdpApi uc_cmd;

    uc_cmd = (enum EAdpApi)((*ptrMsg++) & 0x7F);
    len --;

    switch (uc_cmd) {
    case SERIAL_MSG_ADP_STATUS:
        return _adp_msg_status(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_DATA_CONFIRM:
        return _adp_data_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_DATA_INDICATION:
        return _adp_data_ind(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_NETWORK_STATUS_INDICATION:
        return _adp_network_status_ind(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_DISCOVERY_CONFIRM:
        return _adp_discovery_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_NETWORK_START_CONFIRM:
        return _adp_network_start_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_NETWORK_JOIN_CONFIRM:
        return _adp_network_join_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_NETWORK_LEAVE_CONFIRM:
        return _adp_network_leave_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_NETWORK_LEAVE_INDICATION:
        return _adp_network_leave_ind(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_RESET_CONFIRM:
        return _adp_reset_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_SET_CONFIRM:
        return _adp_set_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_GET_CONFIRM:
        return _adp_get_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_LBP_CONFIRM:
        return _adp_lbp_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_LBP_INDICATION:
        return _adp_lbp_ind(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_ROUTE_DISCOVERY_CONFIRM:
        return _adp_route_discovery_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_PATH_DISCOVERY_CONFIRM:
        return _adp_route_path_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_MAC_SET_CONFIRM:
        return _adp_mac_set_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_MAC_GET_CONFIRM:
        return _adp_mac_get_cfm(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_BUFFER_INDICATION:
        return _adp_buffer_ind(ptrMsg, len);
        break;
    case SERIAL_MSG_ADP_DISCOVERY_INDICATION:
        return _adp_discovery_ind(ptrMsg, len);
        break;
    default:
        return false;
        break;
    }
}

void ifaceAdp_api_init()
{
    memset(&sx_g3_adp_api, 0, sizeof(sx_g3_adp_api));
    suc_g3_adp_app_id_req = ADP_API_INVALID;

    /* register to USI callback */
    hal_usi_set_callback(PROTOCOL_ADP_G3, ifaceAdp_api_ReceivedCmd);
}

void ifaceAdp_select_api(uint8_t adp_app_id)
{
	if (adp_app_id < ADP_API_MAX_NUM) {
		suc_g3_adp_app_id_req = adp_app_id;
	} else {
		suc_g3_adp_app_id_req = ADP_API_INVALID;
	}
}
