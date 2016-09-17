/**
 * \file
 *
 * \brief Cycles application for G3 Coordinator
 *
 * Copyright (c) 2016 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <sys/time.h>
#include <time.h>

#include "AdpApi.h"
#include "drivers/g3/network_adapter_g3.h"
#include "dlms_emu_coord.h"
#include "app_adp_mng.h"
#include "Config.h"
#include "async_ping.h"
#include "dlms_emu_report.h"

#define UNUSED(v)                  (void)(v)
#define ADP_PATH_METRIC_TYPE       0

#define DLMS_REPORT_CONSOLE
#define DLMS_DEBUG_CONSOLE

#ifdef DLMS_DEBUG_CONSOLE
#	define LOG_APP_DEBUG(a)   printf a
#else
#	define LOG_APP_DEBUG(a)   (void)0
#endif


#ifdef DLMS_REPORT_CONSOLE
#	define LOG_APP_REPORT(a)   printf a
#else
#	define LOG_APP_REPORT(a)   (void)0
#endif

typedef struct x_cycles_stat {
	uint32_t ul_success;
	uint32_t ul_errors;
	uint32_t ul_cmean_time;
} x_cycles_stat_t;

enum {
	ASSOCIATION_REQUEST,
	DATE_REQUEST,
	SO2_REQUEST,
	NEXBLOCK_REQUEST,
	RELEASE_REQUEST
} dlms_status_tag;

enum {
	ERROR_NO_ERROR = 0,
	ERROR_TIMEOUT_DATA_CONFIRM = 1,
	ERROR_BAD_TX = 2,
	ERROR_TIMEOUT_DATA_INDICATION = 3,
	ERROR_NON_CONNECTED_NODE = 4,
	ERROR_INTEGRITY = 5
} st_error_codes_dlms;

/* Invalid 432 address */
#define INVALID_432_ADDRESS              0xffff

enum {
	SC_WAIT_INITIAL_LIST,
	SC_PATH_REQUEST_LIST,
	SC_TIME_NEXT_CYCLE,
	SC_CYCLES,
} st_sort_cycles;

enum {
	RN_INITIAL_STATE,
	RN_WAIT_ANSWER_NODES_REGISTERED
} st_registered_nodes;

enum {
	SEND,
	WAIT_DATA_CONFIRM,
	WAIT_DATA_INDICATION
} st_execute_cycle;

enum {
	EMULATION_RUN,
	EMULATION_STOPPED,
	EMULATION_ERASE_STATISTICAL,
	EMULATION_INIT
} pin_app_dlms_emulation;

/* Configure Number of steps and lengths */
#define NUM_STEPS	5

/* Configure Application Behaviour */
static bool sb_enable_wait_reg_nodes;
static bool sb_enable_dlms_cycles;
static bool sb_enable_ping_cycles;
static bool sb_enable_preq_cycles;

static uint32_t sul_ping_cycle_time;
static uint32_t sul_ping_cycle_timeout;
static uint32_t sul_ping_cycle_ttl;
static uint32_t sul_ping_cycle_len;

static uint32_t sul_dlms_numdays_cycles;
static uint32_t sul_dlms_time_between_cycles;
static uint32_t sul_dlms_time_out_data_cycles;
static uint32_t sul_dlms_time_between_msg;
static uint32_t sul_dlms_time_waiting_start;
static uint32_t sul_dlms_time_wait_preq_cfm;
static uint32_t sul_dlms_time_between_preq_no_dlms;

/* standard low security level association request */
uint8_t cmd_DLMS_associationRequest[] =
{
	0x60, 0x36, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85,
	0x74, 0x05, 0x08, 0x01, 0x01, 0x8A, 0x02, 0x07,
	0x80, 0x8B, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08,
	0x02, 0x01, 0xAC, 0x0A, 0x80, 0x08, 0x41, 0x54,
	0x4D, 0x30, 0x30, 0x31, 0x35, 0x39, 0xBE, 0x10,
	0x04, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x06, 0x5F,
	0x1F, 0x04, 0x00, 0x00, 0x10, 0x1C, 0x00, 0xFD
};
/* request date obis0000010000FF class 0008 attr 02 */
uint8_t cmd_DLMS_date_OBIS0000010000FF_0008_02[] =
{
	0xC0, 0x01, 0xC1, 0x00, 0x08, 0x00, 0x00, 0x01,
	0x00, 0x00, 0xFF, 0x02, 0x00
};
/* S02 number of days to query */
/* request S02 profile */
uint8_t cmd_DLMS_S02_OBIS0100630100FF_0007_02[] =
{
	0xC0, 0X01,
	0XC2, 0X00,
	0X07,
	0X01, 0X00, 0X63, 0X01, 0X00, 0XFF,
	0X02,
	0X01, 0X01, 0X02, 0X04, 0X02, 0X04, 0X12, 0X00, 0X08, 0X09, 0X06,
	0X00, 0X00, 0X01, 0X00, 0X00, 0XFF,
	0X0F, 0X02, 0X12, 0X00, 0X00, 0X09, 0X0C,
	0X07, 0XDE, 0X05, 0X15, 0X01, 0X08, 0X00, 0X00, 0X00, 0X80, 0X00, 0X80, 0X09, 0X0C,
	0X07, 0XDE, 0X05, 0X15, 0X02, 0X08, 0X00, 0X00, 0X00, 0X80, 0X00, 0X80, 0X01, 0X00
};
/*next block request */
uint8_t cmd_DLMS_NextBlock[] = {0xC0, 0x02, 0xC2, 0x00, 0x00, 0x00, 0x02};
/* release request */
uint8_t cmd_DLMS_release_request[] = {0x62, 0x00};
/* DLMS  ANSWER VALIDATION */
const uint8_t assoc_response_val[] = {0x61, 0x29, 0xA1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01, 0x01, 0xA2, 0x03, 0x02};
const uint8_t release_response_val[] = {0x63, 0x03, 0x80, 0x01, 0x00};
const uint8_t get_request_s02val[] = {0xC4, 0x02};
const uint8_t get_request_date_val[] = {0xC4, 0x01, 0xC1, 0x00, 0x09};

static x_node_list_t spx_node_list[DLMS_MAX_DEV_NUM];
static x_cycles_stat_t sx_cycles_stat[DLMS_MAX_DEV_NUM];
static uint16_t sus_last_position_in_use;
static bool sb_update_nodes;
static uint32_t sul_timer_dlms_start_wait;
static uint32_t sul_cycles_counter;
static uint16_t sus_node_cycling;
static uint8_t suc_step_cycling;
static uint32_t sul_timer_next_cycle;
static uint32_t sul_time_out;

static uint32_t sul_timer_wait_path_cfm;
static uint32_t sul_timer_max_to_req_paths;
static struct TAdpPathDiscoveryConfirm sx_path_nodes[DLMS_MAX_DEV_NUM];


typedef struct _ping_cycle_node_stats {
	uint32_t ul_pings_sent;
	uint32_t ul_pings_successful;
	uint32_t ul_ping_errors;
}s_ping_cycle_node_stats_t;

struct s_ping_cycle_status_t {
	s_ping_cycle_node_stats_t as_ping_cycle_node_stats[DLMS_MAX_DEV_NUM];
	uint32_t ul_current_node_ping;
};

struct s_ping_cycle_status_t s_ping_cycle_status;
static bool sb_ping_sent;


static bool s_new_cmd;
static uint8_t suc_lastblock;
static uint8_t suc_blocknumber;
static uint32_t sul_total_time_cycle;
static uint32_t sul_start_time_cycle;
static uint32_t sul_start_time_node_cycle;

static time_t sul_sys_time;
static struct timeval sx_time_init_value;

/* IPv6 input and output buffers and received length */
uint8_t puc_rx_buff[MAX_LENGTH_IPv6_PDU - IPv6_HEADER_LENGTH - UDP_HEADER_LENGTH];
uint8_t puc_tx_buff[MAX_LENGTH_IPv6_PDU - IPv6_HEADER_LENGTH - UDP_HEADER_LENGTH];

/* Pointers to IPv6 payloads */
uint8_t *pUdpPayload_rx;
uint8_t *pUdpPayload_tx;

/* Socket for the UDP over PLC communication */
Socket *px_udp_plc_socket;

/* Select the first network interface for PLC */
NetInterface *plc_interface = &netInterface[0];

/* Own Extended Address */
static uint8_t spuc_extended_address[8];

/* Netwrok Information */
static x_net_info_t net_Info;

static void _init_ping_cycle_status_data(void){
	uint32_t ul_i;
	s_ping_cycle_status.ul_current_node_ping = 0;
	for(ul_i = 0 ; ul_i < DLMS_MAX_DEV_NUM; ul_i ++){
		s_ping_cycle_status.as_ping_cycle_node_stats[ul_i].ul_pings_sent = 0;
		s_ping_cycle_status.as_ping_cycle_node_stats[ul_i].ul_pings_successful = 0;
		s_ping_cycle_status.as_ping_cycle_node_stats[ul_i].ul_ping_errors = 0;
	}

}

static error_t _send_ping_to_dev(uint32_t ul_dev_idx)
{
	IpAddr srcIpAddr;
	error_t x_error;

	/* Set link-local address, based on the PAN_ID and the short address */
	ipv6StringToAddr(APP_IPV6_GENERIC_LINK_LOCAL_ADDR, &srcIpAddr.ipv6Addr);
	/* Adapt the IPv6 address to the G3 connection data */
	srcIpAddr.ipv6Addr.b[8] = (uint8_t) (DLMS_G3_COORD_PAN_ID >> 8);
	srcIpAddr.ipv6Addr.b[9] = (uint8_t) (DLMS_G3_COORD_PAN_ID);
	srcIpAddr.ipv6Addr.b[14] = (uint8_t) (spx_node_list[ul_dev_idx].us_short_address >> 8);
	srcIpAddr.ipv6Addr.b[15] = (uint8_t) (spx_node_list[ul_dev_idx].us_short_address);
	srcIpAddr.length = sizeof(Ipv6Addr);

	x_error = async_ping_send(plc_interface, &srcIpAddr, sul_ping_cycle_len, sul_ping_cycle_ttl, sul_ping_cycle_timeout * 1000);
	return x_error;

}

static uint32_t _get_time_ms()
{
	struct timeval time_value;
	struct timeval temp_diff;
	uint32_t ul_res;

	gettimeofday(&time_value, NULL);

	temp_diff.tv_sec = time_value.tv_sec - sx_time_init_value.tv_sec;
	temp_diff.tv_usec= time_value.tv_usec - sx_time_init_value.tv_usec;

	/* Using while instead of if below makes the code slightly more robust. */
	while(temp_diff.tv_usec < 0) {
		temp_diff.tv_usec += 1000000;
		temp_diff.tv_sec -= 1;
	}

	ul_res = (uint32_t)((temp_diff.tv_sec * 1000) + (temp_diff.tv_usec / 1000));

	return ul_res;
}


#if defined (DLMS_DEBUG_CONSOLE) || defined (DLMS_REPORT_CONSOLE)

static char c_log_full_cycles_time_query[100];
static char c_log_cycles_step_query[600];
static char c_print_result_test_query[120];
static uint8_t S02Status;

static void _log_S02_Status(uint32_t cycleId, uint16_t us_node)
{
	sprintf(c_log_cycles_step_query, "cycle %u node> 0x%04x; status> %d", cycleId, spx_node_list[us_node].us_short_address, S02Status);
	printf("[DLMS_EMU] %s\r\n", c_log_cycles_step_query);
	S02Status = 0;
}

static void _log_full_cycles_time(uint32_t ul_cycleId, uint32_t ul_cycleTime)
{
	uint16_t us_node_idx;

	sprintf(c_log_full_cycles_time_query, "cycleId: %u cycleTime: %u ul_absolute_time: %u",
		ul_cycleId, ul_cycleTime, _get_time_ms());

	LOG_APP_REPORT(("[DLMS_EMU] Cycle summary: %s\r\n", c_log_full_cycles_time_query));

	/* Summary nodes  */
	us_node_idx = 0;
	while (us_node_idx < net_Info.us_num_nodes) {
		sprintf(c_log_cycles_step_query, "node ID: 0x%04x   Success: %u   Errors: %u   Availability: %u   TimerCycle: %u",
			spx_node_list[us_node_idx].us_short_address, sx_cycles_stat[us_node_idx].ul_success,
			sx_cycles_stat[us_node_idx].ul_errors,
			(((sx_cycles_stat[us_node_idx].ul_success * 100) / (sx_cycles_stat[us_node_idx].ul_success + sx_cycles_stat[us_node_idx].ul_errors) * 100) / 100),
			sx_cycles_stat[us_node_idx].ul_cmean_time);

		us_node_idx++;

		LOG_APP_REPORT(("[DLMS_EMU] %s\r\n", c_log_cycles_step_query));
		LOG_APP_REPORT(("[DLMS_EMU] ------------------------------------------------------------------------------------\r\n"));
	}
}

static void _updatecycle_stat(uint8_t uc_node_idx, uint8_t uc_step, uint8_t uc_error)
{
	if (uc_error == ERROR_NO_ERROR) {
		if (suc_step_cycling == 2 || uc_step == 4) {
			sx_cycles_stat[uc_node_idx].ul_success++;
			S02Status++;
		}
	} else {
		sx_cycles_stat[uc_node_idx].ul_errors++;
	}
}

static void _log_cycles_step(uint32_t ul_cycleId, uint16_t us_node, uint8_t uc_status, uint8_t uc_step)
{
	sprintf(c_log_cycles_step_query, "cycleId: %u, 0x%04x, status: %d, step: %d", ul_cycleId, spx_node_list[us_node].us_short_address, uc_status, uc_step);
	LOG_APP_REPORT(("[DLMS_EMU] %s\r\n", c_log_cycles_step_query));
}

//***********************************************************************************
/**
 *************************************************************************************/
static void _print_result_test(uint8_t uc_node_idx, uint8_t uc_step, uint8_t uc_error, uint32_t num_cycle)
{
	uint32_t cTime;
	cTime = _get_time_ms() - sul_start_time_node_cycle;

	if ((uc_step == NUM_STEPS) && (uc_error == ERROR_NO_ERROR)) {
		sprintf(c_print_result_test_query, "[0x%04x] Time %u \tOK", spx_node_list[uc_node_idx].us_short_address, cTime);
	} else {
		sprintf(c_print_result_test_query, "[0x%04x] step: %d Time %u \t >ERROR", spx_node_list[uc_node_idx].us_short_address, uc_step, cTime);
	}

	/* UPDATE MEAN CYCLE TIME */
	sx_cycles_stat[uc_node_idx].ul_cmean_time = (sx_cycles_stat[uc_node_idx].ul_cmean_time * (num_cycle - 1) + cTime) / num_cycle;
	LOG_APP_REPORT(("[DLMS_EMU] %s\r\n", c_print_result_test_query));
}
#endif

//***********************************************************************************
/**
 *************************************************************************************/
static uint8_t _checkReceivedData(uint8_t uc_step, uint8_t *data)
{
	uint8_t value;
	value = false;
	switch (uc_step) {
	case ASSOCIATION_REQUEST:
		if (memcmp(data, assoc_response_val, sizeof(assoc_response_val)) == 0) {
			value = true;
			LOG_APP_DEBUG(("[DLMS_EMU] ASSOCIATION_RESPONSE received\r\n"));
		} else {
			LOG_APP_DEBUG(("[DLMS_EMU] Error: wrong message received in DLMS step %d\r\n", uc_step));
		}
		break;

	case DATE_REQUEST:
		if (memcmp(data, get_request_date_val, sizeof(get_request_date_val)) == 0) {
			value = true;
			LOG_APP_DEBUG(("[DLMS_EMU] DATE_RESPONSE received\r\n"));
		} else {
			LOG_APP_DEBUG(("[DLMS_EMU] Error: wrong message received in DLMS step %d\r\n", uc_step));
		}
		break;

	case SO2_REQUEST:
		LOG_APP_DEBUG(("[DLMS_EMU] SO2_RESPONSE received\r\n"));
		value = true;
		break;

	case NEXBLOCK_REQUEST:
		if (memcmp(data, get_request_s02val, sizeof(get_request_s02val)) == 0) {
			value = true;
			if (*(data + 3) == 0x01) {
				/* last block received */
				suc_lastblock = 1;
			}
			LOG_APP_DEBUG(("[DLMS_EMU] NEXBLOCK_REQUEST received. Last block: %d\r\n", suc_lastblock));
		} else {
			LOG_APP_DEBUG(("[DLMS_EMU] Error: wrong message received in DLMS step %d\r\n", uc_step));
		}
		break;
	case RELEASE_REQUEST:
		if (memcmp(data, release_response_val, sizeof(release_response_val)) == 0) {
			value = true;
			LOG_APP_DEBUG(("[DLMS_EMU] RELEASE_RESPONSE received\r\n"));
		} else {
			LOG_APP_DEBUG(("[DLMS_EMU] Error: wrong message received in DLMS step %d\r\n", uc_step));
		}
		break;
	default:
		LOG_APP_DEBUG(("[DLMS_EMU] Error: wrong message received in DLMS step %d\r\n", uc_step));
		break;

	}
	return value;
}

//***********************************************************************************
/**
 *************************************************************************************/
static uint16_t _generateStr(uint8_t uc_step, uint8_t uc_invoke_order)
{
	uint16_t ui_lenghtmsg;
	uint8_t * puc_dlms_message;
	/* assign pointer to message */
	switch (uc_step) {
	case ASSOCIATION_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_associationRequest);
		puc_dlms_message = cmd_DLMS_associationRequest;
		LOG_APP_DEBUG(("[DLMS_EMU] ASSOCIATION_REQUEST sent to node [0x%04x]\r\n", spx_node_list[sus_node_cycling].us_short_address));
		break;
	case DATE_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_date_OBIS0000010000FF_0008_02);
		puc_dlms_message = cmd_DLMS_date_OBIS0000010000FF_0008_02;
		cmd_DLMS_date_OBIS0000010000FF_0008_02[2] = uc_invoke_order;
		LOG_APP_DEBUG(("[DLMS_EMU] DATE_REQUEST sent to node [0x%04x]\r\n", spx_node_list[sus_node_cycling].us_short_address));
		break;
	case SO2_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_S02_OBIS0100630100FF_0007_02);
		puc_dlms_message = cmd_DLMS_S02_OBIS0100630100FF_0007_02;
		cmd_DLMS_S02_OBIS0100630100FF_0007_02[2] = uc_invoke_order + 1;
		suc_lastblock = 0;
		suc_blocknumber = 1;
		LOG_APP_DEBUG(("[DLMS_EMU] S02_REQUEST sent to node [0x%04x]\r\n", spx_node_list[sus_node_cycling].us_short_address));
		break;
	case NEXBLOCK_REQUEST:
		ui_lenghtmsg = sizeof(cmd_DLMS_NextBlock);
		puc_dlms_message = cmd_DLMS_NextBlock;
		cmd_DLMS_NextBlock[2] = uc_invoke_order + 1;
		suc_blocknumber++;
		cmd_DLMS_NextBlock[6] = suc_blocknumber;
		LOG_APP_DEBUG(("[DLMS_EMU] NEXBLOCK_REQUEST sent to node [0x%04x]\r\n", spx_node_list[sus_node_cycling].us_short_address));
		break;
	case RELEASE_REQUEST:
	default:
		ui_lenghtmsg = sizeof(cmd_DLMS_release_request);
		puc_dlms_message = cmd_DLMS_release_request;
		LOG_APP_DEBUG(("[DLMS_EMU] RELEASE_REQUEST sent to node [0x%04x]\r\n", spx_node_list[sus_node_cycling].us_short_address));
		break;

	}

	/* copy msg to transmision buffer */
	memcpy(pUdpPayload_tx, puc_dlms_message, ui_lenghtmsg);

	return ui_lenghtmsg;
}

/***********************************************************************************
 **
 *************************************************************************************/
static bool _execute_cycle(uint16_t us_node, uint8_t uc_step, uint8_t* puc_error)
{
	bool end = false;
	uint16_t us_lenght_msg;
	size_t act_rx_size = 0;
	size_t act_tx_size = 0;
	uint16_t us_src_port = APP_SOCKET_PORT;
	IpAddr srcIpAddr;
	error_t x_error;

	/* Set link-local address, based on the PAN_ID and the short address */
	ipv6StringToAddr(APP_IPV6_GENERIC_LINK_LOCAL_ADDR, &srcIpAddr.ipv6Addr);
	/* Adapt the IPv6 address to the G3 connection data */
	srcIpAddr.ipv6Addr.b[8] = (uint8_t) (DLMS_G3_COORD_PAN_ID >> 8);
	srcIpAddr.ipv6Addr.b[9] = (uint8_t) (DLMS_G3_COORD_PAN_ID);
	srcIpAddr.ipv6Addr.b[14] = (uint8_t) (spx_node_list[us_node].us_short_address >> 8);
	srcIpAddr.ipv6Addr.b[15] = (uint8_t) (spx_node_list[us_node].us_short_address);
	srcIpAddr.length = sizeof(Ipv6Addr);

	socketReceiveFrom(px_udp_plc_socket, &srcIpAddr, &us_src_port, (uint8_t *)pUdpPayload_rx, DLMS_MAX_RX_SIZE, &act_rx_size, SOCKET_FLAG_WAIT_ALL);

	if(act_rx_size > 0) {
		s_new_cmd = true;
	} else {
		s_new_cmd = false;
	}

	switch (st_execute_cycle) {
	case SEND:
		LOG_APP_DEBUG(("[DLMS_EMU] _execute_cycle: SEND\r\n"));

		/* Get Initial time to start cycle with new node */
		if (uc_step == 0) {
			sul_start_time_node_cycle = _get_time_ms();
		}

		if (spx_node_list[us_node].us_short_address == DLMS_INVALID_ADDRESS) {
			*puc_error = ERROR_NON_CONNECTED_NODE;
			end = true;
			LOG_APP_DEBUG(("[DLMS_EMU] _execute_cycle: Nothing to send (no connected nodes)\r\n"));

		} else {
			LOG_APP_DEBUG(("[DLMS_EMU] _execute_cycle: SEND - sending data...\r\n"));
			us_lenght_msg = _generateStr(suc_step_cycling, 0xc1);

			x_error = socketSendTo(px_udp_plc_socket, &srcIpAddr, us_src_port, pUdpPayload_tx, us_lenght_msg, &act_tx_size, SOCKET_FLAG_WAIT_ALL);

			if(x_error != NO_ERROR) {
				LOG_APP_DEBUG(("[DLMS_EMU] Unsuccessful socketSendTo()!\r\n"));
			}

			sul_time_out = sul_dlms_time_out_data_cycles ;
			st_execute_cycle = WAIT_DATA_CONFIRM;
		}
		break;

	case WAIT_DATA_CONFIRM:
		if (!sul_time_out) {
			*puc_error = ERROR_TIMEOUT_DATA_CONFIRM;
			end = true;
			LOG_APP_DEBUG(("[DLMS_EMU] _execute_cycle: ERROR_TIMEOUT_DATA_CONFIRM\r\n"));
		}

		if (s_new_cmd) {
			LOG_APP_DEBUG(("[DLMS_EMU] _execute_cycle: WAIT_DATA_CONFIRM: s_new_cmd RX\r\n"));

			sul_time_out = sul_dlms_time_out_data_cycles ;

			if (_checkReceivedData(uc_step, pUdpPayload_rx) == true) {
				*puc_error = ERROR_NO_ERROR;
				end = true;
			} else {
				*puc_error = ERROR_INTEGRITY;
				end = false;
			}
			st_execute_cycle = SEND;
			s_new_cmd = false;
			LOG_APP_DEBUG(("[DLMS_EMU] _execute_cycle: WAIT_DATA_CONFIRM: Cmd processed. Error: %d\r\n", *puc_error));
		}
		break;

	default:
		/* To avoid warnings. This point will never be reached. */
		break;
	}

	if (end) {
		st_execute_cycle = SEND;
	}

	return end;
}

static error_t initialize_udp_ip_DC(void)
{
	error_t x_error;
	Ipv6Addr ipv6_addr;
	IpAddr localIpAddr;
	const IpAddr *px_localIpAddr = &localIpAddr;

	/* Select the G3 network adapter */
	netSetDriver(plc_interface, &g3_adapter);
	x_error = netConfigInterface(plc_interface);
	if(x_error) {
		return x_error;
	}

	/* Open UDP over PLC socket */
	px_udp_plc_socket = socketOpen(SOCKET_TYPE_DGRAM, SOCKET_IP_PROTO_UDP);

	/* Failed to open socket? */
  if(!px_udp_plc_socket) {
  	return ERROR_OPEN_FAILED;
  }

	/* Set timeout for blocking operations */
	x_error = socketSetTimeout(px_udp_plc_socket, APP_DEFAULT_TIMEOUT);
	if(x_error) return x_error;

	/* Set link-local address, based on the PAN_ID and the short address */
	ipv6StringToAddr(APP_IPV6_GENERIC_COORDINATOR_ADDR, &ipv6_addr);
	/* Adapt the IPv6 address to the G3 connection data */
	ipv6_addr.b[8] = (uint8_t) (G3_COORDINATOR_PAN_ID >> 8);
	ipv6_addr.b[9] = (uint8_t) (G3_COORDINATOR_PAN_ID);
	/* Data Concentrator's short address is 0x0000 */
	ipv6_addr.b[14] = (uint8_t) 0x00;
	ipv6_addr.b[15] = (uint8_t) 0x00;

	x_error = ipv6SetLinkLocalAddr(plc_interface, &ipv6_addr);
	if(x_error) return x_error;

	/* Associate the socket with the local IPv6 address and the respective port */
	localIpAddr.ipv6Addr = ipv6_addr;
	localIpAddr.length = sizeof(Ipv6Addr);
	x_error = socketBind(px_udp_plc_socket, px_localIpAddr, APP_SOCKET_PORT);
	if(x_error) return x_error;

	/* Associate the socket with the relevant interface */
	x_error = socketBindToInterface(px_udp_plc_socket, plc_interface);

	return x_error;
}

/**
 * \brief Create main Cycles Application task and create timer to update internal counters.
 *
 */
void dlms_emu_init(uint8_t *puc_ext_addr)
{
	//struct TAdpMacGetConfirm getConfirm;

	/* Default behaviour variables (timers in seconds) */
	sb_enable_wait_reg_nodes = true;
	sb_enable_dlms_cycles = false;
	sb_enable_ping_cycles = false;
	sb_enable_preq_cycles = true;
	sul_ping_cycle_time = 3;
	sul_ping_cycle_timeout = 60;
	sul_ping_cycle_ttl = 10;
	sul_ping_cycle_len = 10;
	sul_dlms_numdays_cycles = 1;
	cmd_DLMS_S02_OBIS0100630100FF_0007_02[53] += sul_dlms_numdays_cycles;
	sul_dlms_time_between_cycles = 20;
	sul_dlms_time_out_data_cycles = 50;
	sul_dlms_time_between_msg = 1;
	sul_dlms_time_waiting_start = 60;
	sul_dlms_time_wait_preq_cfm = 60;
	sul_dlms_time_between_preq_no_dlms = 60;

	/* Init variables and status */
	st_sort_cycles = SC_WAIT_INITIAL_LIST;
	st_registered_nodes = RN_INITIAL_STATE;
	st_execute_cycle = SEND;
	net_Info.us_num_nodes = 0;
	sus_last_position_in_use = 0;

	/* Clear Node Information */
	memset(spx_node_list, 0, sizeof(spx_node_list));
	memset(sx_cycles_stat, 0, sizeof(sx_cycles_stat));

	/* Pointers to IPv6 payloads */
	pUdpPayload_rx = (uint8_t *)(puc_rx_buff);
	pUdpPayload_tx = (uint8_t *)(puc_tx_buff);

	LOG_APP_REPORT(("[DLMS_EMU] ------------ START --------------\r\n"));
	LOG_APP_REPORT(("[DLMS_EMU] DLMS EMU Application: COORDINATOR\r\n"));
	LOG_APP_REPORT(("[DLMS_EMU] dlms_emu_init: SC_WAIT_START_CYCLES\r\n"));

	/* Init local vars */
	sul_timer_dlms_start_wait = sul_dlms_time_waiting_start;
	sul_timer_next_cycle = 0;
	sul_time_out = 0;
	sb_update_nodes = false;

	sul_cycles_counter = 1;
	sul_total_time_cycle = 0;

	memset(sx_path_nodes, 0, sizeof(sx_path_nodes));
	sul_timer_wait_path_cfm = 0;
	sul_timer_max_to_req_paths = sul_dlms_time_between_preq_no_dlms;

	/* Set Extended Address */
	memcpy(spuc_extended_address, puc_ext_addr, sizeof(spuc_extended_address));

	/* Initialize UDP-IP over G3 */
	initialize_udp_ip_DC();

	sb_ping_sent = false;
	_init_ping_cycle_status_data();

	sul_sys_time = time(NULL);
	gettimeofday(&sx_time_init_value, NULL);

	/* Set network info */
	net_Info.puc_extended_addr = spuc_extended_address;
	net_Info.us_num_nodes = 0;
	net_Info.us_num_path_nodes = 0;
	net_Info.px_node_list = spx_node_list;
	net_Info.px_path_nodes = sx_path_nodes;
}

static void _update_counter(uint32_t *pul_counter, uint32_t ul_time_diff)
{
	if (*pul_counter) {
		if (ul_time_diff >= *pul_counter) {
			*pul_counter = 0;
		} else {
			*pul_counter -= ul_time_diff;
		}
	}
}

/**
 * \brief Update internal counters.
 *
 */
static void _dlms_emu_update(void)
{
	time_t ul_curr_time;
	uint32_t ul_time_diff;

	ul_curr_time = time(NULL);

	if (sul_sys_time == ul_curr_time) {
		/* Do nothing */
		return;
	} else if (sul_sys_time > ul_curr_time) {
		/* Overflow */
		sul_sys_time = ul_curr_time;
		/* Do nothing */
		return;
	} else {
		ul_time_diff = ul_curr_time - sul_sys_time;
		sul_sys_time = ul_curr_time;

		/* Update counters */
		_update_counter(&sul_timer_dlms_start_wait, ul_time_diff);
		_update_counter(&sul_timer_next_cycle, ul_time_diff);
		_update_counter(&sul_time_out, ul_time_diff);

		if (sb_enable_preq_cycles) {
			_update_counter(&sul_timer_wait_path_cfm, ul_time_diff);
			_update_counter(&sul_timer_max_to_req_paths, ul_time_diff);
		}

	}
}

/**
 * \brief Periodic task to process Cycles App. Initialize and start Cycles Application and launch timer
 * to update internal counters.
 *
 */
void dlms_emu_process(void)
{
	uint8_t uc_error;
	error_t x_error;
	systime_t roundTripTime;
	uint32_t i;

	_dlms_emu_update();

	if (sb_enable_ping_cycles) {
		if (sb_ping_sent) {
			x_error = async_ping_rcv( &roundTripTime);

			if (x_error == ERROR_TIMEOUT) {
				//LOG_APP_DEBUG(("[DLMS_EMU] Ping Process Timeout  Error: %d \r\n", x_error));
			} else {
				if(x_error != NO_ERROR) {
					s_ping_cycle_status.as_ping_cycle_node_stats[s_ping_cycle_status.ul_current_node_ping].ul_ping_errors += 1;
					LOG_APP_REPORT(("[DLMS_EMU] Unsuccessful Ping! to 0x%04x Error: %d \r\n", spx_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address , x_error));
					sb_ping_sent = 0;
				} else {
					s_ping_cycle_status.as_ping_cycle_node_stats[s_ping_cycle_status.ul_current_node_ping].ul_pings_successful += 1;
					LOG_APP_REPORT(("[DLMS_EMU] Ping Successful to 0x%04x! RoundTripTime: %d\r\n", spx_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address, roundTripTime));
					sb_ping_sent = 0;
				}

				s_ping_cycle_status.ul_current_node_ping++;

				if (s_ping_cycle_status.ul_current_node_ping == net_Info.us_num_nodes) {
					s_ping_cycle_status.ul_current_node_ping = 0;
					for(i = 0; i < net_Info.us_num_nodes; i++){
						LOG_APP_REPORT(("[DLMS_EMU] Ping Report node 0x%04x  -->  Pings sent: %d  ; Successful Ping: %d; Errors: %d Success Rate : %f\r\n",spx_node_list[i].us_short_address, s_ping_cycle_status.as_ping_cycle_node_stats[i].ul_pings_sent, s_ping_cycle_status.as_ping_cycle_node_stats[i].ul_pings_successful, s_ping_cycle_status.as_ping_cycle_node_stats[i].ul_ping_errors, (100.0 *(float)s_ping_cycle_status.as_ping_cycle_node_stats[i].ul_pings_successful)/((float)s_ping_cycle_status.as_ping_cycle_node_stats[i].ul_pings_sent) ));
					}
				}
			}
		}

		if (!sb_ping_sent) {
			if (net_Info.us_num_nodes){
				x_error = _send_ping_to_dev(s_ping_cycle_status.ul_current_node_ping);
				if (x_error == NO_ERROR) {
				   LOG_APP_REPORT(("\r\n[DLMS_EMU] Ping to 0x%04x Sent ok!\r\n", spx_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address ));
				   s_ping_cycle_status.as_ping_cycle_node_stats[s_ping_cycle_status.ul_current_node_ping].ul_pings_sent += 1;
				   sb_ping_sent = true;
				} else {
				   LOG_APP_REPORT(("\r\n[DLMS_EMU] Fail sending Ping to 0x%04x!  Error: %d \r\n", spx_node_list[s_ping_cycle_status.ul_current_node_ping].us_short_address, x_error ));
				}

			}
		}
	}

	if (sul_timer_dlms_start_wait || sul_timer_next_cycle) {
		return;
	}

	if (sb_enable_preq_cycles) {
		if (sul_timer_wait_path_cfm) {
			return;
		}

		if (sul_timer_max_to_req_paths == 0) {
			/* Set status to SC_PATH_REQUEST_LIST */
			net_Info.us_num_path_nodes = 0;
			st_sort_cycles = SC_PATH_REQUEST_LIST;
		}
	}

	switch (st_sort_cycles) {
	case SC_WAIT_INITIAL_LIST:
		if (sb_update_nodes) {
			sb_update_nodes = false;
			net_Info.us_num_nodes = adp_mng_update_registered_nodes(&spx_node_list);

			if (net_Info.us_num_nodes) {
				uint16_t i = 0;
				LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process: Updated joined devices list.\n"));
				LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process: %d nodes registered.\r\n", net_Info.us_num_nodes));
				for(i = 0; i < net_Info.us_num_nodes; i++) {
					LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process:    Position: %d -> [0x%04x]\n", i, spx_node_list[i].us_short_address));
				}

				if (sb_enable_preq_cycles) {
					/* Get Path Nodes Info */
					net_Info.us_num_path_nodes = 0;
					st_sort_cycles = SC_PATH_REQUEST_LIST;
				} else {
					/* Start Cycles */
					st_sort_cycles = SC_TIME_NEXT_CYCLE;
				}

				/* Init Cycles Variables */
				sul_cycles_counter = 1;
				sul_total_time_cycle = 0;

				if (sb_enable_wait_reg_nodes) {
					/* Waiting Network Stability */
					sul_timer_dlms_start_wait = sul_dlms_time_waiting_start;
				}

				LOG_APP_DEBUG(("\n[DLMS_EMU] dlms_emu_process: SC_TIME_NEXT_CYCLE\r\n"));
		} else {
				/* Restart Waiting Timer */
				sul_timer_dlms_start_wait = sul_dlms_time_waiting_start;
			}
		} else {
			/* Restart Waiting Timer */
			sul_timer_dlms_start_wait = sul_dlms_time_waiting_start;
		}
		break;

	case SC_PATH_REQUEST_LIST:
		if (sb_update_nodes) {
			LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process: Updated joined devices list.\n"));
			sb_update_nodes = false;
			net_Info.us_num_nodes = adp_mng_update_registered_nodes(&spx_node_list);

			if (sb_enable_wait_reg_nodes) {
				sul_timer_dlms_start_wait = sul_dlms_time_waiting_start;
			}
			break;

		}

		if (net_Info.us_num_nodes == net_Info.us_num_path_nodes) {

			/* Start Cycles */
			st_sort_cycles = SC_TIME_NEXT_CYCLE;
			sul_timer_next_cycle = sul_dlms_time_between_cycles;
		} else {
			sul_timer_wait_path_cfm = sul_dlms_time_wait_preq_cfm;
			AdpPathDiscoveryRequest(spx_node_list[net_Info.us_num_path_nodes].us_short_address, ADP_PATH_METRIC_TYPE);
		}
		break;

	case SC_TIME_NEXT_CYCLE:
		if (sb_update_nodes) {
			LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process: Updated joined devices list.\n"));
			sb_update_nodes = false;
			net_Info.us_num_nodes = adp_mng_update_registered_nodes(&spx_node_list);

			if (sb_enable_wait_reg_nodes) {
				sul_timer_dlms_start_wait = sul_dlms_time_waiting_start;
				break;
			}
		}

		if (net_Info.us_num_nodes) {
			sul_start_time_cycle = _get_time_ms();
			sus_node_cycling = 0;
			suc_step_cycling = 0;
			st_sort_cycles = SC_CYCLES;

			LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process: %d nodes registered.\r\n", net_Info.us_num_nodes));
			for(i = 0; i < net_Info.us_num_nodes; i++) {
				LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process:    Position: %d -> [0x%04x]\n", i, spx_node_list[i].us_short_address));
			}
			LOG_APP_DEBUG(("\n[DLMS_EMU] dlms_emu_process: SC_CYCLES\r\n"));

		} else {
			/* Restart Waiting Timer */
			sul_timer_dlms_start_wait = sul_dlms_time_waiting_start;
			st_sort_cycles = SC_WAIT_INITIAL_LIST;
		}
		break;

	case SC_CYCLES:
		if (sb_enable_dlms_cycles) {
			if (sus_node_cycling < net_Info.us_num_nodes) {
				if (_execute_cycle(sus_node_cycling, suc_step_cycling, &uc_error)) {
					_updatecycle_stat(sus_node_cycling, suc_step_cycling, uc_error);
					_log_cycles_step(sul_cycles_counter, sus_node_cycling, uc_error, suc_step_cycling);
					if (uc_error != ERROR_NO_ERROR) {
						_print_result_test(sus_node_cycling, suc_step_cycling, uc_error, sul_cycles_counter);
						suc_step_cycling = 0;
						sus_node_cycling++;
					} else {
						if ((suc_lastblock == 0) && (suc_step_cycling == NEXBLOCK_REQUEST)) {

						} else {
							suc_step_cycling++;
							sul_timer_next_cycle = sul_dlms_time_between_msg      ;
							LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process: Waiting next cycle\r\n"));
						}

						if (suc_step_cycling == NUM_STEPS) {
							_log_S02_Status(sul_cycles_counter, sus_node_cycling);
							_print_result_test(sus_node_cycling, suc_step_cycling, uc_error, sul_cycles_counter);
							suc_step_cycling = 0;
							sus_node_cycling++;
						}
					}
				}
			} else {
				sul_total_time_cycle = _get_time_ms() - sul_start_time_cycle;
				_log_full_cycles_time(sul_cycles_counter, sul_total_time_cycle);
				sul_cycles_counter++;
				LOG_APP_DEBUG(("[DLMS_EMU] dlms_emu_process: SC_TIME_NEXT_CYCLE\r\n"));

				if (sb_enable_preq_cycles) {
					/* Get Path Nodes Info before each complete cycle */
					net_Info.us_num_path_nodes = 0;
					st_sort_cycles = SC_PATH_REQUEST_LIST;
				} else {
					/* Next Cycles */
					net_Info.us_num_nodes = adp_mng_update_registered_nodes(&spx_node_list);
					st_sort_cycles = SC_TIME_NEXT_CYCLE;
					sul_timer_next_cycle = sul_dlms_time_between_cycles;
				}
			}
		}
		break;
	}
}

/**
 * \brief Join Node Handler
 *
 */
void dlms_emu_join_node(uint8_t *puc_extended_address, uint16_t us_short_address)
{
	(void)puc_extended_address;
	(void)us_short_address;

	/* Set flag to update node list before */
	sb_update_nodes = true;

	/* Add node to the list */
	//_add_node_to_list(puc_extended_address, us_short_address);

	if (sb_enable_preq_cycles) {
		/* Restart timer */
		sul_timer_max_to_req_paths = sul_dlms_time_between_preq_no_dlms;
	}


}

/**
 * \brief Leave Node Handler
 *
 */
void dlms_emu_leave_node(uint16_t us_short_address)
{
	(void)us_short_address;

	/* Set flag to update node list before */
	sb_update_nodes = true;

	/* Remove node to the list */
	//_remove_node_from_list(us_short_address);

	if (sb_enable_preq_cycles) {
		/* Restart timer */
		sul_timer_max_to_req_paths = sul_dlms_time_between_preq_no_dlms;
	}
}

/**
 * \brief Data Indication Handler
 *
 */
void dlms_emu_data_ind_handler(void *pvDataIndication)
{
	/* Pass Data Indication to IP layer */
	ipv6_receive_packet ((struct TAdpDataIndication *)pvDataIndication);

	if (sb_enable_preq_cycles) {
		/* Restart timer */
		sul_timer_max_to_req_paths = sul_dlms_time_between_preq_no_dlms;
	}

}

/**
 * \brief Path Request Confirmation Handler
 *
 */
void dlms_emu_path_node_cfm(struct TAdpPathDiscoveryConfirm *pPathDiscoveryConfirm)
{
	if (sb_enable_preq_cycles) {
		struct TAdpPathDiscoveryConfirm *px_path_node;

		/* Use net_Info.us_num_path_nodes as index of the path table */
		px_path_node = &sx_path_nodes[net_Info.us_num_path_nodes];

		/* Update PATH info */
		memcpy(px_path_node, pPathDiscoveryConfirm, sizeof(struct TAdpPathDiscoveryConfirm));

		/* Reset PREQ waiting timer */
		sul_timer_wait_path_cfm = 0;

		/* Update next node */
		net_Info.us_num_path_nodes++;

		/* Update JSON object */
		dlms_emu_report_json_obj(&net_Info);

		/* Restart timer */
		sul_timer_max_to_req_paths = sul_dlms_time_between_preq_no_dlms;
	} else {
		(void)pPathDiscoveryConfirm;
	}
}

