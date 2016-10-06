#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "socket_handler.h"
#include "app_debug.h"
#include "ifaceNet_api.h"
#include "net_info_mng.h"
#include "net_info_report.h"

#ifdef APP_DEBUG_CONSOLE
#	define LOG_APP_DEBUG(a)   printf a
#else
#	define LOG_APP_DEBUG(a)   (void)0
#endif

static int si_net_info_id;
static int si_net_info_link_fd;
static int si_net_info_data_fd;
static unsigned char suc_net_info_buf[MAX_NET_INFO_SOCKET_SIZE];

static x_net_info_t sx_net_info;
static bool sb_pending_path_cfm;

/* Waiting Process timer */
static uint32_t sul_waiting_proc_timer;

/* Timers based on 10 ms */
#define TIMER_TO_REQ_PATH_INFO    500 // 5 seconds


/**
 * \brief Process messages received from External Interface.
 * Unpack External Interface protocol command
 */
static void _net_info_server_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf++) {
    case NET_INFO_WCMD_START_CYCLES:
		{
			PRINTF("Net Info manager: start cycles\n");
		}
    	break;

    case NET_INFO_WCMD_STOP_CYCLES:
		{
			PRINTF("Net Info manager: stop cycles\n");
		}
    	break;

    case NET_INFO_WCMD_GET_ID:
		{
			PRINTF("Net Info manager: get info id\n");
		}
    	break;
    }
}

static void NetInfoGetConfirm(net_info_get_cfm_t *px_cfm_info)
{

}

static void NetInfoEventIndication(net_info_event_ind_t *px_event_info)
{
	uint8_t *puc_ev_info;
	uint16_t us_ev_size;
	uint8_t uc_event;

	/* Check event */
	if (px_event_info->uc_event_id >= NET_INFO_EV_INVALID) {
		return;
	}

	puc_ev_info = px_event_info->puc_event_info;

	us_ev_size = ((uint16_t)*puc_ev_info++) << 8;
	us_ev_size += *puc_ev_info++;

	switch(px_event_info->uc_event_id) {
	case NET_INFO_UPDATE_NODE_LIST:
		/* Update net info */
		sx_net_info.us_num_nodes = ((uint16_t)*puc_ev_info++) << 8;
		sx_net_info.us_num_nodes += *puc_ev_info++;
		memcpy(&sx_net_info.x_node_list, puc_ev_info, sx_net_info.us_num_nodes * 10);

		net_info_report_netlist(&sx_net_info);
		break;

	case NET_INFO_UPDATE_BLACK_LIST:
		/* Update net info */
		sx_net_info.us_black_nodes = ((uint16_t)*puc_ev_info++) << 8;
		sx_net_info.us_black_nodes += *puc_ev_info++;
		memcpy(&sx_net_info.puc_black_list, puc_ev_info, sx_net_info.us_black_nodes << 3);

		net_info_report_blacklist(&sx_net_info);
		break;

	case NET_INFO_UPDATE_PATH_INFO:
	{
		uint8_t uc_status;
		uint16_t us_node_addr;

		/* Free flag */
		sb_pending_path_cfm = false;

		/* Extract path info */
		uc_status = *puc_ev_info++;
		us_node_addr = ((uint16_t)*puc_ev_info++) << 8;
		us_node_addr += *puc_ev_info++;

		/* Set path info validation */
		if ((us_node_addr < 500) && (uc_status == 0)) {
			sx_net_info.b_path_info[us_node_addr] = true;
			sx_net_info.us_num_path_nodes++;
			/* Report path node */
			net_info_report_path_info(us_node_addr, puc_ev_info);
		} else {
			/* Blocking process to send next PREQ message */
			sul_waiting_proc_timer = TIMER_TO_REQ_PATH_INFO;
		}
	}
		break;

	}

}

/**
 * \brief Periodic task to process Cycles App. Initialize and start Cycles Application and launch timer
 * to update internal counters.
 *
 */
void net_info_mng_process(void)
{
	if (sul_waiting_proc_timer) {
		sul_waiting_proc_timer--;
		return;
	}

	if (sb_pending_path_cfm) {
		return;
	}

	if (sx_net_info.us_num_nodes > sx_net_info.us_num_path_nodes) {
		uint16_t us_node_idx;

		/* Search next node to get path info. Pos 0 is reserved for coord. */
		for (us_node_idx = 1; us_node_idx < 500; us_node_idx++) {
			if (sx_net_info.b_path_info[us_node_idx] == false) {
				sb_pending_path_cfm = true;
				NetInfoGetPathRequest(us_node_idx);
				break;
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

	net_info_callbacks.get_confirm = NetInfoGetConfirm;
	net_info_callbacks.event_indication = NetInfoEventIndication;

	NetInfoSetCallbacks(&net_info_callbacks);

	sb_pending_path_cfm = false;
	sul_waiting_proc_timer = 0;
}


void net_info_mng_callback(socket_ev_info_t *_ev_info)
{
	if (_ev_info->i_socket_fd >= 0) {
		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
			/* Manage LINK */
			si_net_info_link_fd = _ev_info->i_socket_fd;
			socket_accept_conn(_ev_info);
		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
			/* Receive DATA */
			ssize_t i_bytes;
			si_net_info_data_fd = _ev_info->i_socket_fd;
			/* Read data from Socket */
			i_bytes = read(_ev_info->i_socket_fd, suc_net_info_buf, MAX_NET_INFO_SOCKET_SIZE);
			if (i_bytes > 0) {
				_net_info_server_rcv_cmd(suc_net_info_buf, i_bytes);
			} else {
				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
			}
		}
	}
}
