#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "cl_432_defs.h"
#include "mac_defs.h"
#include "mac_pib.h"
#include "prime_api_host.h"
#include "prime_api_defs_host.h"
#include "ifacePrime_api.h"
#include "ifaceMngLayer.h"

#include "socket_handler.h"

#include "dlms_emu.h"
#include "dlms_emu_report.h"

#include "debug.h"

/* Configure Number of steps and lengths */
#define NUM_STEPS                          5

#define DLMS_SO2_NUMBER_OF_BLOCKS         (S02_NUMBEROFDAYS * 24 / 4)

enum {
	DLMS_VALID_MESSAGE,
	DLMS_INVALID_MESSAGE,
};

enum {
	DLMS_CYCLE_SUCCESS,
	DLMS_CYCLE_FAIL,
};

typedef enum {
	CMD_SEND,
	CMD_CONFIRM,
	CMD_INDICATION,
	CMD_TIMEOUT
} cmd_cycles_t;

typedef enum {
	ASSOCIATION_REQUEST,
	DATE_REQUEST,
	SO2_REQUEST,
	NEXTBLOCK_REQUEST,
	RELEASE_REQUEST
} dlms_status_tag_t;

static struct {
	uint16_t dst_address;
	uint8_t serial_number[16];
	uint8_t len_serial_number;
	uint8_t mac[6];
} sx_list_nodes[DLMS_EMU_MAX_NUM_NODES_CONNECTED];

/* store the maximum index in the list of connected nodes */
static uint16_t sus_max_index_connected_node;
static uint32_t sul_total_cycle_number;
static uint16_t sus_current_node;
static dlms_status_tag_t suc_current_node_dlms_step;
static uint8_t suc_blocknumber;
static dl_432_buffer_t sx_cmd_tx_432;

static int si_dlms_emu_app_id;

static timer_t st_timeout_timer;
static timer_t st_next_cycle_timer;
static timer_t st_next_node_timer;
static timer_t st_next_step_timer;

static void _cycle_next_node(void);
static void _next_step(void);
static void _start_cycle(void);
static void _timeout(void);
static uint16_t _get_next_node_to_cycle(uint16_t us_index);
static uint16_t _get_number_nodes_connected(void);

static void _st_machine(uint16_t us_index, dlms_status_tag_t dlms_status, cmd_cycles_t cmd);

static dlms_report_info_t sx_node_report_info;
static int si_dlms_emu_link_fd;
static int si_dlms_emu_data_fd;
static unsigned char suc_dlms_emu_buf[MAX_DLMSEMU_SOCKET_SIZE];


/* standard low security level association request */
uint8_t cmd_DLMS_associationRequest[] = {
	0x60, 0x36, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0x8A, 0x02, 0x07, 0x80, 0x8B, 0x07, 0x60, 0x85,
	0x74, 0x05, 0x08, 0x02, 0x01, 0xAC, 0x0A, 0x80, 0x08, 0x47, 0x45, 0x45, 0x30, 0x30, 0x31, 0x35, 0x39, 0xBE, 0x10, 0x04, 0x0E,
	0x01, 0x00, 0x00, 0x00, 0x06, 0x5F, 0x1F, 0x04, 0x00, 0x00, 0x10, 0x1C, 0x00, 0xFD, 0x60, 0x36, 0xA1, 0x09, 0x06, 0x07, 0x60,
	0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0x8A, 0x02, 0x07, 0x80, 0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x01, 0xAC, 0x0A,
	0x80, 0x08, 0x47, 0x45, 0x45, 0x30, 0x30, 0x31, 0x35, 0x39, 0xBE, 0x10, 0x04, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5F, 0x1F,
	0x04, 0x00, 0x00, 0x10, 0x1C, 0x00, 0xFD
};

/* request date obis0000010000FF class 0008 attr 02 */
uint8_t cmd_DLMS_date_OBIS0000010000FF_0008_02[] = {0xC0, 0x01, 0xC1, 0x00, 0x08, 0x00, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x02, 0x00};

/* S02 number of days to query */
/* request S02 profile */
uint8_t cmd_DLMS_S02_OBIS0100630100FF_0007_02[] = {
	0xC0, 0X01, 0XC2, 0X00, 0X07, 0X01, 0X00, 0X63, 0X01, 0X00, 0XFF, 0X02, 0X01, 0X01, 0X02, 0X04, 0X02, 0X04, 0X12, 0X00, 0X08,
	0X09, 0X06, 0X00, 0X00, 0X01, 0X00, 0X00, 0XFF, 0X0F, 0X02, 0X12, 0X00, 0X00, 0X09, 0X0C, 0X07, 0XDE, 0X05, 0X15, 0X01, 0X08,
	0X00, 0X00, 0X00, 0X80, 0X00, 0X80, 0X09, 0X0C, 0X07, 0XDE, 0X05, (0x15 + S02_NUMBEROFDAYS), 0X02, 0X08, 0X00, 0X00, 0X00,
	0X80, 0X00, 0X80, 0X01, 0X00
};

/*next block request */
uint8_t cmd_DLMS_NextBlock[] = {0xC0, 0x02, 0xC2, 0x00, 0x00, 0x00, 0x02};

/* release request */
uint8_t cmd_DLMS_release_request[] = {0x62, 0x00};

/* DLMS  ANSWER VALIDATION */
const uint8_t assoc_response_val[] = {0x61, 0x29, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0xA2, 0x03, 0x02};
const uint8_t release_response_val[] = {0x63, 0x03, 0x80, 0x01, 0x00};
const uint8_t get_request_s02val[] = {0xC4, 0x02, 0xC2};

const uint8_t get_request_date_val[] = {0xC4, 0x01, 0xC1, 0x00, 0x09};

/* Result to Report */
static char spc_rep_result_fail_timeout[] = "DLMS ERROR TIMEOUT";
static char spc_rep_result_fail_invalid[] = "DLMS ERROR INVALID";
static char spc_rep_result_fail[] = "DLMS ERROR";
static char spc_rep_result_success[] = "DLMS SUCCESS";
/* Command to Report */
static char pc_rep_cmd_get[] = "GET";
//static char pc_rep_cmd_set[] = "SET";

static void _timer_handler(int sig, siginfo_t *si, void *puc_data)
{
    timer_t *tidp;
    tidp = si->si_value.sival_ptr;

    if (*tidp == st_timeout_timer) {
    	PRINTF("DLMS EMU: _timer_handler _timeout\n");
        _timeout();
    } else if (*tidp == st_next_cycle_timer) {
    	PRINTF("DLMS EMU: _timer_handler _start_cycle\n");
        _start_cycle();
    } else if (*tidp == st_next_node_timer) {
    	PRINTF("DLMS EMU: _timer_handler _cycle_next_node\n");
        _cycle_next_node();
    } else if (*tidp == st_next_step_timer) {
    	PRINTF("DLMS EMU: _timer_handler _next_step\n");
        _next_step();
    } else {
    	PRINTF("DLMS EMU: _timer_handler UNKNOWN\n");
    }

}

static int _create_local_timer(char *name, timer_t *timer_id)
{
    struct sigevent         te;
    struct sigaction        sa;
    int                     sigNo = SIGRTMIN;
    int res;

    /* Set up signal handler. */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = _timer_handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(sigNo, &sa, NULL) == -1) {
        fprintf(stderr, "Failed to setup signal handling for %s.\n", name);
        return(-1);
    }

    /* Set and enable alarm */
    te.sigev_notify = SIGEV_SIGNAL;
    te.sigev_signo = sigNo;
    te.sigev_value.sival_ptr = timer_id;
    res = timer_create(CLOCK_REALTIME, &te, timer_id);
    if (res == -1) {
    	printf("_error_timer_create: %s\r\n", name);
		return(-1);
	}

    return(0);
}


static int _start_timer_sec(char *name, timer_t *timer_id, int expire_sec, int interval_sec)
{
    struct itimerspec       its;
    int res;

    its.it_interval.tv_sec = interval_sec;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = expire_sec;
    its.it_value.tv_nsec = 0;
    res = timer_settime(*timer_id, 0, &its, NULL);
    if (res == -1) {
    	printf("_error_timer_settime: %s\r\n", name);
		return(-1);
	}

    return(0);
}

static int _start_timer_ms(char *name, timer_t *timer_id, int expire_ms, int interval_ms)
{
    struct itimerspec       its;
    int res;

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = interval_ms * 1000000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = expire_ms * 1000000;
    res = timer_settime(*timer_id, 0, &its, NULL);
    if (res == -1) {
    	printf("_error_timer_settime: %s\r\n", name);
		return(-1);
	}

    return(0);
}

static int _stop_timer(char *name, timer_t *timer_id)
{
	struct itimerspec its;
    int res;

	if (*timer_id) {
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 0;
		res = timer_settime(*timer_id, 0, &its, NULL);
		if (res == -1) {
			printf("_error_stop_timer: %s\r\n", name);
			return(-1);
		}
	}

    return (0);
}

/*
 * \brief generate dlms message to send
 *
 * \param uc_step dlms message step to generate
 * \param uc_invoke_order number of sequence of the dlms message
 *
 */
static uint16_t _generate_str(dlms_status_tag_t uc_step, uint8_t uc_invoke_order)
{
	uint16_t ui_lenghtmsg;
	uint8_t *puc_dlms_message;

	/* assign pointer to message */
	switch (uc_step) {
	case ASSOCIATION_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_associationRequest);
		puc_dlms_message = cmd_DLMS_associationRequest;
		PRINTF("DLMS EMU: Send ASSOCIATION_REQUEST\n");
		break;

	case DATE_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_date_OBIS0000010000FF_0008_02);
		puc_dlms_message = cmd_DLMS_date_OBIS0000010000FF_0008_02;
		cmd_DLMS_date_OBIS0000010000FF_0008_02[2] = uc_invoke_order;
		PRINTF("DLMS EMU: Send DATE_REQUEST\n");
		break;

	case SO2_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_S02_OBIS0100630100FF_0007_02);
		puc_dlms_message = cmd_DLMS_S02_OBIS0100630100FF_0007_02;
		cmd_DLMS_S02_OBIS0100630100FF_0007_02[2] = uc_invoke_order + 1;
		suc_blocknumber = 1;
		PRINTF("DLMS EMU: Send SO2_REQUEST\n");
		break;

	case NEXTBLOCK_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_NextBlock);
		puc_dlms_message = cmd_DLMS_NextBlock;
		cmd_DLMS_NextBlock[2] = uc_invoke_order + 1;
		suc_blocknumber++;
		cmd_DLMS_NextBlock[6] = suc_blocknumber;
		PRINTF("DLMS EMU: Send NEXTBLOCK_REQUEST\n");
		break;

	case RELEASE_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_release_request);
		puc_dlms_message = cmd_DLMS_release_request;
		PRINTF("DLMS EMU: Send RELEASE_REQUEST\n");
		break;
	}

	/* copy msg to transmission buffer */
	memcpy(sx_cmd_tx_432.dl.buff, puc_dlms_message, ui_lenghtmsg);
	return ui_lenghtmsg;
}

/*
 * \brief timeout action when the dlms task step fail
 *
 *
 */
static void _timeout(void)
{
	/* clear timer pointer */
	_st_machine(0, ASSOCIATION_REQUEST, CMD_TIMEOUT);
}

/*
 * \brief Start dlms cycles form node 0
 *
 */
static void _start_cycle(void)
{
	/* Look for the first node with 432 connection open */
	if (_get_number_nodes_connected() != 0) {
		sus_current_node = _get_next_node_to_cycle(0);
		if (sus_current_node < DLMS_EMU_MAX_NUM_NODES_CONNECTED) {
			sul_total_cycle_number++;
			dlms_emu_report_start_cycle();
			suc_current_node_dlms_step = ASSOCIATION_REQUEST;

			_st_machine(sus_current_node, suc_current_node_dlms_step, CMD_SEND);
		}
	}
}

/*
 * \brief Start dlms cycles form node 0
 *
 */
static void _stop_cycles(void)
{
	/* Cancel all timers */
	_stop_timer("timeout", &st_timeout_timer);
	_stop_timer("next cycle", &st_next_cycle_timer);
	_stop_timer("next node", &st_next_node_timer);
	_stop_timer("next step", &st_next_step_timer);

	sul_total_cycle_number = 0;
	suc_current_node_dlms_step = ASSOCIATION_REQUEST;

	//printf("_stop_cycles\r\n");
}

/*
 * \brief get next node to cycle if all nodes have been cycles wait before start from with the first node
 *
 */
static void _cycle_next_node(void)
{
	/* go to next node */
	sus_current_node = _get_next_node_to_cycle(sus_current_node);

	if (sus_current_node < DLMS_EMU_MAX_NUM_NODES_CONNECTED) {
		suc_current_node_dlms_step = ASSOCIATION_REQUEST;
		_st_machine(sus_current_node, suc_current_node_dlms_step, CMD_SEND);
	} else {
		/* end of cycles..  start next cycle from beginning */
		dlms_emu_report_end_cycle();
		_start_timer_sec("next cycle", &st_next_cycle_timer, DLMS_EMU_TIME_BETWEEEN_CYCLES, 0);
	}
}

/*
 * \brief execute next step of the dlms cycles application
 *
 * \return
 *
 */
static void _next_step(void)
{
	sx_node_report_info.i_element_id = suc_current_node_dlms_step;
	sprintf(&sx_node_report_info.pc_dev_sn[0],"%s", sx_list_nodes[sus_current_node].serial_number);
	sprintf(&sx_node_report_info.pc_result[0],"%s", spc_rep_result_success);
	sprintf(&sx_node_report_info.pc_operation[0],"%s", pc_rep_cmd_get);
	dlms_emu_report_node_cycle(&sx_node_report_info);

	switch (suc_current_node_dlms_step) {
	case ASSOCIATION_REQUEST:
		suc_current_node_dlms_step = DATE_REQUEST;
		suc_blocknumber = 0;
		_st_machine(sus_current_node, suc_current_node_dlms_step, CMD_SEND);
		break;

	case DATE_REQUEST:
		suc_current_node_dlms_step = SO2_REQUEST;
		_st_machine(sus_current_node, suc_current_node_dlms_step, CMD_SEND);
		break;

	case SO2_REQUEST:
		suc_current_node_dlms_step = NEXTBLOCK_REQUEST;
		suc_blocknumber = 0;
		_st_machine(sus_current_node, suc_current_node_dlms_step, CMD_SEND);
		break;

	case NEXTBLOCK_REQUEST:
		suc_blocknumber++;
		if (suc_blocknumber > DLMS_SO2_NUMBER_OF_BLOCKS) {
			suc_current_node_dlms_step = RELEASE_REQUEST;
			suc_blocknumber = 0;
		}

		_st_machine(sus_current_node, suc_current_node_dlms_step, CMD_SEND);
		break;

	case RELEASE_REQUEST:
	default:
		/* go to next node */
		sus_current_node = _get_next_node_to_cycle(sus_current_node);
		if (sus_current_node < DLMS_EMU_MAX_NUM_NODES_CONNECTED) {
			_start_timer_sec("next node", &st_next_node_timer, DLMS_EMU_TIME_BETWEEEN_CYCLES, 0);
		} else {
			/* end of cycles..  start next cycle from beginning */
			dlms_emu_report_end_cycle();
			_start_timer_sec("next cycle", &st_next_cycle_timer, DLMS_EMU_TIME_BETWEEEN_CYCLES, 0);
		}
		break;
	}
}

/*
 * \brief state machine of dlms cycles application.
 *
 * \param us_index node index to cycle
 * \param dlms_status dlms cycle step to execute
 * \param cmd status machines command to execute
 *
 */
static void _st_machine(uint16_t us_index, dlms_status_tag_t dlms_status, cmd_cycles_t cmd)
{
	uint16_t uc_length_msg;

	switch (cmd) {
	case CMD_SEND:
		if (sx_list_nodes[us_index].dst_address != CL_432_INVALID_ADDRESS) {
			uc_length_msg = _generate_str(dlms_status, 0xc1);
			ifacePrime_select_api(si_dlms_emu_app_id);
			prime_cl_432_dl_data_request(0, 0, us_index, &sx_cmd_tx_432, uc_length_msg, 0);
		} else {
			_cycle_next_node();
		}

		break;

	case CMD_CONFIRM:
		break;

	case CMD_INDICATION:
		if (dlms_status == RELEASE_REQUEST) {
			_start_timer_ms("next_cycle", &st_next_cycle_timer, DLMS_EMU_TIME_BETWEEEN_CYCLES, 0);
		} else {
			_start_timer_ms("next_step", &st_next_step_timer, DLMS_EMU_TIME_WAIT_BETWEEN_MESSAGES, 0);
		}break;

	case CMD_TIMEOUT:
		sx_node_report_info.i_element_id = suc_current_node_dlms_step;
		sprintf(&sx_node_report_info.pc_dev_sn[0],"%s", sx_list_nodes[sus_current_node].serial_number);
		sprintf(&sx_node_report_info.pc_result[0],"%s", spc_rep_result_fail_timeout);
		sprintf(&sx_node_report_info.pc_operation[0],"%s", pc_rep_cmd_get);

		dlms_emu_report_node_cycle(&sx_node_report_info); // DLMS_CYCLE_FAIL);
		_cycle_next_node();
		break;
	}
}

/*
 * \brief read the list of dlms nodes and return the value;
 *
 * \return number of nodes connected to 432 CL layer
 */
static uint16_t _get_number_nodes_connected(void)
{
	uint16_t us_next_index = 0;
	uint16_t us_num_nodes = 0;

	do {
		if (sx_list_nodes[us_next_index].dst_address != CL_432_INVALID_ADDRESS) {
			us_num_nodes++;
			sus_max_index_connected_node = us_next_index + 1;
		}

		us_next_index++;
	} while (us_next_index < DLMS_EMU_MAX_NUM_NODES_CONNECTED);

	return us_num_nodes;
}

/*
 * \brief read in the list of nodes, next node to cycle
 *
 * \return nest index in the list of nodes
 *
 */
static uint16_t _get_next_node_to_cycle(uint16_t us_index)
{
	uint16_t us_next_index;

	for (us_next_index = (us_index + 1); us_next_index < sus_max_index_connected_node; us_next_index++) {
		if (sx_list_nodes[us_next_index].dst_address != CL_432_INVALID_ADDRESS) {
			return us_next_index;
		}
	}

	return DLMS_EMU_MAX_NUM_NODES_CONNECTED;
}

void _dlms_mngp_rsp_cb(uint8_t* ptrMsg, uint16_t len)
{
    uint16_t us_pib_attrib;
    uint8_t uc_index;
    uint8_t *puc_pib_value;
    uint16_t us_pib_size;
    uint8_t uc_next;
    uint8_t *ptr;
    uint8_t uc_enhanced_pib;
    uint8_t uc_records=0;
    uint8_t uc_record_len=0;
    uint16_t us_last_iterator=0;

    (void)(uc_enhanced_pib);
    (void)(us_pib_size);
    (void)(uc_next);
    (void)(uc_index);

    ptr = ptrMsg + 2;
    /*Pib Attribute (2 bytes) | Index (1 byte) | Pib Value (var) | Next (1 byte) */
    if (len<4) {
    	// Error!!! Message too short
    	return;
    }

    if (*ptr != MNGP_PRIME_LISTRSP) {
    	us_pib_attrib   = ((uint16_t)(*ptr++)) << 8;
    	us_pib_attrib  += (uint16_t)(*ptr++);
    	/* Standard response for a PIB*/
//    	if (us_pib_attrib == PIB_MAC_EUI_48) {
//			puc_pib_value = ptr + 1;
//			memcpy(puc_mac_address, puc_pib_value, 6);
//		}
    } else {
    	int16_t i = 0;
        //Enhanced PIB
        uc_enhanced_pib = *ptr++;
        us_pib_attrib   = ((uint16_t)(*ptr++)) << 8;
        us_pib_attrib  += (uint16_t)(*ptr++);

        if (us_pib_attrib == PIB_ATML_432_NODE_LIST) {
        	uc_records = *ptr++;
        	uc_record_len = *ptr++;
        	us_pib_size = len - 5;
			/* Load data */
			us_last_iterator = 0xFFFF;
			puc_pib_value = ptr;

			for ( i= 0; i < uc_records; i++) {
				uint16_t us_address = 0;
				us_last_iterator = ((puc_pib_value[0] << 8) + puc_pib_value[1]);
				/* Get one record... */
				us_address = ((puc_pib_value[2] << 8) + puc_pib_value[3]);

				if (us_address >= (0x0FFF - 1)) {
					/* Broadcast connection, ignore */
					puc_pib_value += uc_record_len+2;
					continue;
				}

				/* update max index */
				if (us_address >= sus_max_index_connected_node) {
					sus_max_index_connected_node = us_address + 1;
				}

				sx_list_nodes[us_address].dst_address = us_address;
				memcpy(sx_list_nodes[us_address].serial_number, puc_pib_value + 4, DLMS_EMU_SN432_MAC_SIZE);
				sx_list_nodes[us_address].len_serial_number = *(puc_pib_value + 4 + DLMS_EMU_SN432_MAC_SIZE);
				*(sx_list_nodes[us_address].serial_number + sx_list_nodes[us_address].len_serial_number) = 0; /* End of line */
				memcpy(sx_list_nodes[us_address].mac,  puc_pib_value + 4 + DLMS_EMU_SN432_MAC_SIZE + 1, 6);

				/* advance buffer pointer to record length plus iterator length */
				puc_pib_value += uc_record_len + 2;
			}

			if ((uc_records == 0) || (uc_records < MNGP_PRIME_ENHANCED_LIST_MAX_RECORDS)) {
				/* End query */
				PRINTF("DLMS EMU: Updated Connected Nodes Table\n");
				_start_cycle();
			} else {
				uint16_t us_bigendian_iterator = 0;

				if (us_last_iterator == 0xFFFF) {
					return;
				}

				if ((us_last_iterator & 0x8000) != 0) {
					return;
				}

				//Ask for the next iterator..
				us_last_iterator++;

				//reverse interator to allow a direct "memcpy"
				us_bigendian_iterator  = (us_last_iterator & 0xFF ) << 8;
				us_bigendian_iterator += (us_last_iterator >> 8 ) & 0xFF;

				/* Request next elements on the list */
				mngLay_NewMsg(MNGP_PRIME_EN_PIBQRY);
				mngLay_AddGetPibListEnQuery(PIB_ATML_432_NODE_LIST, MNGP_PRIME_ENHANCED_LIST_MAX_RECORDS,
											(uint8_t*) &us_bigendian_iterator);
				mngLay_SendMsg(si_dlms_emu_app_id);
			}
        }
    }
}

/*
 * \brief Update list of connected nodes before to start cycles
 *
 */
static void _update_nodes_list(void)
{
	uint16_t us_index;

	/* PRIME Mng : Use Enhanced to query 8250 list (432 connected nodes) */
	mngLay_NewMsg(MNGP_PRIME_EN_PIBQRY);
	us_index = 0; /* query first element */
	mngLay_AddGetPibListEnQuery(PIB_ATML_432_NODE_LIST, MNGP_PRIME_ENHANCED_LIST_MAX_RECORDS, (uint8_t*)&us_index);
	mngLay_SendMsg(si_dlms_emu_app_id);
}

/*
 * \brief validate the contend of the message received
 *
 * \param uc_step dlms step message to validate
 * \param *puc_data pointer to the dlms messsage
 *
 * \return valid or invalid message
 */
static uint8_t _check_dlms_message(dlms_status_tag_t uc_step, uint8_t *puc_data)
{
	(void)(*puc_data);

	uint8_t uc_result;
	uc_result = DLMS_VALID_MESSAGE;

	switch (uc_step) {
	case ASSOCIATION_REQUEST:
		uc_result = DLMS_VALID_MESSAGE;
		if (memcmp((void *)puc_data, (void *)assoc_response_val, (int)16) == 0) {
			uc_result = DLMS_VALID_MESSAGE;
		} else {
			/* invalid response received */
			uc_result = DLMS_INVALID_MESSAGE;
		}

		break;

	case DATE_REQUEST:
		if (memcmp((void *)puc_data, (void *)get_request_date_val, (int)5) == 0) {
			uc_result = DLMS_VALID_MESSAGE;
		} else {
			/* invalid response received */
			uc_result = DLMS_INVALID_MESSAGE;
		}

		break;

	case SO2_REQUEST:
		if (memcmp((void *)puc_data, (void *)get_request_s02val, (int)2) == 0) {
			uc_result = DLMS_VALID_MESSAGE;
		} else {
			/* invalid response received */
			uc_result = DLMS_INVALID_MESSAGE;
		}

		break;

	case NEXTBLOCK_REQUEST:
		if (memcmp((void *)puc_data, (void *)get_request_s02val, (int)2) == 0) {
			uc_result = DLMS_VALID_MESSAGE;
		} else {
			/* invalid response received */
			uc_result = DLMS_INVALID_MESSAGE;
		}

		break;

	case RELEASE_REQUEST:
		if (memcmp((void *)puc_data, (void *)release_response_val, (int)5) == 0) {
			uc_result = DLMS_VALID_MESSAGE;

		} else {
			/* invalid response received */
			uc_result = DLMS_INVALID_MESSAGE;
		}

		break;
	}

	if (uc_result == DLMS_INVALID_MESSAGE) {
		PRINTF("DLMS EMU: RSP ERROR (%u)\n", uc_step);
	} else {
		PRINTF("DLMS EMU: RSP OK (%u)\n", uc_step);
	}

	return uc_result;
}

/*
 * \brief cl 432 layer indication call back function
 *
 * \param uc_dst_lsap destination LSAP address
 * \param uc_src_lsap sourCe LSAP address
 * \param us_dst_address CL432 layes destination address
 * \param src_address CL432 source address
 * \param *puc_data pointer to buffer data
 *
 */
static void _dlms_emu_cl_432_dl_data_ind_cb(uint8_t uc_dst_lsap, uint8_t uc_src_lsap, uint16_t us_dst_address, uint16_t src_address, uint8_t *puc_data,
uint16_t uc_lsdu_len, uint8_t uc_link_class)
{
	(void)(uc_dst_lsap);
	(void)(uc_src_lsap);
	(void)(uc_lsdu_len);
	(void)(uc_link_class);
	(void)(us_dst_address);
	(void)(src_address);

	_stop_timer("timeout", &st_timeout_timer);

	if (_check_dlms_message(suc_current_node_dlms_step, puc_data) == DLMS_VALID_MESSAGE) {
		_st_machine(sus_current_node, suc_current_node_dlms_step, CMD_INDICATION);
	} else {
		/* invalid message */
		sx_node_report_info.i_element_id = suc_current_node_dlms_step;
		sprintf(&sx_node_report_info.pc_dev_sn[0],"%s", sx_list_nodes[sus_current_node].serial_number);
		sprintf(&sx_node_report_info.pc_result[0],"%s", spc_rep_result_fail_invalid);
		sprintf(&sx_node_report_info.pc_operation[0],"%s", pc_rep_cmd_get);

		dlms_emu_report_node_cycle(&sx_node_report_info);

		_start_timer_sec("next node", &st_next_node_timer, DLMS_EMU_TIME_BETWEEEN_CYCLES, 0);
	}
}

/*
 * \brief cl 432 layer confirm call back function
 *
 * \param uc_dst_lsap destination LSAP address
 * \param uc_src_lsap sourCe LSAP address
 * \param us_dst_address CL432 layes destination address
 * \param src_address CL432 source address
 * \param *puc_data pointer to buffer data
 *
 */
static void _dlms_emu_cl_432_dl_data_cfm_cb(uint8_t uc_dst_lsap, uint8_t uc_src_lsap, uint16_t us_dst_address, uint8_t uc_tx_status)
{
	(void)(uc_dst_lsap);
	(void)(uc_src_lsap);
	(void)(us_dst_address);

	switch (uc_tx_status) {
	case CL_432_TX_STATUS_SUCCESS:
		PRINTF("DLMS EMU: TX CFM OK\n");
		/* data confirm success */
		_start_timer_sec("timeout", &st_timeout_timer, DLMS_EMU_TIME_WAIT_RESPONSE, 0);
		break;

	case CL_432_TX_STATUS_PREVIOUS_COMM:
	case CL_432_TX_STATUS_TIMEOUT:
	case CL_432_TX_STATUS_ERROR_BAD_ADDRESS:
	case CL_432_TX_STATUS_ERROR_BAD_HANLDER:
		PRINTF("DLMS EMU: TX CFM ERROR [%u]\n", uc_tx_status);
		/* data confirm timeout or handler error */
		sx_node_report_info.i_element_id = suc_current_node_dlms_step;
		sprintf(&sx_node_report_info.pc_dev_sn[0],"%s", sx_list_nodes[sus_current_node].serial_number);
		sprintf(&sx_node_report_info.pc_result[0],"%s", spc_rep_result_fail);
		sprintf(&sx_node_report_info.pc_operation[0],"%s", pc_rep_cmd_get);

		dlms_emu_report_node_cycle(&sx_node_report_info); // DLMS_CYCLE_FAIL);
		_start_timer_sec("next node", &st_next_node_timer, DLMS_EMU_TIME_BETWEEEN_CYCLES, 0);
		break;

	default:
		/* unexpected return value from 432_layer */
		break;
	}
}

/*
 * \brief Join indication callback
 *
 * \param puc_device_id destination address of the leave indication node
 * \param uc_device_id_len
 * \param us_dst_address
 * \param puc_mac  mac address of the join node
 */

static void _dlms_emul_cl_432_join_ind_cb(uint8_t *puc_device_id, uint8_t uc_device_id_len, uint16_t us_dst_address, uint8_t *puc_mac)
{
	uint8_t uc_length;

	/* check max index */
	if (us_dst_address >= sus_max_index_connected_node) {
		sus_max_index_connected_node = us_dst_address + 1;
	}

	sx_list_nodes[us_dst_address].dst_address = us_dst_address;
	uc_length = (uc_device_id_len > 16) ? 16 : uc_device_id_len;
	memcpy(sx_list_nodes[us_dst_address].serial_number, puc_device_id, uc_length);
	/* Insert End of String */
	*(sx_list_nodes[us_dst_address].serial_number + uc_length) = 0;
	sx_list_nodes[us_dst_address].len_serial_number = uc_length;
	memcpy(sx_list_nodes[us_dst_address].mac, puc_mac, 6);
}

/*
 * \brief leave indication callback for 432 layer
 *
 * \param us_dst_address destination address of the leave indication node
 *
 */
static void _dlms_emul_cl_432_leave_ind_cb(uint16_t us_dst_address)
{
	sx_list_nodes[us_dst_address].dst_address = CL_432_INVALID_ADDRESS;
}

/**
 * \brief Process messages received from CLI.
 * Unpack CLI protocol command
 */
static void _dlms_emu_rcv_cmd(uint8_t* buf, uint16_t buflen)
{
    uint8_t *puc_buf;

    puc_buf = buf;

    switch (*puc_buf++) {
		case DLMS_EMU_CMD_START_CYCLES:
		{
			PRINTF("DLMS EMU: start cycles\n");
			_update_nodes_list();
		}
    	break;

    case DLMS_EMU_CMD_STOP_CYCLES:
		{
			PRINTF("DLMS EMU: stop cycles\n");
			_stop_cycles();
		}
    	break;

    }
}

/*
 * \brief dlms cycles initialization function.
 *
 *
 */
void dlms_emu_init(int _app_id)
{
	prime_cl_432_callbacks_t cl432_callbacks;
	uint16_t us_index;

	si_dlms_emu_app_id = _app_id;

	memset(&cl432_callbacks, 0, sizeof(prime_cl_432_callbacks_t));
	cl432_callbacks.prime_cl_432_dl_data_cfm_cb = _dlms_emu_cl_432_dl_data_cfm_cb;
	cl432_callbacks.prime_cl_432_dl_data_ind_cb = _dlms_emu_cl_432_dl_data_ind_cb;
	cl432_callbacks.prime_cl_432_dl_join_ind_cb = _dlms_emul_cl_432_join_ind_cb;
	cl432_callbacks.prime_cl_432_dl_leave_ind_cb = _dlms_emul_cl_432_leave_ind_cb;

	prime_cl_432_set_callbacks(_app_id, &cl432_callbacks);

	for (us_index = 0; us_index < DLMS_EMU_MAX_NUM_NODES_CONNECTED; us_index++) {
		sx_list_nodes[us_index].dst_address = CL_432_INVALID_ADDRESS;
	}

	/* max index in connected nodes index */
	sus_max_index_connected_node = 1;

	/* Set MNGP callback response */
	mngLay_SetRspCallback(_app_id, _dlms_mngp_rsp_cb);

	dlms_emu_report_init();

	/* Create local timers */
	_create_local_timer("timeout", &st_timeout_timer);
	_create_local_timer("next cycles", &st_next_cycle_timer);
	_create_local_timer("next node", &st_next_node_timer);
	_create_local_timer("next step", &st_next_step_timer);
}


void dlms_emu_callback(socket_ev_info_t *_ev_info)
{
	if (_ev_info->i_socket_fd >= 0) {
		if (_ev_info->i_event_type == SOCKET_EV_LINK_TYPE) {
			/* Manage LINK */
			si_dlms_emu_link_fd = _ev_info->i_socket_fd;
			socket_accept_conn(_ev_info);
		} else if (_ev_info->i_event_type == SOCKET_EV_DATA_TYPE) {
			/* Receive DATA */
			ssize_t i_bytes;
			si_dlms_emu_data_fd = _ev_info->i_socket_fd;
			/* Read data from Socket */
			i_bytes = read(_ev_info->i_socket_fd, suc_dlms_emu_buf, MAX_DLMSEMU_SOCKET_SIZE);
			if (i_bytes > 0) {
				_dlms_emu_rcv_cmd(suc_dlms_emu_buf, i_bytes);
			} else {
				socket_check_connection(_ev_info->i_app_id, _ev_info->i_socket_fd);
			}
		}
	}
}
