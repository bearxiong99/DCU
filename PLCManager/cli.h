#ifndef __CLI_O_TCP__
#define __CLI_O_TCP__

#include <unistd.h>
#include <stdint.h>

#define VERSION_HI	                     0x01
#define VERSION_LOW                      0x01

#define TRUE  1
#define FALSE 0

#define CLIoTCPPORT                      9000
#define MAX_CLI_BUFFER_SIZE              1000

enum cli_commands {
	PRIME_CMD_PLME_GET_PIB = 0,
	PRIME_CMD_PLME_GET_PIB_RSP = 1,
	PRIME_CMD_PLME_SET_PIB = 2,
	PRIME_CMD_PLME_SET_PIB_RSP = 3,
	PRIME_CMD_MLME_GET_PIB = 4,
	PRIME_CMD_MLME_GET_PIB_RSP = 5,
	PRIME_CMD_MLME_SET_PIB = 6,
	PRIME_CMD_MLME_SET_PIB_RSP = 7,
	PRIME_CMD_INVALD
};

void cli_init(int _app_id);
void cli_callback(socket_ev_info_t *_ev_info);

#endif /* __CLI_O_TCP__ */
