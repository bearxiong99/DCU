#ifndef DLMS_EMU_H_INCLUDED
#define DLMS_EMU_H_INCLUDED

#include <stdint.h>
#include "socket_handler.h"

/* define load curve length in the dlms query */
#define S02_NUMBEROFDAYS 2

/* max number of nodes */
#define DLMS_EMU_MAX_NUM_NODES_CONNECTED     (15 + 1)

/* time waiting for next node cycles */
#define DLMS_EMU_TIME_WAITING_IDLE           (45)

/* waiting time after cycling all nodes */
#define DLMS_EMU_TIME_BETWEEEN_CYCLES        (5)

/* waiting time before cycling the next node */
#define DLMS_EMU_TIME_WAIT_NODES             (10)

/* time waiting a dlms message response */
#define DLMS_EMU_TIME_WAIT_RESPONSE          (180)

/* time waiting a dlms message request */
#define DLMS_EMU_TIME_WAIT_REQUEST           (180)

/* time waiting after received a response message */
#define DLMS_EMU_TIME_WAIT_BETWEEN_MESSAGES  (100)

#define MAX_DLMSEMU_SOCKET_SIZE              100

#define DLMS_EMU_SN432_MAC_SIZE		         16

enum dlms_emu_commands {
	DLMS_EMU_CMD_STOP_CYCLES = 0x30,
	DLMS_EMU_CMD_START_CYCLES = 0x31,
	DLMS_EMU_CMD_INVALD
};

typedef struct {
	uint16_t dst_address;
	uint8_t serial_number[16];
	uint8_t len_serial_number;
	uint8_t mac[6];
} x_node_list_t;

void dlms_emu_init(int _app_id);
void dlms_emu_callback(socket_ev_info_t *_ev_info);

#endif /* DLMS_EMU_H_INCLUDED */
