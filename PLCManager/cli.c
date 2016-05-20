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

static x_cli_node_vs_t sx_cli_nodes_version[CLI_NUM_MAX_NODE_VS];

static void _get_registered_nodes(void)
{
	sb_update_to_xml = true;
	ifacePrime_select_api(si_cli_app_id);
	prime_cl_null_mlme_list_get_request(PIB_MAC_LIST_REGISTER_DEVICES);
}

static void _get_version_nodes(void)
{
	uint8_t puc_eui48[6];

	memset(puc_eui48, 0xFF, 6);

	/* Get version for all nodes */
	ifacePrime_select_api(si_cli_app_id);
	bmng_fup_get_version_request(puc_eui48);
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
		x_cli_node_vs_t *px_node_info;
		uint8_t uc_node_idx;
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
			px_node_info = sx_cli_nodes_version;
			for (uc_node_idx = 0; uc_node_idx < CLI_NUM_MAX_NODE_VS; uc_node_idx++, px_node_info++) {
				if (memcmp(px_node_info->puc_eui48, x_node.mac_addr, 6) == 0) {
					break;
				}
			}

			if (uc_node_idx == CLI_NUM_MAX_NODE_VS) {
				xml_rep_add_node(xml_fd, &x_node, NULL);
			} else {
				xml_rep_add_node(xml_fd, &x_node, px_node_info);
			}

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

static void _fup_version_ind_cb (uint8_t* puc_eui48, uint8_t uc_vendor_len, uint8_t * puc_vendor,
        uint8_t uc_model_len,  uint8_t * puc_model, uint8_t uc_version_len, uint8_t * puc_version)
{
	x_cli_node_vs_t *px_node_info;
	uint8_t uc_idx;
	uint8_t puc_null_eui48[6];
	uint8_t uc_free_idx;

	/* Search NODE MAC in table */
	memset(puc_null_eui48, 0, 6);
	px_node_info = sx_cli_nodes_version;
	uc_free_idx = 0xFF;
	for (uc_idx = 0; uc_idx < CLI_NUM_MAX_NODE_VS; uc_idx++, px_node_info++) {
		if (memcmp(px_node_info->puc_eui48, puc_eui48, 6) == 0) {
			/* update node version */
			memcpy(px_node_info->puc_fw_version, puc_version, uc_version_len);
			memcpy(px_node_info->puc_model, puc_model, uc_model_len);
			memcpy(px_node_info->puc_vendor, puc_vendor, uc_vendor_len);
			/* End strings */
			px_node_info->puc_fw_version[uc_version_len] = *(uint8_t *)"\0";
			px_node_info->puc_model[uc_model_len] = *(uint8_t *)"\0";
			px_node_info->puc_vendor[uc_vendor_len] = *(uint8_t *)"\0";

			return;
		} else if (uc_free_idx == 0xFF) {
			if (memcmp(px_node_info->puc_eui48, puc_null_eui48, 6) == 0) {
				/* Get bookmark for first free position */
				uc_free_idx = uc_idx;
			}
		}
	}

	/* update node version in bookmark */
	if (uc_free_idx < 0xFF) {
		px_node_info = &sx_cli_nodes_version[uc_free_idx];
		memcpy(px_node_info->puc_eui48, puc_eui48, 6);
		memcpy(px_node_info->puc_fw_version, puc_version, uc_version_len);
		memcpy(px_node_info->puc_model, puc_model, uc_model_len);
		memcpy(px_node_info->puc_vendor, puc_vendor, uc_vendor_len);
		/* End strings */
		px_node_info->puc_fw_version[uc_version_len] = *(uint8_t *)"\0";
		px_node_info->puc_model[uc_model_len] = *(uint8_t *)"\0";
		px_node_info->puc_vendor[uc_vendor_len] = *(uint8_t *)"\0";
	}

	/* Update XML Topology file */
	xml_rep_update_node_version(px_node_info);
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
	st_bmng_cbs.fup_version_ind_cb = _fup_version_ind_cb;
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
	FILE *stream;
	char *line = NULL;
	size_t len = 0;

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

	/* Check WEB Command Text File */
	stream = fopen("/home/DCWS/public/message.txt", "r+");
	if (stream != NULL) {
		bool b_rem;

		if (getline(&line, &len, stream) != -1) {
			switch (line[0]) {
				case PRIME_CMD_UPDATE_NODE_LIST:
					_get_registered_nodes();
					b_rem = true;
					break;

				case PRIME_CMD_GET_NODE_VERSIONS:
					_get_version_nodes();
					b_rem = true;
					break;

				default:
					b_rem = false;
			}
		}

		fclose(stream);
		if (b_rem) {
			remove("/home/DCWS/public/message.txt");
		}
	}
}

/*	int fd_txt;
	char *ptr_cmd;

	fd_txt = open("/home/DCWS/public/message.txt", O_RDWR);

	ptr_cmd = spuc_web_cmd;
	if (fd_txt != -1) {
		while(read(fd_txt, ptr_cmd++, 1) > 0) {
			if (ptr_cmd == spuc_web_cmd + sizeof(spuc_web_cmd)) {
				break;
			}
		};

		switch (spuc_web_cmd[0]) {
			case PRIME_CMD_UPDATE_NODE_LIST:
				_get_registered_nodes();
				break;

			case PRIME_CMD_GET_NODE_VERSIONS:
				_get_version_nodes();
				break;
		}

		memset(&spuc_web_cmd, 0, sizeof(spuc_web_cmd));
		remove("/home/DCWS/public/message.txt");
	}
}*/

