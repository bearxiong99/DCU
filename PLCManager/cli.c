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

static void _prime_cl_null_mlme_get_cfm_cb(mlme_result_t x_status, uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size)
{
	uint8_t *puc_msg;

	if (uc_pib_size == 0) {
		return;
	}

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
		xml_fd = xml_rep_open();
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
		xml_rep_close(xml_fd);

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
				sb_update_to_xml = true;
				ifacePrime_select_api(si_cli_app_id);
				prime_cl_null_mlme_list_get_request(PIB_MAC_LIST_REGISTER_DEVICES);
			}
	    	break;

    }
}


void cli_init(int _app_id)
{
	prime_cl_null_callbacks_t st_null_cbs;

	si_cli_app_id = _app_id;

	sb_update_to_xml = false;

	/* CL NULL Callbacks: NO NEED */
	st_null_cbs.prime_cl_null_establish_ind_cb = NULL;
	st_null_cbs.prime_cl_null_establish_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_release_ind_cb = NULL;
	st_null_cbs.prime_cl_null_release_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_join_ind_cb = NULL;
	st_null_cbs.prime_cl_null_join_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_leave_ind_cb = NULL;
	st_null_cbs.prime_cl_null_leave_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_data_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_data_ind_cb = NULL;
	st_null_cbs.prime_cl_null_plme_reset_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_plme_sleep_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_plme_resume_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_plme_testmode_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_plme_get_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_plme_set_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_register_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_register_ind_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_unregister_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_unregister_ind_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_promote_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_promote_ind_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_demote_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_demote_ind_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_reset_cfm_cb = NULL;
	st_null_cbs.prime_cl_null_mlme_get_cfm_cb = _prime_cl_null_mlme_get_cfm_cb;
	st_null_cbs.prime_cl_null_mlme_list_get_cfm_cb = _prime_cl_null_mlme_list_get_cfm_cb;
	st_null_cbs.prime_cl_null_mlme_set_cfm_cb = _prime_cl_null_mlme_set_cfm_cb;

	prime_cl_null_set_callbacks(si_cli_app_id, &st_null_cbs);
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

