#ifndef DLMS_EMU_COORD_H_INCLUDED
#define DLMS_EMU_COORD_H_INCLUDED

#include <stdint.h>
#include <stdio.h>

#include "config.h"


/* --- DEFINE APP BEHAVIOUR --- */
/* Adapt DLMS EMU start cycles dynamically to network stability. Always before the TIMER_WAITING_START has expired */
//#define DLMS_EMU_WAIT_REG_NODES


/**************** App feature habilitation ************************/

/**** Enable Short Cycles ****/
//#define DLMS_EMU_ENABLE_SHORT_CYCLES
/**** Enable DLMS EMU path request. PREQ will request to nodes always before launch next cycle ****/
//#define DLMS_EMU_ENABLE_PATH_REQ
/**** Ping Cycle Configuration ****/
//#define DLMS_EMU_ENABLE_PING_CYCLE

/* Build a JSON object with Network Topology and send it through Serial port */
#define JSON_ENABLE


/* --- DEFINE APP LENGTHS --- */
/* Maximum length of IPv6 PDUs for DLMS application */
#define MAX_LENGTH_IPv6_PDU                   1200

/* IPv6 header length */
#define IPv6_HEADER_LENGTH                    40

/* UDP header length */
#define UDP_HEADER_LENGTH                     8

/* Maximum size of DLMS received data */
#define DLMS_MAX_RX_SIZE                      200


/**** Ping Configuration ****/
/* Ping Cycle Period */
#define DLMS_EMU_PING_CYCLE_TIMER_INTERVAL       3
/* Ping Cycle Timeout for response */
//#define DLMS_EMU_PING_CYCLE_TIMEOUT              60
/* Ping Cycle Time to live */
//#define DLMS_EMU_PING_CYCLE_TTL                  10
/* Ping Cycle Msg Lenght */
//#define DLMS_EMU_PING_CYCLE_LEN                  10


/* --- DEFINE APP DLMS OPTIONS --- */
/* Define load curve lenght in the dlms query */
//#define S02_NUMBEROFDAYS                      1

/* Max Number of devices defined by Bootstrap module */
#define DLMS_MAX_DEV_NUM                      G3_MAX_DEV

/* G3 Coordinator PAN defined by Bootstrap module */
#define DLMS_G3_COORD_PAN_ID                  G3_COORDINATOR_PAN_ID

/* G3 Coordinator PAN defined by Bootstrap module */
#define DLMS_INVALID_ADDRESS                  0




/* --- DEFINE APP TIMERS --- */
/* Time between cycles in SECONDS */
//#define TIMER_BETWEEN_CYCLES                  20

/* Timeout for messages in SECONDS */
//#define TIME_OUT_DATA_MSG                     50

/* Time between messages in SECONDS */
//#define TIMER_BETWEEN_MESSAGES                1

/* Time to wait before start cycling in SECONDS (approx. 60 sec per device) */
//#define TIMER_WAITING_START                   120 /* 2 minutes */

/* Time max PATH REQ cfm in SECONDS (only in case of uncomment DLMS_EMU_ENABLE_PATH_REQ) */
//#define TIME_MAX_WAIT_PREQ_CFM                120 /* 2 min */

/* Time max Path request process in SECONDS (only in case of uncomment DLMS_EMU_ENABLE_PATH_REQ) */
//#define TIME_MAX_BETWEEN_PREQ_WITOUT_DATA     60 /* 1 min */


typedef struct x_node_list{
	uint16_t us_short_address;
	uint8_t puc_extended_address[8];
} x_node_list_t;

typedef struct x_net_info {
	uint8_t *puc_extended_addr;
	uint16_t us_num_nodes;
	uint16_t us_num_path_nodes;
	x_node_list_t *px_node_list;
	struct TAdpPathDiscoveryConfirm *px_path_nodes;
} x_net_info_t;

void dlms_emu_init(uint8_t *puc_ext_addr);
void dlms_emu_update(void);
void dlms_emu_process(void);
void dlms_emu_join_node(uint8_t *puc_extended_address, uint16_t us_short_address);
void dlms_emu_leave_node(uint16_t us_short_address);
void dlms_emu_data_ind_handler(void *pvDataIndication);
void dlms_emu_path_node_cfm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm);

#endif /* DLMS_EMU_COORD_H_INCLUDED */
