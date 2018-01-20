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

#include "config.h"
#include "socket_handler.h"
#include "ifaceNet_api.h"
#include "net_info_mng.h"
#include "net_info_report.h"
#include "http_mng.h"
#include "tools.h"
#include "fu_mng.h"

#ifdef NET_INFO_DEBUG_CONSOLE
#	define LOG_NET_INFO_DEBUG(a)   printf a
#else
#	define LOG_NET_INFO_DEBUG(a)   (void)0
#endif


static int si_net_info_id;

/* gateway info */
static x_gw_data_t sx_gw_data;
static bool sb_net_start;
static uint8_t suc_webcmd_pending;

/* Connection status */
static uint16_t sus_num_devices;
static x_dev_addr_t spx_current_addr_list[NUM_MAX_NODES];
static char spc_fu_filename[200];

static bool sb_pending_preq_cfm;
static bool sb_pending_cdata_cfm;

/* Waiting Process timer */
static uint32_t sul_waiting_cdata_timer;
static uint32_t sul_waiting_preq_timer;

/* Timers based on 10 ms */
#define TIMER_TO_CDATA_INFO       500 // 5 seconds
#define TIMER_TO_REQ_PATH_INFO    500 // 5 seconds

static void _reset_node_list(void)
{
	memset(spx_current_addr_list, 0, sizeof(spx_current_addr_list));
	sus_num_devices = 0;
	/* update reports for NULL node list */
	net_info_report_devlist(spx_current_addr_list, sus_num_devices);
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

static void _extract_network_config(uint8_t *puc_net_cfg)
{
	char puc_ln_buf[50];
	int i_ln_len, i_size_fd;
	int fd;
	uint8_t *puc_ptr;
	uint8_t uc_idx;

	uc_idx = 0;

	// G3 version
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/g3stackversion", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02u.%02u.%02u %02u-%02u-%02u", *puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2),
						*(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 6;

	// pan id
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/panid", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X", *puc_ptr, *(puc_ptr + 1));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 2;

	// short addr
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/shortaddress", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X", *puc_ptr, *(puc_ptr + 1));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 2;

	// extended addr
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/extendedaddress", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X", *puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2),
			*(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 8;

	// PPP : MAC
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/pppmacdaddress", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X:%02X:%02X:%02X:%02X:%02X", *puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2),
			*(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 6;

	// PPP : LLA
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/ppplla", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// PPP : ULA
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/pppula", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// G3 PLC : LLA
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/plclla", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// G3 PLC : ULA
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/plcula", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// G3 PLC : Prefix
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/plcprefix", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// Routes : G3 PLC
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/routeplc", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// Routes : PPP
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/routeppp", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// SEC: PSK
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/secpsk", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// SEC: GMK
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/secgmk", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
			*puc_ptr, *(puc_ptr + 1), *(puc_ptr + 2), *(puc_ptr + 3), *(puc_ptr + 4), *(puc_ptr + 5), *(puc_ptr + 6), *(puc_ptr + 7),
			*(puc_ptr + 8), *(puc_ptr + 9), *(puc_ptr + 10), *(puc_ptr + 11), *(puc_ptr + 12), *(puc_ptr + 13), *(puc_ptr + 14), *(puc_ptr + 15));
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx += 16;

	// CONF_MAX_JOIN_WAIT_TIME
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/maxjoinwaittime", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%u", *puc_ptr);
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx ++;

	// CONF_MAX_HOPS
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/maxhops", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%u", *puc_ptr);
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx ++;

	// PLC SNIFFER
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/sniffer_en", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	i_ln_len = sprintf(puc_ln_buf, "%u", *puc_ptr);
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
	uc_idx ++;

	// GPRS
	puc_ptr = &puc_net_cfg[uc_idx];
	fd = open("/home/cfg/gprs_en", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
	if (tools_gprs_check() == 0) {
		*puc_ptr = 1;
	} else {
		*puc_ptr = 0;
	}
	i_ln_len = sprintf(puc_ln_buf, "%u", *puc_ptr);
	i_size_fd = write(fd, puc_ln_buf, i_ln_len);
	close(fd);
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
		tools_plc_down();
		tools_plc_up();
		sb_net_start = true;
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
	{
		/* Get attribute id */
		puc_data_ptr++; /* uc_status */
		ul_att_id = (uint32_t)(*puc_data_ptr++ << 24);
		ul_att_id += (uint32_t)(*puc_data_ptr++ << 16);
		ul_att_id += (uint32_t)(*puc_data_ptr++ << 8);
		ul_att_id += (uint32_t)(*puc_data_ptr++);

		/* Check Extended Address request */
		if (ul_att_id == ADP_IB_MANUF_NET_CONFIG) {
			puc_data_ptr += 3;

			/* Extract net config data */
			_extract_network_config(puc_data_ptr);

			/* Reset semaphore and flag to get data from gateway */
			sx_gw_data.b_is_valid = true;
			sul_waiting_cdata_timer = 0;
			sb_pending_cdata_cfm = false;
		}
	}
		break;


	case NET_INFO_ADP_MAC_GET_CFM:
	{
		/* Get attribute id */
		puc_data_ptr++; /* uc_status */
		ul_att_id = (uint32_t)(*puc_data_ptr++ << 24);
		ul_att_id += (uint32_t)(*puc_data_ptr++ << 16);
		ul_att_id += (uint32_t)(*puc_data_ptr++ << 8);
		ul_att_id += (uint32_t)(*puc_data_ptr++);
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

	if (sb_net_start == false) {
		return;
	}

	if (sul_waiting_cdata_timer) {
		sul_waiting_cdata_timer--;
		return;
	}

	if (sul_waiting_preq_timer) {
		/* Blocking timer */
		sul_waiting_preq_timer--;
		return;
	}

	/* Check PPP0 interface is enable */
	if (tools_plc_check() == -1) {
			tools_plc_reset();
			system("killall pppd");
			system("killall chat");
			sleep(2);
			printf ("ppp0 not exist\n");
			/* It file doesn't exists, PPP0 iface should be restarted */
			tools_plc_up();
	}

	/* Check Validity of gateway Data */
	if (sx_gw_data.b_is_valid == false) {
		/* get gateway extended address */
		NetInfoAdpGetRequest(ADP_IB_MANUF_NET_CONFIG, 0);
		/* Blocking process to wait gw Data Msg */
		sul_waiting_cdata_timer = TIMER_TO_CDATA_INFO;
		sb_pending_cdata_cfm = true;
		return;
	}

	/* Check Web Command pending to send Command to Node JS */
	if (suc_webcmd_pending != WEBCMD_INVALD) {
		http_mng_send_cmd(suc_webcmd_pending, 0);
		suc_webcmd_pending = WEBCMD_INVALD;
	}

	/* Check FU start file */
	if (tools_fu_start_check(spc_fu_filename)) {
		if (fu_mng_start(spc_fu_filename) == -1) {
			fprintf(stderr, "FUP ERROR\n");
		}

		/* Restart PLC gateway: Update gateway info */
		sx_gw_data.b_is_valid = false;
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

	sb_net_start = false;

	tools_plc_down();
	tools_plc_reset();
	_reset_node_list();

	memset(&sx_gw_data, 0, sizeof(sx_gw_data));

	net_info_callbacks.event_indication = NetInfoEventIndication;
	NetInfoSetCallbacks(&net_info_callbacks);

	sb_pending_preq_cfm = false;
	sb_pending_cdata_cfm = false;
	sul_waiting_cdata_timer = 0;
	sul_waiting_preq_timer = 0;

	suc_webcmd_pending = WEBCMD_INVALD;
}

/**
 */
void net_info_webcmd_process(uint8_t* buf)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf) {
    case WEBCMD_UPDATE_DEVLIST:
		{
			LOG_NET_INFO_DEBUG(("Net Info manager: update device list\n"));
			http_mng_send_cmd(LNXCMD_REFRESH_DEVLIST, 0);
		}
    	break;

    case WEBCMD_ENABLE_GPRS_MOD:
		{
			bool b_up;
			char puc_ln_buf[50];
			int i_ln_len, i_size_fd;
			int fd;

			LOG_NET_INFO_DEBUG(("Net Info manager: enable GPRS module\n"));
			b_up = false;
			tools_gprs_enable();
			if (tools_gprs_detect()) {
				tools_gprs_up();
				if (tools_gprs_check()) {
					b_up = true;
				}
			}

			fd = open("/home/cfg/gprs_en", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
			if (b_up) {
				i_ln_len = sprintf(puc_ln_buf, "1");
			} else {
				i_ln_len = sprintf(puc_ln_buf, "0");
			}
			i_size_fd = write(fd, puc_ln_buf, i_ln_len);
			close(fd);

			suc_webcmd_pending = LNXCMD_REFRESH_GPRS;
		}
    	break;

    case WEBCMD_DISABLE_GPRS_MOD:
		{
			char puc_ln_buf[50];
			int i_ln_len, i_size_fd;
			int fd;

			LOG_NET_INFO_DEBUG(("Net Info manager: disable GPRS module\n"));
			tools_gprs_down();
			tools_gprs_disable();

			i_ln_len = sprintf(puc_ln_buf, "0");
			fd = open("/home/cfg/gprs_en", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
			i_size_fd = write(fd, puc_ln_buf, i_ln_len);
			close(fd);

			suc_webcmd_pending = LNXCMD_REFRESH_GPRS;
		}
    	break;

    case WEBCMD_ENABLE_SNIFFER_MOD:
		{
			uint8_t uc_sniffer_enable;
			char puc_ln_buf[50];
			int i_ln_len, i_size_fd;
			int fd;

			LOG_NET_INFO_DEBUG(("Net Info manager: enable PLC SNIFFER\n"));
			uc_sniffer_enable = 1;
			NetInfoAdpMacSetRequest(MAC_PIB_MANUF_ENABLE_MAC_SNIFFER, 0, 1, &uc_sniffer_enable);

			i_ln_len = sprintf(puc_ln_buf, "1");
			fd = open("/home/cfg/sniffer_en", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
			i_size_fd = write(fd, puc_ln_buf, i_ln_len);
			close(fd);

			suc_webcmd_pending = LNXCMD_REFRESH_SNIFFER;
		}
    	break;

    case WEBCMD_DISABLE_SNIFFER_MOD:
		{
			uint8_t uc_sniffer_enable;
			char puc_ln_buf[50];
			int i_ln_len, i_size_fd;
			int fd;

			LOG_NET_INFO_DEBUG(("Net Info manager: disable PLC SNIFFER\n"));
			uc_sniffer_enable = 0;
			NetInfoAdpMacSetRequest(MAC_PIB_MANUF_ENABLE_MAC_SNIFFER, 0, 1, &uc_sniffer_enable);

			i_ln_len = sprintf(puc_ln_buf, "0");
			fd = open("/home/cfg/sniffer_en", O_RDWR|O_CREAT, S_IROTH|S_IWOTH|S_IXOTH);
			i_size_fd = write(fd, puc_ln_buf, i_ln_len);
			close(fd);

			suc_webcmd_pending = LNXCMD_REFRESH_SNIFFER;
		}
    	break;
    }
}
