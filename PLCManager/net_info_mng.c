#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/stat.h>

#include "sys/time.h"

#include "config.h"
#include "socket_handler.h"
#include "ifaceNet_api.h"
#include "net_info_mng.h"
#include "net_info_report.h"
#include "http_mng.h"
#include "gpio.h"

#ifdef NET_INFO_DEBUG_CONSOLE
#	define LOG_NET_INFO_DEBUG(a)   printf a
#else
#	define LOG_NET_INFO_DEBUG(a)   (void)0
#endif

static int si_net_info_id;

/* Coordinator info */
static x_coord_data_t sx_coord_data;

/* Connection status */
static uint16_t sus_num_devices;
static x_dev_addr_t spx_current_addr_list[NUM_MAX_NODES];
static char spuc_ppp_iface_file[] = "/sys/class/net/ppp0/statistics/rx_packets";
//static char syscmd_ppp_down[] =  "killall pppd";
//static char syscmd_ppp_up[] =  "pppd call g3 /dev/ttyS2 115200 &";

static char syscmd_ppp_down[] =  "/etc/init.d/PLCpppUp stop";
static char syscmd_ppp_up[] =  "/etc/init.d/PLCpppUp start";

static bool sb_pending_preq_cfm;
static bool sb_pending_cdata_cfm;

/* Waiting Process timer */
static uint32_t sul_waiting_cdata_timer;
static uint32_t sul_waiting_preq_timer;

/* Timers based on 10 ms */
#define TIMER_TO_CDATA_INFO       500 // 5 seconds
#define TIMER_TO_REQ_PATH_INFO    500 // 5 seconds

static void _init_reset_plc(void)
{
	/* Configure ERASE pinout */
	GPIOExport(ERASE_GPIO_ID);
	GPIODirection(ERASE_GPIO_ID, GPIO_OUT);
	GPIOWrite(ERASE_GPIO_ID, ERASE_GPIO_DISABLE);
	usleep(500);
	/* Configure RESET */
	GPIOExport(RESET_GPIO_ID);
	GPIODirection(RESET_GPIO_ID, GPIO_OUT);

	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_DISABLE);
	usleep(500);
}

static void _reset_plc(void)
{
	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_ENABLE);
	usleep(500);

	GPIOWrite(RESET_GPIO_ID, RESET_GPIO_DISABLE);
	usleep(5000);
}

//static int _get_timestamp_ms(void)
//{
//    struct timeval te;
//    int ms;
//
//    gettimeofday(&te, NULL);
//    ms = te.tv_sec*1000 + te.tv_usec/1000;
//
//    return ms;
//}

static void _ppp0_iface_down(void)
{
	system(syscmd_ppp_down);
	sleep(2);
}

static void _ppp0_iface_up(void)
{
	system(syscmd_ppp_up);
	sleep(5);
}


static void _reset_node_list(void)
{
	memset(spx_current_addr_list, 0, sizeof(spx_current_addr_list));
	sus_num_devices = 0;
	/* update reports for NULL node list */
	net_info_report_devlist(spx_current_addr_list, sus_num_devices);
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

static void _join_node(uint16_t us_short_address, uint8_t *puc_extended_address)
{
	uint8_t puc_ext_addr_ascii[24];
	uint16_t us_node_idx;
	bool b_already_connected = false;

	sprintf((char*)puc_ext_addr_ascii, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X",
			puc_extended_address[0], puc_extended_address[1], puc_extended_address[2],
			puc_extended_address[3], puc_extended_address[4], puc_extended_address[5],
			puc_extended_address[6], puc_extended_address[7]);

	LOG_NET_INFO_DEBUG(("[NET_INFO] _join_node: short_address = %hu extended addr %s\n", us_short_address, puc_ext_addr_ascii));

	/* Search if node is already in cycle list */
	for (us_node_idx = 0; us_node_idx < NUM_MAX_NODES; us_node_idx++) {
		if (!memcmp(spx_current_addr_list[us_node_idx].puc_ext_addr, puc_extended_address, 8)) {
			b_already_connected = true;
			spx_current_addr_list[us_node_idx].us_short_addr = us_short_address;
			break;
		}
	}

	if (!b_already_connected) {
		/* Add node to cycle list */
		for (us_node_idx = 0; us_node_idx < NUM_MAX_NODES; us_node_idx++) {
			if (spx_current_addr_list[us_node_idx].us_short_addr == LBS_INVALID_SHORT_ADDRESS) {
				spx_current_addr_list[us_node_idx].us_short_addr = us_short_address;
				memcpy(spx_current_addr_list[us_node_idx].puc_ext_addr, puc_extended_address, 8);
				sus_num_devices++;
				break;
			}
		}

		/* update reports for node list */
		net_info_report_devlist(spx_current_addr_list, sus_num_devices);
		/* Update web interface */
		http_mng_send_cmd(LNXCMD_REFRESH_DEVLIST, 0);
	}
}

static void _leave_node(uint16_t us_short_address)
{
	uint16_t us_node_idx;

	/* Reset node in cycle list */
	for (us_node_idx = 0; us_node_idx < NUM_MAX_NODES; us_node_idx++) {
		if (spx_current_addr_list[us_node_idx].us_short_addr == us_short_address) {
			spx_current_addr_list[us_node_idx].us_short_addr = LBS_INVALID_SHORT_ADDRESS;
			memset(spx_current_addr_list[us_node_idx].puc_ext_addr, 0x00, 8);
			sus_num_devices--;

			/* update reports for node list */
			net_info_report_devlist(spx_current_addr_list, sus_num_devices);
			/* Update web interface */
			http_mng_send_cmd(LNXCMD_REFRESH_DEVLIST, 0);
			break;
		}
	}
}

static void _process_adp_event(uint8_t *puc_ev_data)
{
	uint8_t *puc_data_ptr;
	uint8_t uc_adp_event;
	uint16_t us_len;
	uint32_t ul_att_id;

	puc_data_ptr = puc_ev_data;

	uc_adp_event = *puc_data_ptr++;
	us_len = (uint16_t)(*puc_data_ptr++ << 8);
	us_len += (uint16_t)(*puc_data_ptr++);
	switch(uc_adp_event) {

	case NET_INFO_ADP_NET_START_CFM:
		_reset_node_list();
		_ppp0_iface_down();
		_ppp0_iface_up();
		break;

	case NET_INFO_ADP_JOIN_IND:
	{
		uint16_t us_short_add;

		/* Get short address */
		us_short_add = (uint16_t)(*puc_data_ptr++ << 8);
		us_short_add += (uint16_t)(*puc_data_ptr++);
		/* Manage Registered Nodes List */
		_join_node(us_short_add, puc_data_ptr);
	}
		break;

	case NET_INFO_ADP_LEAVE_IND:
	{
		uint16_t us_short_add;

		/* Get short address */
		us_short_add = (uint16_t)(*puc_data_ptr++ << 8);
		us_short_add += (uint16_t)(*puc_data_ptr++);
		/* Manage Registered Nodes List */
		_leave_node(us_short_add);
	}
		break;

	case NET_INFO_ADP_SET_CFM:
		break;


	case NET_INFO_ADP_MAC_SET_CFM:
		break;


	case NET_INFO_ADP_GET_CFM:
		break;


	case NET_INFO_ADP_MAC_GET_CFM:
		/* Get attribute id */
		puc_data_ptr++; /* uc_status */
		ul_att_id = (uint32_t)(*puc_data_ptr++ << 24);
		ul_att_id += (uint32_t)(*puc_data_ptr++ << 16);
		ul_att_id += (uint32_t)(*puc_data_ptr++ << 8);
		ul_att_id += (uint32_t)(*puc_data_ptr++);

		/* Check Extended Address request */
		if (ul_att_id == MAC_PIB_MANUF_EXTENDED_ADDRESS) {
			puc_data_ptr += 3;
			memcpy(sx_coord_data.puc_ext_addr, puc_data_ptr, 8);

			/* Reset semaphore and flag to get data from coordinator */
			sx_coord_data.b_is_valid = true;
			sul_waiting_cdata_timer = 0;
			sb_pending_cdata_cfm = false;
		}
		break;

	}
}

static void NetInfoEventIndication(net_info_event_ind_t *px_event_info)
{
	/* Check event */
	if (px_event_info->uc_event_id >= NET_INFO_EV_INVALID) {
		return;
	}

	switch(px_event_info->uc_event_id) {

	case NET_INFO_ADP_EVENT:
		_process_adp_event(px_event_info->puc_event_info);
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
	struct stat dataFile;

	if (sul_waiting_cdata_timer) {
		sul_waiting_cdata_timer--;
		return;
	}

	if (sul_waiting_preq_timer) {
		/* Blocking timer */
		sul_waiting_preq_timer--;
		return;
	}

	/* Check PPP0 interface stats file*/
	if (lstat (spuc_ppp_iface_file, &dataFile) == -1) {
			printf ("ppp0 not exist\n");
			/* It file doesn't exists, PPP0 iface should be restarted */
			_ppp0_iface_up();
	}

	/* Check Validity of Coordinator Data */
	if (sx_coord_data.b_is_valid == false) {
		/* get coordinator extended address */
		NetInfoAdpMacGetRequest(MAC_PIB_MANUF_EXTENDED_ADDRESS, 0);
		/* Blocking process to wait Coord Data Msg */
		sul_waiting_cdata_timer = TIMER_TO_CDATA_INFO;
		sb_pending_cdata_cfm = true;
		return;
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

	_ppp0_iface_down();
	_init_reset_plc();
	_reset_plc();
	_reset_node_list();

	memset(&sx_coord_data, 0, sizeof(sx_coord_data));

	net_info_callbacks.event_indication = NetInfoEventIndication;
	NetInfoSetCallbacks(&net_info_callbacks);

	sb_pending_preq_cfm = false;
	sb_pending_cdata_cfm = false;
	sul_waiting_cdata_timer = 0;
	sul_waiting_preq_timer = 0;
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
    case WEBCMD_UPDATE_DEVLIST:
		{
			LOG_NET_INFO_DEBUG(("Net Info manager: update dash board info\n"));
			//net_info_report_dashboard(&sx_net_info, &sx_net_statistics);
			http_mng_send_cmd(LNXCMD_REFRESH_DEVLIST, 0);
		}
    	break;
    }
}
