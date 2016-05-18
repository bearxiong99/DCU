#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "socket_handler.h"
#include "cli.h"
#include "xml_rep.h"

#include "ifacePrime_api.h"
#include "prime_api_host.h"
#include "prime_api_defs_host.h"
#include "mac_pib.h"

static int si_cli_app_id;

/* CLI socket descriptors */
static int si_cli_link_fd;
static int si_cli_data_fd;

static unsigned char suc_cli_rcv_buf[MAX_CLI_BUFFER_SIZE];
static unsigned char suc_cli_tx_buf[MAX_CLI_BUFFER_SIZE];

static bool sb_update_to_xml;

static x_cli_prime_info_t sx_prime_info;
static uint8_t suc_info_status;

static void _get_registered_nodes(void)
{
	sb_update_to_xml = true;
	ifacePrime_select_api(si_cli_app_id);
	prime_cl_null_mlme_list_get_request(PIB_MAC_LIST_REGISTER_DEVICES);
}

static void _get_pib(uint16_t us_pib)
{
	ifacePrime_select_api(si_cli_app_id);
	prime_cl_null_mlme_get_request(us_pib);
}

static void _prime_cl_null_mlme_get_cfm_cb(mlme_result_t x_status, uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size)
{
	if (uc_pib_size == 0) {
		return;
	}

	switch (us_pib_attrib) {
		case PIB_MAC_EUI_48:
			memcpy(sx_prime_info.mac_addr, pv_pib_value, uc_pib_size);
			suc_info_status = PRIME_INFO_GET_FWV;
			break;

		case PIB_MAC_APP_FW_VERSION:
			memcpy(sx_prime_info.fw_version, pv_pib_value, uc_pib_size);
			sx_prime_info.fw_version[uc_pib_size] = *(uint8_t *)"\0";
			suc_info_status = PRIME_INFO_GET_VID;
			break;

		case PIB_MAC_APP_VENDOR_ID:
			sx_prime_info.us_vendor_id = ((uint16_t)(*(uint8_t *)pv_pib_value)) << 8;
			sx_prime_info.us_vendor_id += ((uint16_t)(*(uint8_t *)pv_pib_value+ 1));
			suc_info_status = PRIME_INFO_GET_PID;
			break;

		case PIB_MAC_APP_PRODUCT_ID:
			sx_prime_info.us_product_id = ((uint16_t)(*(uint8_t *)pv_pib_value)) << 8;
			sx_prime_info.us_product_id += ((uint16_t)(*(uint8_t *)pv_pib_value+ 1));
			suc_info_status = PRIME_INFO_CREATE_FILE;
			break;
	}

	if (si_cli_data_fd > 0) {
		uint8_t *puc_msg;

		puc_msg = suc_cli_tx_buf;

		*puc_msg++ = PRIME_CMD_PLME_GET_PIB_RSP;
		*puc_msg++ = x_status;
		*puc_msg++ = (uint8_t)(us_pib_attrib >> 8);
		*puc_msg++ = (uint8_t)(us_pib_attrib);
		memcpy(puc_msg, pv_pib_value, uc_pib_size);
		puc_msg += uc_pib_size;

		/* Send notification to CLI */
		write(si_cli_data_fd, suc_cli_tx_buf, puc_msg - suc_cli_tx_buf);
	}
}

static void _prime_cl_null_mlme_set_cfm_cb(mlme_result_t x_result)
{
	uint8_t *puc_msg;

	puc_msg = suc_cli_tx_buf;

	*puc_msg++ = PRIME_CMD_PLME_SET_PIB_RSP;
	*puc_msg++ = x_result;

	/* Send notification to CLI */
	write(si_cli_data_fd, suc_cli_tx_buf, puc_msg - suc_cli_tx_buf);
}

static void _prime_cl_null_mlme_list_get_cfm_cb(mlme_result_t x_status, uint16_t us_pib_attrib, uint8_t *puc_pib_buff, uint16_t us_pib_len)
{
	if (us_pib_len == 0) {
		return;
	}

	if (sb_update_to_xml) {
		uint8_t *puc_buff;
		x_cli_node_reg_t x_node;
		int i;
		int xml_fd;

		/* open xml file */
		xml_fd = xml_rep_open_nodelist();
		if (xml_fd < 1) {
			sb_update_to_xml = false;
			return;
		}

		puc_buff = puc_pib_buff;
		for (i = us_pib_len; i > 0; i -= sizeof(x_cli_node_reg_t)) {
			memcpy(x_node.mac_addr, puc_buff, 6);
			puc_buff +=6;
			x_node.us_lnid = (uint16_t)*puc_buff++ << 8;
			x_node.us_lnid += (uint16_t)*puc_buff++;
			x_node.uc_state = *puc_buff++;
			x_node.us_lsid = (uint16_t)*puc_buff++ << 8;
			x_node.us_lsid += (uint16_t)*puc_buff++;
			x_node.us_sid = (uint16_t)*puc_buff++ << 8;
			x_node.us_sid += (uint16_t)*puc_buff++;
			x_node.uc_level = *puc_buff++;
			x_node.uc_mac_cap1 = *puc_buff++;
			x_node.uc_mac_cap2 = *puc_buff++;

			/* add node to xml file */
			xml_rep_add_node(xml_fd, &x_node);
		}

		/* close xml file */
		xml_rep_close_nodelist(xml_fd);

		sb_update_to_xml = false;

	} else {
		uint8_t *puc_msg;

		puc_msg = suc_cli_tx_buf;

		*puc_msg++ = PRIME_CMD_MLME_GET_LIST_PIB_RSP;

		/* Send notification to CLI */
		write(si_cli_data_fd, suc_cli_tx_buf, puc_msg - suc_cli_tx_buf);
	}
}

/**
 * \brief Process messages received from CLI.
 * Unpack CLI protocol command
 */
static void _cli_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf++) {
		case PRIME_CMD_MLME_GET_PIB:
			{
				uint16_t us_pib_attr;

				us_pib_attr = ((uint16_t)*puc_buf++) << 8;
				us_pib_attr += (uint16_t)*puc_buf++;

				//send data
				ifacePrime_select_api(si_cli_app_id);
				prime_cl_null_mlme_get_request(us_pib_attr);
			}
			break;

	    case PRIME_CMD_MLME_SET_PIB:
			{
				uint16_t us_pib_attr;
				uint8_t uc_pib_size;

				us_pib_attr = ((uint16_t)*puc_buf++) << 8;
				us_pib_attr += (uint16_t)*puc_buf++;
				uc_pib_size = (uint16_t)*puc_buf++;

				//send data
				ifacePrime_select_api(si_cli_app_id);
				prime_cl_null_mlme_set_request(us_pib_attr, puc_buf, uc_pib_size);
			}
	    	break;

	    case PRIME_CMD_UPDATE_NODE_LIST:
			{
				_get_registered_nodes();
			}
	    	break;

    }
}

static void _network_event_ind_cb(bmng_net_event_t *px_net_event)
{
	bool b_update_node_list;

	b_update_node_list = false;

	switch (px_net_event->net_event) {
		case BMNG_NET_EVENT_REGISTER:
			b_update_node_list = true;
			break;

		case BMNG_NET_EVENT_UNREGISTER:
			b_update_node_list = true;
			break;

		case BMNG_NET_EVENT_PROMOTE:
			b_update_node_list = true;
			break;

		case BMNG_NET_EVENT_DEMOTE:
			b_update_node_list = true;
			break;

		case BMNG_NET_EVENT_ALIVE:
			b_update_node_list = false;
			break;
	}

	if (b_update_node_list) {
		_get_registered_nodes();
	}
}


void cli_init(int _app_id)
{
	prime_cl_null_callbacks_t st_null_cbs;
	prime_bmng_callbacks_t st_bmng_cbs;

	si_cli_app_id = _app_id;

	sb_update_to_xml = false;

	/* Clear PRIME info */
	memset(&sx_prime_info, 0, sizeof(sx_prime_info));

	/* CL NULL Callbacks: NO NEED */
	memset(&st_null_cbs, 0, sizeof(st_null_cbs));
	st_null_cbs.prime_cl_null_mlme_get_cfm_cb = _prime_cl_null_mlme_get_cfm_cb;
	st_null_cbs.prime_cl_null_mlme_list_get_cfm_cb = _prime_cl_null_mlme_list_get_cfm_cb;
	st_null_cbs.prime_cl_null_mlme_set_cfm_cb = _prime_cl_null_mlme_set_cfm_cb;
	prime_cl_null_set_callbacks(si_cli_app_id, &st_null_cbs);

	memset(&st_bmng_cbs, 0, sizeof(st_bmng_cbs));
	st_bmng_cbs.network_event_ind_cb = _network_event_ind_cb;
	bmng_set_callbacks(si_cli_app_id, &st_bmng_cbs);

	suc_info_status = PRIME_INFO_GET_MAC;
	_get_registered_nodes();
}

void cli_callback(socket_ev_info_t *_ev_info)
{
	if (_ev_info->i_socket_fd >= 0) {
		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
			/* Manage LINK */
			si_cli_link_fd = _ev_info->i_socket_fd;
			socket_accept_conn(_ev_info);
		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
			/* Receive DATA */
			ssize_t i_bytes;
			si_cli_data_fd = _ev_info->i_socket_fd;
			/* Read data from Socket */
			i_bytes = read(_ev_info->i_socket_fd, suc_cli_rcv_buf, MAX_CLI_BUFFER_SIZE);
			if (i_bytes > 0) {
				_cli_rcv_cmd(suc_cli_rcv_buf, i_bytes);
			} else {
				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
			}
		}
	}
}

void cli_process(void)
{
	/* Check WEB Command Text File */
	int fd_txt;
	char c;

	fd_txt = open("/home/DCWS/public/message.txt", O_RDONLY);

	if (fd_txt != -1) {
		read(fd_txt, &c, sizeof(c));

		switch (c) {
			case PRIME_CMD_UPDATE_NODE_LIST:
				sb_update_to_xml = true;
				ifacePrime_select_api(si_cli_app_id);
				prime_cl_null_mlme_list_get_request(PIB_MAC_LIST_REGISTER_DEVICES);
				break;
		}

		remove("/home/DCWS/public/message.txt");
	}

	/* Check PRIME info status */
	if (suc_info_status != PRIME_INFO_OK) {
		switch(suc_info_status) {
		case PRIME_INFO_GET_MAC:
			_get_pib(PIB_MAC_EUI_48);
			suc_info_status = PRIME_INFO_WAIT_RSP;
			break;

		case PRIME_INFO_GET_FWV:
			_get_pib(PIB_MAC_APP_FW_VERSION);
			suc_info_status = PRIME_INFO_WAIT_RSP;
			break;

		case PRIME_INFO_GET_VID:
			_get_pib(PIB_MAC_APP_VENDOR_ID);
			suc_info_status = PRIME_INFO_WAIT_RSP;
			break;

		case PRIME_INFO_GET_PID:
			_get_pib(PIB_MAC_APP_PRODUCT_ID);
			suc_info_status = PRIME_INFO_WAIT_RSP;
			break;

		case PRIME_INFO_WAIT_RSP:
			break;

		case PRIME_INFO_CREATE_FILE:
			xml_rep_prime_info(&sx_prime_info);
			suc_info_status = PRIME_INFO_OK;
			break;
		}
	}
}

