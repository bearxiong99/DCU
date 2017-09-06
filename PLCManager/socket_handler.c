/*
 * socket_handler.c
 *
 *  Created on: 22/3/2016
 *      Author: cristian.guemes
 */

#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#include "errno.h"
#include "socket_handler.h"

#ifdef SOCKET_DEBUG_CONSOLE
#	define LOG_SOCKET_DEBUG(a)   printf a
#else
#	define LOG_SOCKET_DEBUG(a)   (void)0
#endif

/* Array to store Socket file descriptors */
static x_socket_t sx_sockets[SOCKET_MAX_NUM];

static fd_set sx_socket_listeners;
static int si_max_listener;

static struct timeval sx_timeout;

void socket_init(void)
{
	/* Prepare list of file descriptor to monitor with "select" */
	FD_ZERO(&sx_socket_listeners);
	si_max_listener = 0;

	/* Configure timeout for listen ports */
	sx_timeout.tv_sec  = 1;
	sx_timeout.tv_usec = 0;

	/* Init sockets info */
	memset(&sx_sockets, (int)SOCKET_ERROR, sizeof(sx_sockets));
}

void socket_restart(void)
{
	int i_idx;

	// reset Socket info
	for(i_idx = 0; i_idx < SOCKET_MAX_NUM; i_idx++) {
		if (sx_sockets[i_idx].i_socket_fd != SOCKET_ERROR) {
			close(sx_sockets[i_idx].i_socket_fd);
			memset(&sx_sockets[i_idx], (int)SOCKET_ERROR, sizeof(sx_sockets[i_idx]));
		}
	}

	FD_ZERO(&sx_socket_listeners);
	si_max_listener = 0;
}


void socket_check_connection(int _app_id, int _fd)
{
	int n;

	/* check if socket is valid for read */
	ioctl(_fd, FIONREAD, &n);

	if (n == 0) {
		/* Socket has been closed... */
		socket_dettach_connection(_app_id, _fd);
	}
}

void socket_append_listener(int _fd)
{
	/* Update MAX num of listener index */
	si_max_listener = (_fd > si_max_listener)? _fd: si_max_listener;

	FD_SET(_fd, &sx_socket_listeners);
}

void socket_remove_listener(int _fd)
{
	/* Update MAX num of listener index */
	si_max_listener = (_fd == si_max_listener)? (si_max_listener - 1): si_max_listener;

	FD_CLR(_fd, &sx_socket_listeners);
}

bool socket_attach_connection(int _app_id, int _fd)
{
	int i;

	for (i = 0; i < SOCKET_MAX_CONN; i++) {
			if (sx_sockets[_app_id].i_conn_fd[i] == SOCKET_UNUSED) {
					sx_sockets[_app_id].i_conn_fd[i] = _fd;
					FD_SET(_fd, &sx_socket_listeners);
					/* Update MAX num of listener index */
					si_max_listener = (_fd > si_max_listener)? _fd: si_max_listener;
					LOG_SOCKET_DEBUG(("Socket: socket_attach_connection(app_id[%u], socket[%u])\r\n", _app_id, _fd));
					return true;
					break;
			}
	}

	return false;
}

void socket_dettach_connection(int _app_id, int _fd)
{
	int i;

	for (i = 0; i < SOCKET_MAX_CONN; i++) {
			if (sx_sockets[_app_id].i_conn_fd[i] == _fd) {
					sx_sockets[_app_id].i_conn_fd[i] = SOCKET_UNUSED;
					close(_fd);
					FD_CLR(_fd, &sx_socket_listeners);
					/* Update MAX num of listener index */
					si_max_listener = (_fd == si_max_listener)? (si_max_listener - 1): si_max_listener;
					LOG_SOCKET_DEBUG(("Socket: socket_dettach_connection(app_id[%u], socket[%u])\r\n", _app_id, _fd));
					break;
			}
	}
}

socket_res_t socket_create_server(int _app_id, int _addr, int _port)
{
	int i_listen_sd;
	int i_on = 1;
	int i_rc;
	struct sockaddr_in x_addr;

	if (_app_id > SOCKET_MAX_NUM) {
		LOG_SOCKET_DEBUG(("Socket: server socket() failed"));
		return(-1);
	}

	i_listen_sd = socket(AF_INET, SOCK_STREAM, 0);
	if (i_listen_sd < 0) {
		LOG_SOCKET_DEBUG(("Socket: server socket() failed"));
		return(-1);
	}

	/* Allow socket descriptor to be reuseable */
	i_rc = setsockopt(i_listen_sd, SOL_SOCKET,  SO_REUSEADDR, &i_on, sizeof(i_on));
	if (i_rc < 0) {
		LOG_SOCKET_DEBUG(("Socket: setsockopt() failed"));
	    close(i_listen_sd);
	    return(-1);
	}

	/* Bind the socket */
	memset(&x_addr, 0, sizeof(struct sockaddr_in));
	x_addr.sin_family      = AF_INET;
	x_addr.sin_addr.s_addr = htonl(_addr);
	x_addr.sin_port        = htons(_port);
	i_rc = bind(i_listen_sd, (struct sockaddr *)&x_addr, sizeof(x_addr));
	if (i_rc < 0) {
		LOG_SOCKET_DEBUG(("Socket: bind() failed"));
		close(i_listen_sd);
		return(-1);
	}

	/* Set the listen back log */
	i_rc = listen(i_listen_sd, 1);
	if (i_rc < 0) {
		LOG_SOCKET_DEBUG(("Socket: listen() failed"));
	    close(i_listen_sd);
	    return(-1);
	}

	/* Add to socket list */
	sx_sockets[_app_id].i_socket_fd = i_listen_sd;
	sx_sockets[_app_id].i_socket_addr = _addr;
	sx_sockets[_app_id].i_socket_port = _port;

	/* Add to SOCKET listeners */
	socket_append_listener(sx_sockets[_app_id].i_socket_fd);

	LOG_SOCKET_DEBUG(("Socket: create (app_id[%u], socket[%u])\r\n", _app_id, i_listen_sd));

	return i_listen_sd;
}

int socket_select(socket_ev_info_t *_event_info)
{
	int i_sel_ret, i_fd_ready;
	int i, c;
	fd_set working_set;

	memcpy(&working_set, &sx_socket_listeners, sizeof(sx_socket_listeners));

	/* Configure timeout for listen ports */
	sx_timeout.tv_sec  = 0;
	sx_timeout.tv_usec = 10000;  /* 10ms */

	/* Wait on file descriptors for data available */
	i_sel_ret = select(si_max_listener + 1, &working_set, NULL, NULL, &sx_timeout);

	if (i_sel_ret == 0) {
		/* Timeout */
		return SOCKET_TIMEOUT;
	}

	if (i_sel_ret < 0) {
		/* Error */
		LOG_SOCKET_DEBUG(("Socket: SOCKET_ERROR errno(%u)\r\n", errno));
		return SOCKET_ERROR;
	}

	/* Get LINK events */
	i_fd_ready = -1;
	for(i = 0; i <= si_max_listener; i++) {
		if (FD_ISSET(i, &working_set)) {
			i_fd_ready = i;
			break;
		}
	}

	if (i_fd_ready < 0) {
		/* Error */
		LOG_SOCKET_DEBUG(("Socket: SOCKET is not setted in master set\r\n"));
		return SOCKET_ERROR;
	}

	/* Create event info */
	for (i = 0; i < SOCKET_MAX_NUM; i++) {
		/* Check Link Socket */
		if (sx_sockets[i].i_socket_fd == i_fd_ready) {
				_event_info->i_app_id = i;
				_event_info->i_event_type = SOCKET_EV_LINK_TYPE;
				_event_info->i_socket_fd = i_fd_ready;
				LOG_SOCKET_DEBUG(("Socket: SOCKET_EV_LINK_TYPE(app_id[%u], socket[%u])\r\n", i, i_fd_ready));
				return SOCKET_SUCCESS;
		} else {
			/* Check Connection Socket */
			for(c = 0; c < SOCKET_MAX_CONN; c++) {
				if (sx_sockets[i].i_conn_fd[c] == i_fd_ready) {
					_event_info->i_app_id = i;
					_event_info->i_socket_fd = i_fd_ready;
					_event_info->i_event_type = SOCKET_EV_DATA_TYPE;
					LOG_SOCKET_DEBUG(("Socket: SOCKET_EV_DATA_TYPE(app_id[%u], socket[%u])\r\n", i, i_fd_ready));
					return SOCKET_SUCCESS;
				}
			}
		}
	}

	LOG_SOCKET_DEBUG(("Socket: SOCKET_GENERIC_ERROR\r\n"));
	return SOCKET_ERROR;
}

void socket_accept_conn(socket_ev_info_t *_event_info)
{
	int fd, opt_flag;

	fd = accept(_event_info->i_socket_fd, NULL, NULL);

	opt_flag = 1;
	setsockopt(_event_info->i_socket_fd, IPPROTO_TCP, TCP_NODELAY, (char *) &opt_flag, sizeof(opt_flag));

	if (socket_attach_connection(_event_info->i_app_id, fd) == false) {
		LOG_SOCKET_DEBUG(("Socket: ERROR atttach conn(app_id[%u], socket[%u])\r\n", _event_info->i_app_id, fd));
		close(fd);
	} else {
		LOG_SOCKET_DEBUG(("Socket: atttach conn(app_id[%u], socket[%u])\r\n", _event_info->i_app_id, fd));
	}
}

int socket_get_fd(int _app_id) {
	if (_app_id < SOCKET_MAX_NUM) {
		return sx_sockets[_app_id].i_socket_fd;
	} else {
		return -1;
	}
}

int socket_get_connections_fds(int _app_id, int *_puc_connections)
{
	int i_conn_num, i_conn_idx;
	int *p_conn;

	i_conn_num = 0;
	p_conn = _puc_connections;

	for(i_conn_idx = 0; i_conn_idx < SOCKET_MAX_CONN; i_conn_idx++) {
		*p_conn++ = sx_sockets[_app_id].i_conn_fd[i_conn_idx];
		if (sx_sockets[_app_id].i_conn_fd[i_conn_idx] > 0) {
			i_conn_num++;
		}
	}

	return i_conn_num;
}
