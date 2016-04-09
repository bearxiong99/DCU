/*
 * socket_handler.c
 *
 *  Created on: 22/3/2016
 *      Author: cristian.guemes
 */

#include <sys/types.h>
#include <stdbool.h>

#define SOCKET_MAX_NUM           16
#define SOCKET_MAX_CONN          5

typedef enum socket_result {
	SOCKET_SUCCESS = 0,
	SOCKET_ERROR = -1,
	SOCKET_UNUSED = -1,
	SOCKET_TIMEOUT = -2
} socket_res_t;

typedef enum socket_event_type {
	SOCKET_EV_LINK_TYPE = 0,
	SOCKET_EV_DATA_TYPE = 1
} socket_event_type_t;

typedef struct socket_ev_info {
	int i_app_id;
	int i_socket_fd;
	socket_event_type_t i_event_type;
} socket_ev_info_t;

typedef struct {
	int i_socket_fd;
	int i_socket_port;
	int i_socket_addr;
	int i_conn_fd[SOCKET_MAX_CONN];
} x_socket_t;

void socket_init(void);
void socket_restart(void);
void socket_check_connection(int _app_id, int _fd);
socket_res_t socket_create_server(int _app_id, int _addr, int _port);
void socket_append_listener(int _fd);
void socket_remove_listener(int _fd);
bool socket_attach_connection(int _app_id, int _fd);
void socket_dettach_connection(int _app_id, int _fd);
int socket_select(socket_ev_info_t *_event_info);
void socket_accept_conn(socket_ev_info_t *_event_info);
int socket_get_fd(int _app_id);
int socket_get_connections_fds(int _app_id, int *_puc_connections);

