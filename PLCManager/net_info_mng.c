#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "socket_handler.h"
#include "ifaceNet_api.h"
#include "net_info_mng.h"
#include "net_info_report.h"
#include "http_mng.h"

#ifdef NET_INFO_DEBUG_CONSOLE
#	define LOG_NET_INFO_DEBUG(a)   printf a
#else
#	define LOG_NET_INFO_DEBUG(a)   (void)0
#endif

static int si_net_info_id;
static int si_net_info_link_fd;
static int si_net_info_data_fd;
static unsigned char suc_net_info_buf[MAX_NET_INFO_SOCKET_SIZE];

static net_info_cdata_cfm_t sx_coord_data;
static x_net_info_t sx_net_info;
static x_net_statistics_t sx_net_statistics;
static x_routes_info_t sx_routes_info;

static uint16_t sus_node_addr_path_req;
static bool sb_pending_preq_cfm;
static bool sb_pending_cdata_cfm;


/* Waiting Process timer */
static uint32_t sul_waiting_cdata_timer;
static uint32_t sul_waiting_preq_timer;

/* Timers based on 10 ms */
#define TIMER_TO_CDATA_INFO       500 // 5 seconds
#define TIMER_TO_REQ_PATH_INFO    500 // 5 seconds

/* Statistics */
static uint32_t sul_preq_requests;

static void _start_preq_next_request(void)
{
	sb_pending_preq_cfm = true;
	sul_waiting_preq_timer = TIMER_TO_REQ_PATH_INFO;
	LOG_NET_INFO_DEBUG(("_start_preq_next_request (node 0x%04x)\r\n", sus_node_addr_path_req));
}

static void _cancel_preq_next_request(void)
{
	sb_pending_preq_cfm = false;
	sul_waiting_preq_timer = 0;
	LOG_NET_INFO_DEBUG(("_cancel_preq_next_request\r\n"));
}

static uint16_t _extract_u16(void *vptr_value) {
	uint16_t us_val_swap;
	uint8_t uc_val_tmp;

	uc_val_tmp = *(uint8_t *)vptr_value;
	us_val_swap = (uint16_t)uc_val_tmp;

	uc_val_tmp = *((uint8_t *)vptr_value + 1);
	us_val_swap += ((uint16_t)uc_val_tmp) << 8;

	return us_val_swap;
}

static uint16_t _get_node_idx_from_node_address(uint16_t us_node_addr)
{
	uint16_t us_node_idx;

	for (us_node_idx = 0; us_node_idx < NUM_MAX_NODES; us_node_idx++) {
		if (sx_net_info.x_node_list[us_node_idx].us_short_address == us_node_addr) {
			return us_node_idx;
		}
	}

	return INVALID_NODE_ADDRESS;
}

static uint16_t _get_node_idx_from_routes(uint16_t us_path_idx)
{
	uint8_t *puc_ext_addr;
	uint8_t *puc_ext_addr2;
	uint16_t us_node_idx;

	puc_ext_addr = &sx_routes_info.puc_ext_addr[us_path_idx][0];

	for (us_node_idx = 0; us_node_idx < NUM_MAX_NODES; us_node_idx++) {
		puc_ext_addr2 = sx_net_info.x_node_list[us_node_idx].puc_extended_address;
		if (memcmp(puc_ext_addr, puc_ext_addr2, EXT_ADDRESS_SIZE) == 0) {
			return us_node_idx;
		}
	}

	return INVALID_NODE_ADDRESS;
}

static uint16_t _get_path_idx_from_netinfo(uint16_t us_node_idx)
{
	uint8_t *puc_ext_addr;
	uint8_t *puc_ext_addr2;
	uint16_t us_path_idx;
	uint16_t us_first_available;

	if (sx_net_info.us_num_path_nodes == 0) {
		return 0;
	}

	puc_ext_addr = sx_net_info.x_node_list[us_node_idx].puc_extended_address;

	us_first_available = INVALID_NODE_ADDRESS;

	for (us_path_idx = 0; us_path_idx < NUM_MAX_NODES; us_path_idx++) {
		if ((sx_routes_info.b_path_is_valid[us_path_idx] == false) && (us_first_available == INVALID_NODE_ADDRESS)) {
			/* Available to new path info */
			us_first_available = us_path_idx;
		}
		/* Compare with Extended address */
		puc_ext_addr2 = &sx_routes_info.puc_ext_addr[us_path_idx][0];
		if (memcmp(puc_ext_addr, puc_ext_addr2, EXT_ADDRESS_SIZE) == 0) {
			/* return path info */
			return us_path_idx;
		}
	}

	/* return first available, error if not available */
	return us_first_available;
}

static void _clear_path_info(uint16_t us_node_addr)
{
	if (us_node_addr == 0) {
		/* Init complete routes info */
		memset(&sx_routes_info, 0, sizeof(sx_routes_info));
	} else {
		uint16_t us_node_idx, us_path_idx;

		/* Search Node Idx */
		for (us_node_idx = 0; us_node_idx < NUM_MAX_NODES; us_node_idx++) {
			if (sx_net_info.x_node_list[us_node_idx].us_short_address == us_node_addr) {

				/* Get path Idx */
				us_path_idx = _get_path_idx_from_netinfo(us_node_idx);

				/* Decrease if it is valid before and same short address */
				if (sx_routes_info.b_path_is_valid[us_path_idx] == true) {
					sx_net_info.us_num_path_nodes--;
				}

				sx_routes_info.b_path_is_valid[us_path_idx] = false;
				break;
			}
		}
	}
}

static void NetInfoGetConfirm(net_info_get_cfm_t *px_cfm_info)
{

}

static void NetInfoEventIndication(net_info_event_ind_t *px_event_info)
{
	uint8_t *puc_ev_info;
	uint16_t us_ev_size;

	/* Check event */
	if (px_event_info->uc_event_id >= NET_INFO_EV_INVALID) {
		return;
	}

	puc_ev_info = px_event_info->puc_event_info;

	us_ev_size = _extract_u16(puc_ev_info);
	puc_ev_info += 2;

	switch(px_event_info->uc_event_id) {
	case NET_INFO_UPDATE_NODE_LIST:
	{
		uint16_t us_node_addr;
		char puc_http_cmd[10];
		uint16_t us_data_size;

		/* Update net info */
		sx_net_info.us_num_nodes = _extract_u16(puc_ev_info);
		puc_ev_info += 2;

		us_node_addr = _extract_u16(puc_ev_info);
		puc_ev_info += 2;

		if (sx_net_info.us_num_nodes > 0) {
			memcpy(&sx_net_info.x_node_list, puc_ev_info, sx_net_info.us_num_nodes * 10);
		}

		/* Clear PATH info */
		_clear_path_info(us_node_addr);

		net_info_report_netlist(&sx_net_info);

		/* Check Validity of Coordinator Data */
		if (sx_coord_data.b_is_valid == false) {
			/* get coordinator data */
			NetInfoCoordinatorData();
			/* Blocking process to wait Coord Data Msg */
			sul_waiting_cdata_timer = TIMER_TO_CDATA_INFO;
			sb_pending_cdata_cfm = true;
		}

		/* In case of path needed -> wait time */
		if (sx_net_info.us_num_nodes > sx_net_info.us_num_path_nodes) {
			sus_node_addr_path_req = us_node_addr;
			_start_preq_next_request();
		}

		us_data_size = sprintf(puc_http_cmd, "%u", sx_net_info.us_num_nodes);

		/* Update Web server */
		net_info_report_dashboard(&sx_net_info, &sx_net_statistics);
		http_mng_send_cmd(LNXCMS_UPDATE_NUM_DEV, &sx_net_info.us_num_nodes);


	}
		break;

	case NET_INFO_UPDATE_BLACK_LIST:
		/* Update net info */
		sx_net_info.us_black_nodes = _extract_u16(puc_ev_info);
		puc_ev_info += 2;
		memcpy(&sx_net_info.puc_black_list, puc_ev_info, sx_net_info.us_black_nodes << 3);

		net_info_report_blacklist(&sx_net_info);
		break;

	case NET_INFO_UPDATE_PATH_INFO:
	{
		uint16_t us_node_addr, us_node_idx, us_path_idx;
		uint8_t *puc_ext_addr;
		x_path_info_t *px_path_info;

		us_node_idx = _get_node_idx_from_node_address(sus_node_addr_path_req);
		us_path_idx = _get_path_idx_from_netinfo(us_node_idx);
		if (us_path_idx == INVALID_NODE_ADDRESS) {
			/* ERROR */
			break;
		}
		us_node_addr = sx_net_info.x_node_list[us_node_idx].us_short_address;
		puc_ext_addr = &sx_net_info.x_node_list[us_node_idx].puc_extended_address[0];

		/* Extract path info */
		px_path_info = (x_path_info_t *)puc_ev_info;

		/* Set path info validation */
		if ((us_node_idx < NUM_MAX_NODES) && (px_path_info->m_u8Status == 0)) {
			/* Update statistics */
			sx_net_statistics.us_num_path_succ++;

			/* Cancel next programmed request */
			_cancel_preq_next_request();

			/* Store path info of the node */
			memcpy(&sx_routes_info.x_path_info[us_path_idx], px_path_info, sizeof(x_path_info_t));
			sx_routes_info.b_path_is_valid[us_path_idx] = true;
			memcpy(&sx_routes_info.puc_ext_addr[us_path_idx][0], puc_ext_addr, EXT_ADDRESS_SIZE);
			sx_net_info.us_num_path_nodes++;

			/* reset node to request */
			sus_node_addr_path_req = INVALID_NODE_ADDRESS;

			/* Report Pathlist */
			net_info_report_pathlist(&sx_net_info, &sx_routes_info);

			/* Update Web server */
			net_info_report_dashboard(&sx_net_info, &sx_net_statistics);
			http_mng_send_cmd(LNXCMS_UPDATE_NUM_DEV, &sx_net_info.us_num_nodes);
		}
	}
		break;

	}

}

static void NetInfoCoordData(net_info_cdata_cfm_t *px_cdata)
{
	memcpy(&sx_coord_data, px_cdata, sizeof(sx_coord_data));

	/* Update Net Info */
	memcpy(sx_net_info.puc_extended_addr, sx_coord_data.puc_ext_addr, sizeof(sx_net_info.puc_extended_addr));

	if (sb_pending_cdata_cfm) {
		sb_pending_cdata_cfm = false;
		sul_waiting_cdata_timer = 0;
	}
}

/**
 * \brief Periodic task to process Cycles App. Initialize and start Cycles Application and launch timer
 * to update internal counters.
 *
 */
void net_info_mng_process(void)
{

	if (sul_waiting_cdata_timer) {
		sul_waiting_cdata_timer--;
	}

	if (sul_waiting_preq_timer) {
		/* Blocking timer */
		sul_waiting_preq_timer--;
		return;
	}

	if (sb_pending_preq_cfm) {
		/* Update statistics */
		sx_net_statistics.us_num_path_req++;
		NetInfoGetPathRequest(sus_node_addr_path_req);
		_start_preq_next_request();
	} else {
		// Check nodes and paths integrity
		if (sx_net_info.us_num_path_nodes < sx_net_info.us_num_nodes) {
			uint16_t us_node_idx, us_path_idx;
			x_node_list_t *px_node;
			bool b_check_next_node;

			b_check_next_node = true;

			for (us_node_idx = 0; us_node_idx < NUM_MAX_NODES; us_node_idx++) {
				px_node = &sx_net_info.x_node_list[us_node_idx];
				for (us_path_idx = 0; us_path_idx < NUM_MAX_NODES; us_path_idx++) {
					if (memcmp(px_node->puc_extended_address,
							sx_routes_info.puc_ext_addr[us_path_idx], EXT_ADDRESS_SIZE) == 0) {
						if (sx_routes_info.b_path_is_valid[us_path_idx]) {
							/* Find valid route for this node -> check next node */
						} else {
							/* Find INVALID route for this node -> request */
							sx_net_statistics.us_num_path_req++;
							NetInfoGetPathRequest(px_node->us_short_address);
							_start_preq_next_request();
							b_check_next_node = false;
						}
						break;
					}

					if (us_path_idx == sx_net_info.us_num_path_nodes) {
						/* no more path to check for this node */
						break;
					}
				}

				if (!b_check_next_node) {
					break;
				}
			}
		}
	}

}

/*
 * \brief App initialization function.
 *
 *
 */
void net_info_mng_init(int _app_id)
{
	net_info_callbacks_t net_info_callbacks;

	si_net_info_id = _app_id;

	memset(&sx_net_info, 0, sizeof(sx_net_info));
	memset(&sx_coord_data, 0, sizeof(sx_coord_data));
	memset(&sx_routes_info, 0, sizeof(sx_routes_info));

	net_info_callbacks.get_confirm = NetInfoGetConfirm;
	net_info_callbacks.event_indication = NetInfoEventIndication;
	net_info_callbacks.coordinator_data = NetInfoCoordData;

	NetInfoSetCallbacks(&net_info_callbacks);

	sb_pending_preq_cfm = false;
	sb_pending_cdata_cfm = false;
	sus_node_addr_path_req = INVALID_NODE_ADDRESS;
	sul_waiting_cdata_timer = 0;
	sul_waiting_preq_timer = 0;

	/* Init statistics */
	memset(&sx_net_statistics, 0, sizeof(sx_net_statistics));

	/* reset reports */
	net_info_report_pathlist(&sx_net_info, &sx_routes_info);

}

//
//void net_info_mng_callback(socket_ev_info_t *_ev_info)
//{
//	if (_ev_info->i_socket_fd >= 0) {
//		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
//			/* Manage LINK */
//			si_net_info_link_fd = _ev_info->i_socket_fd;
//			socket_accept_conn(_ev_info);
//		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
//			/* Receive DATA */
//			ssize_t i_bytes;
//			si_net_info_data_fd = _ev_info->i_socket_fd;
//			/* Read data from Socket */
//			i_bytes = read(_ev_info->i_socket_fd, suc_net_info_buf, MAX_NET_INFO_SOCKET_SIZE);
//			if (i_bytes > 0) {
//				_net_info_server_rcv_cmd(suc_net_info_buf, i_bytes);
//			} else {
//				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
//			}
//		}
//	}
//}
//


/**
 */
void net_info_webcmd_process(uint8_t* buf)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf) {
    case WEBCMD_UPDATE_DASHBOARD:
		{
			LOG_NET_INFO_DEBUG(("Net Info manager: update dash board info\n"));
			net_info_report_dashboard(&sx_net_info, &sx_net_statistics);
			http_mng_send_cmd(LNXCMS_UPDATE_NUM_DEV, 0);
		}
    	break;
    }
}
