#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "socket_handler.h"
#include "sniffer.h"
#include "usi_host/usi_host.h"
#include "usi_host/hal_utils.h"

#ifdef SNIFFER_DEBUG_CONSOLE
#	define LOG_SNIFFER_DEBUG(a)   printf a
#else
#	define LOG_SNIFFER_DEBUG(a)   (void)0
#endif

//#define SNIFFER_PROTOCOL_IN_USE            0x13  //PROTOCOL_SNIF_PRIME
#define SNIFFER_PROTOCOL_IN_USE            0x23  //PROTOCOL_SNIF_G3

/* Serial Port descriptor */
static int si_serial_port_fd;
static int si_sniffer_socket_fd;

static int si_sniffer_app_id;

static unsigned char suc_sniffer_rcv_buf[MAX_SNIFFER_BUFFER_SIZE];

static void _sniffer_data_rcv(uint8_t *msg, uint16_t len)
{
	int pi_conn_fds[SOCKET_MAX_CONN];
	int num_conn;
	int idx;

	num_conn = socket_get_connections_fds(si_sniffer_app_id, pi_conn_fds);

	if (num_conn > 0) {
		x_usi_cmd_t usi_cmd;

		usi_cmd.puc_buf = msg; /* remove usi headers */
		usi_cmd.us_len = len; /* rest headers + crc bytes */
		usi_cmd.uc_p_type = SNIFFER_PROTOCOL_IN_USE;
		for (idx = 0; idx < SOCKET_MAX_CONN; idx++) {
			if (pi_conn_fds[idx] > 0) {
				usi_cmd._fd = pi_conn_fds[idx];
				/* send to USI through socket param */
				usi_host_send_cmd(&usi_cmd);
				LOG_SNIFFER_DEBUG(("Sniffer: usi_host_send_cmd(socket[%u])\n", usi_cmd._fd));
			}
		}
	}
}

void sniffer_init(int _app_id, int _serial_fd)
{
	si_serial_port_fd = SOCKET_UNUSED;
	
	si_sniffer_app_id = _app_id;

	if (_serial_fd > 0) {
		si_serial_port_fd = _serial_fd;
		/* Set sniffer callback */
		usi_host_set_sniffer_cb(_sniffer_data_rcv);
	}
}

void sniffer_callback(socket_ev_info_t *_ev_info)
{
	if (_ev_info->i_socket_fd >= 0) {

		/* Catch Sniffer/Prime Manager Socket */
		si_sniffer_socket_fd = _ev_info->i_socket_fd;

		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
			/* Manage LINK */
			socket_accept_conn(_ev_info);
		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
			/* Receive DATA */
			ssize_t i_bytes;
			/* Read data from Socket */
			i_bytes = read(_ev_info->i_socket_fd, suc_sniffer_rcv_buf, MAX_SNIFFER_BUFFER_SIZE);
			if (i_bytes > 0) {
				LOG_SNIFFER_DEBUG(("Sniffer: read socket(socket[%u])\n", _ev_info->i_socket_fd));
				/* Write data to Serial Socket */
				if (si_serial_port_fd > 0) {
					LOG_SNIFFER_DEBUG(("Sniffer: write socket(socket[%u])\n", si_serial_port_fd));
					write(si_serial_port_fd, suc_sniffer_rcv_buf, i_bytes);
				}
			} else {
				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
			}
		}
	}
}

