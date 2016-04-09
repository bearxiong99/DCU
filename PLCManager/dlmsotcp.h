#ifndef __DLMS_O_TCP__
#define __DLMS_O_TCP__

#include <unistd.h>
#include <stdint.h>

#define VERSION_HI	                     0x01
#define VERSION_LOW                      0x01

#define TRUE  1
#define FALSE 0

#define DLMSoTCPPORT                     4059
#define MAX_NUM_NODES_CONNECTED          2000
#define CL_432_INVALID_ADDRESS           (0xFFFF)
#define SERIAL_NUMBER_432_MAC		     16

#define MAX_BUFFER_SIZE                  1514

#define MAX_MSG_LIST                     5
#define MAX_432_SEND_RETRY               2

typedef struct {
	int  i_verbose;
	int  i_verbose_level;
	char sz_tty_name[255];
	unsigned ui_baudrate;
	unsigned ui_server_port;
	unsigned ui_internal_server_port;
} td_x_args;

struct x_dlms_msg {
    uint32_t ui_timestamp;
    uint16_t us_retries;
    uint16_t us_dst;
    uint16_t us_lsap;
    uint16_t us_length;
    uint8_t data[MAX_BUFFER_SIZE];
    struct x_dlms_msg * px_link;
};

struct x_dlms_msg_list {
	struct x_dlms_msg * px_head;
	struct x_dlms_msg * px_tail;
	uint16_t us_count;
};

int dlmsotcp_init(int _app_id);
void dlmsotcp_callback(socket_ev_info_t *_ev_info);
//int dlmsotcp_process();
void dlmsotcp_close_432();

#endif /* __DLMS_O_TCP__ */
