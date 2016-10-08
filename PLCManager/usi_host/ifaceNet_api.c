#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ifaceNet_api.h"

#include "hal_utils.h"

/* buffer used to usi serialization */
static uint8_t spuc_serial_if_buf[32];

/* buffer used to web serialization */
static uint8_t spuc_web_if_buf[32];

/* USI Cmd */
x_usi_cmd_t sx_net_info_msg;

static net_info_callbacks_t sx_net_info_cbs;

static uint8_t _net_info_get_cdata(uint8_t *px_msg)
{
	if (sx_net_info_cbs.coordinator_data) {
		sx_net_info_cbs.coordinator_data((net_info_cdata_cfm_t *)px_msg);
	}

    return true;
}

static uint8_t _net_info_get_cfm(uint8_t *px_msg)
{
	net_info_get_cfm_t net_info_get_cfm;
	uint8_t* ptr_info;

	ptr_info = px_msg;

	net_info_get_cfm.uc_id = *ptr_info++;
	net_info_get_cfm.us_len += ((uint16_t)(*ptr_info++)) << 8;
	net_info_get_cfm.us_len += *ptr_info++;
	memcpy(net_info_get_cfm.puc_param_info, ptr_info, net_info_get_cfm.us_len);

	if (sx_net_info_cbs.get_confirm) {
		sx_net_info_cbs.get_confirm(&net_info_get_cfm);
	}

    return true;
}

static uint8_t _net_info_event_indication(uint8_t *px_msg, uint16_t us_len)
{
	net_info_event_ind_t net_info_event_ind;
	uint8_t* ptr_info;

	if (us_len == 0) {
		return false;
	}

	ptr_info = px_msg;

	net_info_event_ind.uc_event_id = *ptr_info++;
	memcpy(net_info_event_ind.puc_event_info, ptr_info, us_len - 1);

	if (sx_net_info_cbs.event_indication) {
		sx_net_info_cbs.event_indication(&net_info_event_ind);
	}

    return true;
}


static uint8_t ifaceNetInfo_api_ReceivedCmd(uint8_t *px_msg, uint16_t us_len)
{
	uint8_t uc_cmd;
	uint8_t *puc_ptr;
	uint16_t us_size_msg;

	puc_ptr = px_msg;
    uc_cmd = (*puc_ptr++) & 0x7F;

    us_size_msg = us_len - 1;

    switch (uc_cmd) {
    case NET_INFO_EVENT_IND:
        return _net_info_event_indication(puc_ptr, us_size_msg);
        break;
    case NET_INFO_RSP_GET_ID:
        return _net_info_get_cfm(puc_ptr);
        break;
    case NET_INFO_RSP_CDATA_ID:
        return _net_info_get_cdata(puc_ptr);
        break;
    default:
        return false;
        break;
    }
}

void ifaceNetInfo_api_init(void)
{
	memset(&sx_net_info_cbs, 0, sizeof(sx_net_info_cbs));

    /* register to USI callback */
    hal_usi_set_callback(PROTOCOL_NET_INFO_G3, ifaceNetInfo_api_ReceivedCmd);

    /* Set USI data to send messages */
    sx_net_info_msg.uc_p_type = PROTOCOL_NET_INFO_G3;
    sx_net_info_msg.puc_buf = spuc_serial_if_buf;
    sx_net_info_msg._fd = -1;
}

void NetInfoSetCallbacks(net_info_callbacks_t *pf_net_info_callback)
{
	memcpy(&sx_net_info_cbs, pf_net_info_callback, sizeof(sx_net_info_cbs));
}

void NetInfoGetRequest(uint8_t uc_id)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_if_buf;

    *puc_msg++ = NET_INFO_CMD_GET_ID;
    *puc_msg++ = uc_id;

    /* Send to USI */
    sx_net_info_msg.us_len = puc_msg - spuc_serial_if_buf;

    hal_usi_send_cmd(&sx_net_info_msg);

}

void NetInfoGetPathRequest(uint16_t us_short_address)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_if_buf;

    *puc_msg++ = NET_INFO_CMD_GET_PATH_REQ;
    *puc_msg++ = (uint8_t)(us_short_address >> 8);
    *puc_msg++ = (uint8_t)us_short_address;

    /* Send to USI */
    sx_net_info_msg.us_len = puc_msg - spuc_serial_if_buf;

    hal_usi_send_cmd(&sx_net_info_msg);

}

void NetInfoCoordinatorData(void)
{
    uint8_t *puc_msg;

    /* Insert parameters */
    puc_msg = spuc_serial_if_buf;

    *puc_msg++ = NET_INFO_CMD_GET_COORD_DATA;

    /* Send to USI */
    sx_net_info_msg.us_len = puc_msg - spuc_serial_if_buf;

    hal_usi_send_cmd(&sx_net_info_msg);

}

