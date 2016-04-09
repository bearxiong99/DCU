#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "socket_handler.h"
#include "prime_manager.h"
#include "usi_host.h"
#include "hal_utils.h"

static int si_serial_port_fd;

static int si_prime_manager_app_id;

static unsigned char suc_prime_mng_rcv_buf[MAX_PRIMEMNG_BUFFER_SIZE];

void prime_manager_init(int _app_id, int _serial_fd)
{
	si_prime_manager_app_id = _app_id;
	si_serial_port_fd = _serial_fd;

	/* Clear redirection USI socket */
	usi_host_loopback (-1);
}

void prime_manager_callback(socket_ev_info_t *_ev_info)
{
	if (_ev_info->i_socket_fd >= 0) {

		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
			int pi_conn_fds[SOCKET_MAX_CONN];
			int num_conn;

			/* Manage LINK */
			socket_accept_conn(_ev_info);
			/* Maintain only one connection for PLC_MNG_PRIMEMNG_APP_ID */
			num_conn = socket_get_connections_fds(si_prime_manager_app_id, pi_conn_fds);
			if (num_conn > 1) {
				/* Close the last one */
				socket_dettach_connection(si_prime_manager_app_id, pi_conn_fds[1]);
			}
		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
			/* Receive DATA */
			ssize_t i_bytes;
			/* Read data from Socket */
			i_bytes = read(_ev_info->i_socket_fd, suc_prime_mng_rcv_buf, MAX_PRIMEMNG_BUFFER_SIZE);
			if (i_bytes > 0) {
				/* Enable USI loopback mode */
				usi_host_loopback (_ev_info->i_socket_fd);
				/* Send command to USI port */
				write(si_serial_port_fd, suc_prime_mng_rcv_buf, i_bytes);
			} else {
				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
			}
		}
	}
}

