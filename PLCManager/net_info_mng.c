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
#include "usi_host.h"
#include "ifaceNet_api.h"
#include "net_info_mng.h"

#ifdef APP_DEBUG_CONSOLE
#	define LOG_APP_DEBUG(a)   printf a
#else
#	define LOG_APP_DEBUG(a)   (void)0
#endif

static int si_net_info_id;
static int si_net_usi_port_fd;
static int si_net_info_link_fd;
static int si_net_info_data_fd;
static unsigned char suc_net_info_buf[MAX_NET_INFO_SOCKET_SIZE];


/**
 * \brief Process messages received from External Interface.
 * Unpack External Interface protocol command
 */
static void _net_info_server_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf++) {
    case NET_INFO_CMD_START_CYCLES:
		{
			PRINTF("Net Info manager: start cycles\n");
		}
    	break;

    case NET_INFO_CMD_STOP_CYCLES:
		{
			PRINTF("Net Info manager: stop cycles\n");
		}
    	break;

    case NET_INFO_CMD_GET_ID:
		{
			PRINTF("Net Info manager: get info id\n");
			NetInfoGetRequest(*puc_buf++);
		}
    	break;
    }
}

/**
 * \brief Process messages received from External Interface.
 * Unpack External Interface protocol command
 */
static void _net_info_usi_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf++) {
    case NET_INFO_RSP_START_OK:
		{
			PRINTF("Net Info manager: start cycles\n");
		}
    	break;

    case NET_INFO_RSP_STOP_OK:
		{
			PRINTF("Net Info manager: stop cycles\n");
		}
    	break;

    case NET_INFO_RSP_GET_ID:
		{
			PRINTF("Net Info manager: get info id\n");
			NetInfoGetRequest(*puc_buf++);
		}
    	break;
    }
}


static void NetInfoGetConfirm(net_info_get_cfm_t *px_cfm_info)
{

}

static void NetInfoEventIndication(net_info_event_ind_t *px_event_info)
{

}

/**
 * \brief Periodic task to process Cycles App. Initialize and start Cycles Application and launch timer
 * to update internal counters.
 *
 */
static uint8_t suc_sim_rcv = 0;
void net_info_mng_process(void)
{
	uint8_t puc_sim_buff[10];

	puc_sim_buff[0] = NET_INFO_CMD_STOP_CYCLES;

	if (suc_sim_rcv) {
		_net_info_server_rcv_cmd(puc_sim_buff, sizeof(puc_sim_buff));
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

	net_info_callbacks.get_confirm = NetInfoGetConfirm;
	net_info_callbacks.event_indication = NetInfoEventIndication;

	NetInfoSetCallbacks(&net_info_callbacks);
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
