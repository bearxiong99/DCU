#ifndef __CLI_O_TCP__
#define __CLI_O_TCP__

#include <unistd.h>
#include <stdint.h>
#include "socket_handler.h"

#define VERSION_HI	                     0x01
#define VERSION_LOW                      0x01

#define TRUE  1
#define FALSE 0

#define CLIoTCPPORT                      9000
#define MAX_CLI_BUFFER_SIZE              1000

enum cli_get_info_status {
	PRIME_INFO_GET_MAC,
	PRIME_INFO_GET_FWV,
	PRIME_INFO_GET_VID,
	PRIME_INFO_GET_PID,
	PRIME_INFO_WAIT_RSP,
	PRIME_INFO_CREATE_FILE,
	PRIME_INFO_OK
};

enum cli_commands {
	PRIME_CMD_UPDATE_NODE_LIST = 0x30,  // Create an xml file with the content of the registered node list
	PRIME_CMD_PLME_GET_PIB,
	PRIME_CMD_PLME_GET_PIB_RSP,
	PRIME_CMD_PLME_SET_PIB,
	PRIME_CMD_PLME_SET_PIB_RSP,
	PRIME_CMD_MLME_GET_PIB,
	PRIME_CMD_MLME_GET_PIB_RSP,
	PRIME_CMD_MLME_GET_LIST_PIB,
	PRIME_CMD_MLME_GET_LIST_PIB_RSP,
	PRIME_CMD_MLME_SET_PIB,
	PRIME_CMD_MLME_SET_PIB_RSP,
	PRIME_CMD_INVALD
};

typedef struct {
	uint16_t us_lnid;
	uint16_t us_lsid;
	uint16_t us_sid;
	uint8_t uc_state;
	uint8_t uc_level;
	uint8_t uc_mac_cap1;
	uint8_t uc_mac_cap2;
	uint8_t mac_addr[6];
} x_cli_node_reg_t;

typedef struct {
	uint16_t us_vendor_id;
	uint16_t us_product_id;
	uint8_t fw_version[32];
	uint8_t mac_addr[6];
} x_cli_prime_info_t;

void cli_init(int _app_id);
void cli_process(void);
void cli_callback(socket_ev_info_t *_ev_info);

#endif /* __CLI_O_TCP__ */
