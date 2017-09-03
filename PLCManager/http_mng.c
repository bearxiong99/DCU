#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <asm/types.h>
#include <arpa/inet.h>
#include "socket_handler.h"
#include "http_mng.h"
#include "net_info_mng.h"
#include "plcmanager.h"

#ifdef HTTP_DEBUG_CONSOLE
#	define LOG_HTTP_DEBUG(a)   printf a
#else
#	define LOG_HTTP_DEBUG(a)   (void)0
#endif

/* http client socket descriptors */
static bool sb_http_connect;
static int si_http_socket_fd;
static struct sockaddr_in serverAddress;

/* http server socket descriptors */
static int si_http_link_fd;
static int si_http_data_fd;

#define MAX_HTTP_BUFFER_SIZE     1000

static unsigned char suc_http_rx_buf[MAX_HTTP_BUFFER_SIZE];
static unsigned char suc_http_tx_buf[MAX_HTTP_BUFFER_SIZE];

#define HOST                     "127.0.0.1"
#define PORT                     4000
#define MAX_BUFF_HTTP_CMD        50

static const char sc_com = '"';

static uint16_t _build_post_query(uint8_t uc_cmd, void *pv_data)
{
	uint16_t us_len;

	char *tpl = "POST / HTTP/1.1\r\nAccept: application/json\r\nContent-Type: application/json\r\nHost: %s:%u\r\ncontent-length: %u\r\n\r\n%s";
	char page[20];

	(void)pv_data;

	switch(uc_cmd) {
	case LNXCMS_UPDATE_DASHBOARD:
	case LNXCMS_UPDATE_PATHLIST:
	case LNXCMS_UPDATE_ROUNDTIME:
	case LNXCMS_UPDATE_THROUGHPUT:
		sprintf(page, "[{%clnxcmd%c:%u}]", sc_com, sc_com, uc_cmd);
		us_len = sprintf((char *)suc_http_tx_buf, tpl, HOST, PORT, strlen(page), page);
		break;

	default:
		us_len = 0;
	}

	return us_len;
}

/**
 * \brief Process command/response received from NODE JS.
 */
static void _http_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;
    uint8_t *puc_cmd;

    puc_buf = buf;

    //LOG_HTTP_DEBUG(("%s\r\n\r\n", puc_buf));

    puc_cmd = (uint8_t *)strstr(puc_buf, "webcmd");
    if (puc_cmd) {
    	/* Filter only web commands */
    	puc_cmd += 8;

    	/* Pass web cmd to net info */
    	net_info_webcmd_process(puc_cmd);
    }

    memset(suc_http_rx_buf, 0x00, MAX_HTTP_BUFFER_SIZE);
}

void http_mng_init(void)
{
	/* Set the NODE JS server address */
	serverAddress.sin_addr.s_addr = inet_addr(HOST);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);

	memset(suc_http_rx_buf, 0x00, MAX_HTTP_BUFFER_SIZE);

}

void http_mng_send_cmd(uint8_t uc_cmd, void *pv_data)
{
	uint16_t us_size_qry;

	// Build HTTP linux command
	us_size_qry = _build_post_query(uc_cmd, pv_data);
	if (us_size_qry == 0) {
		return;
	}

	/* Send HTTP REST API to the server */
	/* Check internal http connection */
	if (!sb_http_connect) {
		/* Create Client socket to connect NODE JS server */
		si_http_socket_fd = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
		if (si_http_socket_fd == -1)	{
			LOG_HTTP_DEBUG(("Could not create socket\n"));
			return;
		}

		//Connect to remote server
		if (connect(si_http_socket_fd , (struct sockaddr *)&serverAddress , sizeof(serverAddress)) == SOCKET_ERROR) {
			LOG_HTTP_DEBUG(("connect failed. Error\n"));
			sb_http_connect = false;
		} else {
			LOG_HTTP_DEBUG(("Connected\n"));
			sb_http_connect = true;

			/* Add listener to USI port */
			socket_attach_connection(PLC_MNG_HTTP_MNG_APP_ID, si_http_socket_fd);
		}
	}

	if (sb_http_connect) {
		if (write(si_http_socket_fd, (char *)suc_http_tx_buf, us_size_qry) == SOCKET_ERROR) {
			LOG_HTTP_DEBUG(("Cannot sent message\n"));
			socket_dettach_connection(PLC_MNG_HTTP_MNG_APP_ID, si_http_socket_fd);
			sb_http_connect = false;
		} else {
			LOG_HTTP_DEBUG(("PLC message sended:\r\n%u\r\n", uc_cmd));
		}
	}

}

void http_mng_callback(socket_ev_info_t *_ev_info)
{
	if (_ev_info->i_socket_fd >= 0) {
		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
			/* Manage LINK */
			si_http_link_fd = _ev_info->i_socket_fd;
			socket_accept_conn(_ev_info);
		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
			/* Receive DATA */
			ssize_t i_bytes;
			si_http_data_fd = _ev_info->i_socket_fd;
			/* Read data from Socket */
			i_bytes = read(_ev_info->i_socket_fd, suc_http_rx_buf, MAX_HTTP_BUFFER_SIZE);
			if (i_bytes > 0) {
				// process message
				_http_rcv_cmd(suc_http_rx_buf, i_bytes);

				// Close the socket
				socket_dettach_connection(PLC_MNG_HTTP_MNG_APP_ID, _ev_info->i_socket_fd);

				if (_ev_info->i_socket_fd == si_http_socket_fd) {
					// Update connection status
					sb_http_connect = false;
				}
			} else {
				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
			}
		}
	}
}
