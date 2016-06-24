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
#include "debug.h"
#include "cli.h"
#include "dlms_emu.h"
#include "sniffer.h"
#include "prime_manager.h"
#include "dlmsotcp.h"
#include "usi_host.h"
#include "hal_utils.h"


typedef struct {
	char sz_tty_name[255];
	unsigned ui_baudrate;
} x_serial_args_t;

/* Manage Callbacks of applications */
static pf_app_callback_t app_cbs[PLC_MNG_MAX_APP_ID];

static int _getParseInt(char * _szStr, int *_iVal)
{
	char *endptr;
	long tmp;

	errno = 0;    /* To distinguish success/failure after call */
	tmp = strtol(_szStr, &endptr, 10);

	/* Check for various possible errors */
	if ((errno == ERANGE && (tmp == LONG_MAX || tmp == LONG_MIN))
	                   || (errno != 0 && tmp == 0))
	{
	    return -1;
	}

	if (endptr == _szStr)
	{
	    return -1;
	}

	/* If we got here, strtol() successfully parsed a number */
    *_iVal = (int) tmp;

    return 0;
}

static int _parse_arguments(int argc, char** argv, x_serial_args_t *serial_args)
{
	int c;

	while (1)
	{
		static struct option long_options[] = {
	    		{"tty",      required_argument, 0, 't'},
	    		{"baudrate", required_argument, 0, 'b'},
	    		{0, 0, 0, 0}
	   };

		/* getopt_long stores the option index here. */
	   int option_index = 0;

	   c = getopt_long (argc, argv, "t:b", long_options, &option_index);

	   /* Detect the end of the options. */
	   if (c == -1) {
		   break;
	   }

	   switch (c)
	   {
	   	   case 0:
	   		   /* If this option set a flag, do nothing else now. */
	           if (long_options[option_index].flag != 0) {
	        	   break;
	           }
	           break;

	   		case 'b':
	   			if(_getParseInt(optarg,(int*)&serial_args->ui_baudrate) != 0) {
	   		   		return -1;
	   		   	}
	   		   	break;

	   		case 't':
	   			strncpy(serial_args->sz_tty_name, optarg, 255);
	   			break;

	   		case 'h':
	   		case '?':
	   			/* getopt_long already printed an error message. */
	   			return -1;
	   			break;
           }
    }
	return 0;
}


int main(int argc, char** argv)
{
	socket_res_t i_socket_res;
	int pi_usi_fds;
#if BOARD == BOARD_SAMA5EK
	x_serial_args_t serial_args = {"/dev/ttyUSB0", 115200};
#elif BOARD == BOARD_ARIA
	x_serial_args_t serial_args = {"/dev/ttyS0", 115200};
#else
	error No board defined
#endif


	PRINTF_INIT();

	/* Load command line parameters */
	if (_parse_arguments(argc,argv, &serial_args) < 0) {
		PRINTF("PLC Manager v%d,%d\n", PLC_MNG_VERSION_HI, PLC_MNG_VERSION_LO);
		PRINTF("Error, check arguments.\n");
		PRINTF("\t-t tty                : tty device connecting to a base node, default: /dev/ttyUSB0 \n");
		PRINTF("\t-b baudrate           : tty baudrate configuration, default: 115200\n");
		exit(-1);
	}

	/* Init callbacks for applications */
	memset(&app_cbs, 0, sizeof(app_cbs));

	/* init sockets */
	socket_init();

	/* Open SNIFFER server socket */
	//i_socket_res =  socket_create_server(PLC_MNG_SNIFFER_APP_ID, INADDR_ANY, PLC_MNG_SNIFFER_APP_PORT);
	//if (i_socket_res == SOCKET_ERROR) {
	//	PRINTF("Cannot open Sniffer Server socket.");
	//	exit(-1);
	//}

	/* Open PRIME Manager server socket */
	//i_socket_res =  socket_create_server(PLC_MNG_PRIMEMNG_APP_ID, INADDR_ANY, PLC_MNG_PRIMEMNG_APP_PORT);
	//if (i_socket_res == SOCKET_ERROR) {
	//	PRINTF("Cannot open Prime Manager Server socket.");
	//	exit(-1);
	//}

	/* Open CLI internal server socket */
	i_socket_res =  socket_create_server(PLC_MNG_CLI_APP_ID, INADDR_LOOPBACK, PLC_MNG_CLI_APP_PORT);
	//i_socket_res =  socket_create_server(PLC_MNG_CLI_APP_ID, INADDR_ANY, PLC_MNG_CLI_APP_PORT);
	if (i_socket_res == SOCKET_ERROR) {
		PRINTF("Cannot open CLI internal Server socket.");
		exit(-1);
	}

	/* Open DLMSoTCP internal server socket */
	//i_socket_res =  socket_create_server(PLC_MNG_DLMSoTCP_APP_ID, INADDR_ANY, PLC_MNG_DLMSoTCP_APP_PORT);
	//if (i_socket_res == SOCKET_ERROR) {
	//	PRINTF("Cannot open DLMSoTCP internal Server socket.");
	//	exit(-1);
	//}

	/* Open DLMS_EMU internal server socket */
	//i_socket_res =  socket_create_server(PLC_MNG_DLMS_APP_ID, INADDR_ANY, PLC_MNG_DLMS_APP_PORT);
	//if (i_socket_res == SOCKET_ERROR) {
	//	PRINTF("Cannot open DLMSoTCP internal Server socket.");
	//	exit(-1);
	//}

	/* USI_HOST serial connection. */
	//usi_host_init();
	//pi_usi_fds = usi_host_open(serial_args.sz_tty_name, serial_args.ui_baudrate);
	//if (pi_usi_fds == SOCKET_ERROR) {
	//	PRINTF("Cannot open Serial Port socket\n");
	//	exit(-1);
	//} else {
	//	/* Add listener to USI port */
	//	socket_attach_connection(PLC_MNG_USI_APP_ID, pi_usi_fds);
	//}

	/* Register SNIFFER APP callback */
	//app_cbs[PLC_MNG_SNIFFER_APP_ID] = sniffer_callback;
	/* Init Sniffer APP : Serve to SNIFFER tool. */
	//sniffer_init(PLC_MNG_SNIFFER_APP_ID, pi_usi_fds);

	/* Register PRIME MNG APP callback */
	//app_cbs[PLC_MNG_PRIMEMNG_APP_ID] = prime_manager_callback;
	/* Init Prime Manager APP : Serve to Prime Manager Tool. */
	//prime_manager_init(PLC_MNG_PRIMEMNG_APP_ID, pi_usi_fds);

	/* Register CLI internal APP callback */
	app_cbs[PLC_MNG_CLI_APP_ID] = cli_callback;
	/* Init CLI app */
	cli_init(PLC_MNG_CLI_APP_ID);

	/* Register CLI internal APP callback */
	//app_cbs[PLC_MNG_DLMSoTCP_APP_ID] = dlmsotcp_callback;
	/* Init CLI app */
	//dlmsotcp_init(PLC_MNG_DLMSoTCP_APP_ID);

	/* Register CLI internal APP callback */
	//app_cbs[PLC_MNG_DLMS_APP_ID] = dlms_emu_callback;
	/* Init CLI app */
	//dlms_emu_init(PLC_MNG_DLMS_APP_ID);

	while(1) {
		int i_res;
		socket_ev_info_t socket_evet_info;

		/* Wait on SOCKETs for data available */
		i_res = socket_select(&socket_evet_info);

		switch (i_res) {
				case SOCKET_TIMEOUT:
					/* Process USI */
					//usi_host_process();
					/* Process CLI */
					cli_process();
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
						//usi_host_process();
						/* Process CLI */
						cli_process();
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
