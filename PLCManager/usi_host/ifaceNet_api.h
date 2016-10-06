#ifndef IFACENET_API_H
#define IFACENET_API_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define NET_INFO_MAX_VALUE_LENGTH (512)

enum net_info_commands {
	NET_INFO_CMD_GET_ID,
	NET_INFO_CMD_GET_PATH_REQ,
	NET_INFO_CMD_2,
	NET_INFO_CMD_INVALD
};

enum net_info_responses {
	NET_INFO_EVENT_IND,
	NET_INFO_RSP_GET_ID,
	NET_INFO_RSP_INVALD
};

typedef enum net_info_events {
	NET_INFO_UPDATE_NODE_LIST = 0,
	NET_INFO_UPDATE_BLACK_LIST,
	NET_INFO_UPDATE_PATH_INFO,
	NET_INFO_EV_INVALID
} net_info_events_t;

typedef struct net_info_get_cfm {
	uint8_t uc_id;
	uint16_t us_len;
	uint8_t puc_param_info[NET_INFO_MAX_VALUE_LENGTH];
} net_info_get_cfm_t;

typedef struct net_info_event_ind {
	uint8_t uc_event_id;
	uint8_t puc_event_info[NET_INFO_MAX_VALUE_LENGTH];
} net_info_event_ind_t;

typedef void (*pf_get_confirm_t)(net_info_get_cfm_t *px_cfm_info);
typedef void (*pf_event_indication_t)(net_info_event_ind_t *px_ev_info);

/* Net Info Callbacks */
typedef struct net_info_callbacks {
	/* Callback for TX Data Confirm */
	pf_get_confirm_t get_confirm;
	/* Callback for RX Data Indication */
	pf_event_indication_t event_indication;
} net_info_callbacks_t;


void ifaceNetInfo_api_init(void);

void NetInfoSetCallbacks(net_info_callbacks_t *pf_net_info_callback);
void NetInfoGetRequest(uint8_t uc_id);
void NetInfoGetPathRequest(uint16_t us_short_address);

#endif // IFACENET_API_H
