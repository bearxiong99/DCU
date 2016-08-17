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
#include "app_debug.h"
#include "plcmanager.h"

/* http client socket descriptors */
static bool sb_http_connect;
static int si_http_socket_fd;
static struct sockaddr_in serverAddress;

/* http server socket descriptors */
static int si_http_link_fd;
static int si_http_data_fd;

#define MAX_HTTP_BUFFER_SIZE             1000

static unsigned char suc_http_rx_buf[MAX_HTTP_BUFFER_SIZE];
static unsigned char suc_http_tx_buf[MAX_HTTP_BUFFER_SIZE];

#define HOST "127.0.0.1"  //"coding.debuntu.org"
#define CMD0 "/cmd"
#define PORT 3000
#define USERAGENT "HTMLGET 1.0"

static void _build_get_query(char *page)
{
	//char *query;
	char *getpage = page;
	char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
	if(getpage[0] == '/'){
		getpage = getpage + 1;
		PRINTF("Removing leading \"/\", converting %s to %s\n", page, getpage);
	}
	// -5 is to consider the %s %s %s in tpl and the ending \0
	//query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
	sprintf((char *)suc_http_tx_buf, tpl, getpage, HOST, USERAGENT);
	//return query;
}
/*
static void _build_post_query(char *page)
{
	//char *query;
	char *getpage = page;
	char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
	if(getpage[0] == '/'){
		getpage = getpage + 1;
		PRINTF("Removing leading \"/\", converting %s to %s\n", page, getpage);
	}
	// -5 is to consider the %s %s %s in tpl and the ending \0
	//query = (char *)malloc(strlen(host)+strlen(getpage)+strlen(USERAGENT)+strlen(tpl)-5);
	sprintf((char *)suc_http_tx_buf, tpl, getpage, HOST, USERAGENT);
	//return query;
}
*/
/**
 * \brief Process command/response received from NODE JS.
 */
static void _http_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;
    bool b_cmd;

    puc_buf = buf;

    printf("%s\r\n\r\n", puc_buf);

    b_cmd = true;
    // Analizar el contenido del mensaje para saber si es respuesta o comando
    if (b_cmd) {
    	// NODE RESPONSE


    } else {
    	// NODE COMMAND

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

/*void http_mng_process(void)
{
	uint16_t us_data_len;

	//Receive a reply from the server
	if (sb_http_connect) {
		us_data_len = read(si_http_socket_fd , suc_http_rx_buf , MAX_HTTP_BUFFER_SIZE);
		if (us_data_len) {
			PRINTF("recv mesg\n");
			_http_rcv_cmd(suc_http_rx_buf, us_data_len);

			// close the socket
			socket_dettach_connection(PLC_MNG_HTTP_MNG_APP_ID, si_http_socket_fd);

			// Update connection status
			sb_http_connect = false;
		}
	}
}*/

void http_mng_send_cmd(void)
{
	bool b_send_cmd = false;

	// Build HTTP sentence
	_build_get_query(CMD0);
	//_build_post_query(CMD0);

	/* Send HTTP REST API to the server */
	if (b_send_cmd) {
		/* Check internal http connection */
		if (!sb_http_connect) {
			/* Create Client socket to connect NODE JS server */
			si_http_socket_fd = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
			if (si_http_socket_fd == -1)	{
				PRINTF("Could not create socket\n");
				return;
			}

			//Connect to remote server
			if (connect(si_http_socket_fd , (struct sockaddr *)&serverAddress , sizeof(serverAddress)) == SOCKET_ERROR) {
				PRINTF("connect failed. Error\n");
				sb_http_connect = false;
			} else {
				PRINTF("Connected\n");
				sb_http_connect = true;

				/* Add listener to USI port */
				socket_attach_connection(PLC_MNG_HTTP_MNG_APP_ID, si_http_socket_fd);
			}
		}

		if (sb_http_connect) {
			if (write(si_http_socket_fd, (char *)suc_http_tx_buf, strlen((char *)suc_http_tx_buf)) == SOCKET_ERROR) {
				PRINTF ("Cannot sent message");
			}
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
