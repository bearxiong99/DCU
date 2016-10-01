/*
 * main.c
 *
 *  Created on: 22/3/2016
 *      Author: cristian.guemes
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <asm/types.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <unistd.h>

#include "socket_handler.h"
#include "plcmanager.h"
#include "net_info_mng.h"
#include "app_debug.h"
#include "http_mng.h"
#include "sniffer.h"
#include "usi_host.h"
#include "hal_utils.h"


typedef struct {
	char sz_tty_name[255];
	unsigned ui_baudrate;
} x_serial_args_t;

/* Manage Callbacks of applications */
static pf_app_callback_t app_cbs[PLC_MNG_MAX_APP_ID];

int main(int argc, char** argv)
{
	socket_res_t i_socket_res;
	int pi_usi_fds;
	x_serial_args_t serial_args = {"/dev/ttyUSB1", 115200}; //{"/dev/ttyS0", 115200};

	PRINTF_INIT();

	/* Init callbacks for applications */
	memset(&app_cbs, 0, sizeof(app_cbs));

	/* init sockets */
	socket_init();

	/* Open SNIFFER server socket */
	i_socket_res =  socket_create_server(PLC_MNG_SNIFFER_APP_ID, INADDR_ANY, PLC_MNG_SNIFFER_APP_PORT);
	if (i_socket_res == SOCKET_ERROR) {
		PRINTF("Cannot open Sniffer Server socket.");
		exit(-1);
	}

	/* Open ADP internal server socket */
	i_socket_res =  socket_create_server(PLC_MNG_NET_INFO_APP_ID, INADDR_ANY, PLC_MNG_NET_INFO_APP_PORT);
	if (i_socket_res == SOCKET_ERROR) {
		PRINTF("Cannot open Net Info Manager internal Server socket.");
		exit(-1);
	}

	/* Open HTTP internal NODE server socket */
	i_socket_res =  socket_create_server(PLC_MNG_HTTP_MNG_APP_ID, INADDR_ANY, PLC_MNG_HTTP_MNG_APP_PORT);
	if (i_socket_res == SOCKET_ERROR) {
		PRINTF("Cannot open HTTP internal NODE Server socket.");
		exit(-1);
	}

	/* USI_HOST serial connection. */
	usi_host_init();
	pi_usi_fds = usi_host_open(serial_args.sz_tty_name, serial_args.ui_baudrate);
	if (pi_usi_fds == SOCKET_ERROR) {
		PRINTF("Cannot open Serial Port socket\n");
		exit(-1);
	} else {
		/* Add listener to USI port */
		socket_attach_connection(PLC_MNG_USI_APP_ID, pi_usi_fds);
	}

	/* Register SNIFFER APP callback */
	app_cbs[PLC_MNG_SNIFFER_APP_ID] = sniffer_callback;
	/* Init Sniffer APP : Serve to SNIFFER tool. */
	sniffer_init(PLC_MNG_SNIFFER_APP_ID, pi_usi_fds);

	/* Register Net Info Manager APP callback */
	app_cbs[PLC_MNG_NET_INFO_APP_ID] = net_info_mng_callback;
	/* Init CLI app */
	net_info_mng_init(PLC_MNG_NET_INFO_APP_ID);

	/* Register HTTP internal NODE APP callback */
	app_cbs[PLC_MNG_HTTP_MNG_APP_ID] = http_mng_callback;
	/* Init HTTP client manager to connect with NODE server */
	http_mng_init();

	while(1) {
		int i_res;
		socket_ev_info_t socket_evet_info;

		/* Wait on SOCKETs for data available */
		i_res = socket_select(&socket_evet_info);

		switch (i_res) {
				case SOCKET_TIMEOUT:
					/* Process USI */
					usi_host_process();
					//http_mng_send_cmd();
					net_info_mng_process();
					break;

				case SOCKET_ERROR:
					/* Reset sockets */
					//socket_restart();
					/* Reset applications */
					/* ... */
					break;

				case SOCKET_SUCCESS:
					/* Send event to APP */
					if (socket_evet_info.i_app_id == PLC_MNG_USI_APP_ID) {
						/* Process USI */
						usi_host_process();
						//net_info_mng_process();
					} else {
						/* Launch APP callback */
						if (app_cbs[socket_evet_info.i_app_id] != NULL) {
							app_cbs[socket_evet_info.i_app_id](&socket_evet_info);
						}
					}
					break;
		}
	}

	return 0;
}
