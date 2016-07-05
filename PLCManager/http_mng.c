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
#include "debug.h"

/* http socket descriptors */
static bool sb_http_connect;
static int httpsocketfd;
static struct sockaddr_in clientAddress, serverAddress;

#define MAX_HTTP_BUFFER_SIZE             1000

static unsigned char suc_http_rx_buf[MAX_HTTP_BUFFER_SIZE];
static unsigned char suc_http_tx_buf[MAX_HTTP_BUFFER_SIZE];

#define HOST "127.0.0.1"  //"coding.debuntu.org"
#define CMD0 "/cmd"
#define PORT 3000
#define USERAGENT "HTMLGET 1.0"

static void build_get_query(char *page)
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

/**
 * \brief Process messages received from CLI.
 * Unpack CLI protocol command
 */
static void _http_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;

    puc_buf = buf;

}

void http_mng_init(void)
{
	// Example
	build_get_query(CMD0);

	//Create socket
	httpsocketfd = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
	if (httpsocketfd == -1)	{
		PRINTF("Could not create socket\n");
		return;
	}

	/* Set the server address */
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT);

	//Connect to remote server
	if (connect(httpsocketfd , (struct sockaddr *)&serverAddress , sizeof(serverAddress)) < 0) {
		PRINTF("connect failed. Error\n");
		sb_http_connect = false;
	} else {
		PRINTF("Connected\n");
		sb_http_connect = true;
	}
}

void http_mng_process(void)
{
	//uint16_t us_data_len;

	//Receive a reply from the server
	if (sb_http_connect) {
		if (write(httpsocketfd, (char *)suc_http_tx_buf, strlen(suc_http_tx_buf)) == -1) {
			PRINTF ("Cannot sent message");
		}

		//us_data_len = read(httpsocketfd , suc_http_rx_buf , sizeof(suc_http_rx_buf));
		//if(us_data_len) {
		//	PRINTF("recv mesg\n");
		//	_http_rcv_cmd(suc_http_rx_buf, us_data_len);
		//}
	}
}
