/**
 * \file
 *
 * \brief HAL_USI: PLC Service Universal Serial Interface
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h> // POSIX terminal control definitions

#include "hal_utils.h"

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

#ifndef true
#define true    1
#endif

#ifndef false
#define false    1
#endif

/** \brief Minimum overhead introduced by the USI protocol */
/** \note (1 Start Byte, 2 Bytes (Len+Protocol), 1 End Byte, 1 CRC Byte) */
/* @{ */
#define MIN_OVERHEAD    5
/* @} */

/** \brief Special characters used by USI */
/* @{ */
/** Start/end mark in message */
#define MSGMARK       0x7e
/** Escaped start/end mark */
#define ESC_MSGMARK   0x5e
/** Escape mark in message */
#define ESCMARK       0x7d
/** Escaped escape mark */
#define ESC_ESCMARK   0x5d
/* @} */

/** \brief MSG started and ended flags */
/* @{ */
#define MSG_START 1
#define MSG_END   0
/* @} */

/** \brief USI header and CRC lengths */
/* @{ */
#define HEADER_LEN  2
#define CRC8_LEN    1
#define CRC16_LEN   2
#define CRC32_LEN   4
/* @} */

/** \brief USI protocol header format */
/* @{ */
#define TYPE_PROTOCOL_OFFSET       1
#define TYPE_PROTOCOL_MSK       0x3F
#define LEN_PROTOCOL_HI_OFFSET     0
#define LEN_PROTOCOL_HI_MSK     0xFF
#define LEN_PROTOCOL_HI_SHIFT      2
#define LEN_PROTOCOL_LO_OFFSET     1
#define LEN_PROTOCOL_LO_MSK     0xC0
#define LEN_PROTOCOL_LO_SHIFT      6
#define XLEN_PROTOCOL_OFFSET       2
#define XLEN_PROTOCOL_MSK       0x80
#define XLEN_PROTOCOL_SHIFT_L      3
#define XLEN_PROTOCOL_SHIFT_R     10
#define PAYLOAD_OFFSET             2
#define CMD_PROTOCOL_OFFSET        2
#define CMD_PROTOCOL_MSK        0x7F
/* @} */

/** \brief Macro operators
 * \note A: HI, B: LO, C:Xlen
 */
/* @{ */
#define TYPE_PROTOCOL(A)       ((A)&TYPE_PROTOCOL_MSK)
#define LEN_PROTOCOL(A, B)     ((((uint16_t)(A)) << LEN_PROTOCOL_HI_SHIFT) + ((B) >> LEN_PROTOCOL_LO_SHIFT))
#define XLEN_PROTOCOL(A, B, C) ((((uint16_t)(A)) << LEN_PROTOCOL_HI_SHIFT) \
	+ ((B) >> LEN_PROTOCOL_LO_SHIFT) \
	+ (((uint16_t)(C)&XLEN_PROTOCOL_MSK) << XLEN_PROTOCOL_SHIFT_L))
#define LEN_HI_PROTOCOL(A)    (((uint16_t)(A) >> LEN_PROTOCOL_HI_SHIFT) & LEN_PROTOCOL_HI_MSK)
#define LEN_LO_PROTOCOL(A)    (((uint16_t)(A) << LEN_PROTOCOL_LO_SHIFT) & LEN_PROTOCOL_LO_MSK)
#define LEN_EX_PROTOCOL(A)    (((uint16_t)(A & 0x0400)) >> 3)
#define CMD_PROTOCOL(A)       ((A)&CMD_PROTOCOL_MSK)
/* @} */

/** USI command structure */

/** USI communication control parameters structure */
typedef struct {
	/** Buffer index */
	uint16_t us_idx_in;
	/** Buffer data length */
	uint16_t us_len;
} USI_param_t;

/** Reception states */
enum {
	/** Inactive */
	RX_IDLE,
	/** Receiving message */
	RX_MSG,
	/** Processing escape char */
	RX_ESC,
	/** Message received correctly */
	RX_EORX
};

/** Transmission states */
enum {
	/** Inactive */
	TX_IDLE,
	/** Transmitting message */
	TX_MSG
};

/** USI communication control parameters (one entry per port) */
static USI_param_t usi_cfg_param;

/** Flag to check len integrity in usi process */
static bool sb_check_len;

/** Protocol port mapping */
typedef struct {
	/** Protocol Type */
	uint8_t uc_p_type;
	/** Communication Port */
	uint8_t uc_port;
} map_protocols_t;

/** Protocol configuration structure */
typedef struct {
	/** Serialization function pointer */
	uint8_t (*serialization_function)(uint8_t *puc_rx_msg, uint16_t us_len);
} protocol_config_t;

/** Port configuration */
typedef struct {
	/** Serial Communication Descriptor */
	int _fd;
	/** Port buffer size in transmission */
	uint16_t us_tx_size;
	/** Port buffer size in reception */
	uint16_t us_rx_size;
} map_ports_t;

/** Buffer handling structure */
typedef struct {
	/** Size of data in the buffer */
	uint16_t us_size;
	/** Size of the first USI message in the buffer */
	//uint16_t us_current_msg_size;
	/** Pointer to the buffer */
	uint8_t *puc_buf;
} map_buffers_t;

#define TXAUX_SIZE 1000

static map_ports_t usiMapPorts;

static map_buffers_t usi_rx_buf;
static map_buffers_t usi_tx_buf;

static uint8_t puc_rxbuf0[TXAUX_SIZE];
static uint8_t puc_txbuf0[TXAUX_SIZE];

/** Configuration of mapped protocols */
static protocol_config_t usi_cfg_map_protocols[USI_NUMBER_OF_PROTOCOLS];

static int _fd_socket_loopback;

/**
 * \brief Function that translates protocol IDs to indexes for the
 *        protocol tables (configuration and callback functions).
 *
 * \param protocol_id    Protocol identifier
 *
 * \return Protocol index
 */
static uint8_t _usi_prot_id2idx(usi_protocol_t protocol_id)
{
	uint8_t uc_prot_idx;

	switch (protocol_id) {
	case PROTOCOL_MNGP_PRIME_GETQRY:
	case PROTOCOL_MNGP_PRIME_GETRSP:
	case PROTOCOL_MNGP_PRIME_SET:
	case PROTOCOL_MNGP_PRIME_RESET:
	case PROTOCOL_MNGP_PRIME_REBOOT:
	case PROTOCOL_MNGP_PRIME_FU:
	case PROTOCOL_MNGP_PRIME_GETQRY_EN:
	case PROTOCOL_MNGP_PRIME_GETRSP_EN:
		uc_prot_idx = 0;
		break;

	case PROTOCOL_SNIF_PRIME:
		uc_prot_idx = 1;
		break;

	case PROTOCOL_ATPL230:
		uc_prot_idx = 2;
		break;

	case PROTOCOL_USER_DEFINED:
		uc_prot_idx = 3;
		break;

	case PROTOCOL_PRIME_API:
		uc_prot_idx = 4;
		break;

	case PROTOCOL_INTERNAL:
		uc_prot_idx = 5;
		break;

	default:
		uc_prot_idx = USI_INVALID_PROTOCOL_IDX;
	}

	return(uc_prot_idx);
}

/**
 * \brief  This function processes the complete received message.
 *
 * \retval true if there is no error
 * \retval false if length is invalid or command is wrong
 */
static uint8_t _process_msg(uint8_t *puc_rx_buf)
{
	uint8_t (*pf_serialization_function)(uint8_t *, uint16_t);
	uint16_t us_len;
	uint8_t uc_type;
	uint8_t uc_result = false;
	uint8_t uc_retx_len = 0;

	/* Extract protocol */
	uc_type = TYPE_PROTOCOL(puc_rx_buf[TYPE_PROTOCOL_OFFSET]);

	/* Extract length */
	if (uc_type == PROTOCOL_PRIME_API) {
		/* Extract LEN using XLEN */
		us_len = XLEN_PROTOCOL(puc_rx_buf[LEN_PROTOCOL_HI_OFFSET], puc_rx_buf[LEN_PROTOCOL_LO_OFFSET], puc_rx_buf[XLEN_PROTOCOL_OFFSET]);
	} else {
		/* Extract LEN using LEN */
		us_len = LEN_PROTOCOL(puc_rx_buf[LEN_PROTOCOL_HI_OFFSET], puc_rx_buf[LEN_PROTOCOL_LO_OFFSET]);
	}

	/* Protection for invalid length */
	if (!us_len && !((uc_type == PROTOCOL_MNGP_PRIME_RESET) || (uc_type == PROTOCOL_MNGP_PRIME_REBOOT))) {
		return false;
	}

	/* Call decoding function depending on uc_type */
	switch (uc_type) {
	/* PRIME spec.v.1.3.E */
	case PROTOCOL_MNGP_PRIME_GETQRY:
	case PROTOCOL_MNGP_PRIME_GETRSP:
	case PROTOCOL_MNGP_PRIME_SET:
	case PROTOCOL_MNGP_PRIME_RESET:
	case PROTOCOL_MNGP_PRIME_REBOOT:
	case PROTOCOL_MNGP_PRIME_FU:
	case PROTOCOL_MNGP_PRIME_GETQRY_EN:
	case PROTOCOL_MNGP_PRIME_GETRSP_EN:
		pf_serialization_function = usi_cfg_map_protocols[_usi_prot_id2idx(PROTOCOL_MNGP_PRIME)].serialization_function;
		if (pf_serialization_function) {
			if (_fd_socket_loopback > 0) {
				uc_retx_len = us_len;
			}
			/* MNGL spec. including header 2bytes) */
			uc_result = pf_serialization_function(puc_rx_buf, us_len + 2);
		}

		break;

	/* Atmel's serialized protocols */
	case PROTOCOL_ATPL230:
		pf_serialization_function = usi_cfg_map_protocols[_usi_prot_id2idx(PROTOCOL_ATPL230)].serialization_function;
		if (pf_serialization_function) {
			uc_result = pf_serialization_function(&puc_rx_buf[PAYLOAD_OFFSET], us_len);
		}

		break;

	case PROTOCOL_SNIF_PRIME:
		pf_serialization_function = usi_cfg_map_protocols[_usi_prot_id2idx(PROTOCOL_SNIF_PRIME)].serialization_function;
		if (pf_serialization_function) {
			uc_result = pf_serialization_function(&puc_rx_buf[PAYLOAD_OFFSET], us_len);
		}

		break;

	case PROTOCOL_PRIME_API:
		pf_serialization_function = usi_cfg_map_protocols[_usi_prot_id2idx(PROTOCOL_PRIME_API)].serialization_function;
		if (pf_serialization_function) {
			if (_fd_socket_loopback > 0) {
				uc_retx_len = us_len;
			}
			uc_result = pf_serialization_function(&puc_rx_buf[PAYLOAD_OFFSET], us_len);
		}

		break;

	case PROTOCOL_USER_DEFINED:
		pf_serialization_function = usi_cfg_map_protocols[_usi_prot_id2idx(PROTOCOL_USER_DEFINED)].serialization_function;
		if (pf_serialization_function) {
			uc_result = pf_serialization_function(&puc_rx_buf[PAYLOAD_OFFSET], us_len);
		}

		break;

	default:
		break;
	}

	if (uc_retx_len > 0) {
		/* Redirect message */
		x_usi_cmd_t usi_cmd;

		usi_cmd._fd = _fd_socket_loopback;
		usi_cmd.puc_buf = &puc_rx_buf[PAYLOAD_OFFSET];
		usi_cmd.uc_p_type = uc_type;
		usi_cmd.us_len = uc_retx_len;

		hal_usi_send_cmd(&usi_cmd);
	}

	return uc_result;
}

/**
 * \brief  This function processes received message
 *
 *  \param  puc_rx_buf       Pointer to the received buffer
 *  \param  us_msg_size      Buffer length
 *
 *  \return true      if message is OK
 *          false     if message is not OK
 */
static uint8_t _doEoMsg(uint8_t *puc_rx_buf, uint16_t us_msg_size)
{
	uint32_t ul_rx_crc;
	uint32_t ul_ev_crc;
	uint8_t *puc_tb;
	uint16_t us_len;
	uint8_t uc_type;

	/* Get buffer and number of bytes */
	if (us_msg_size < 4) {    /* insufficient data */
		return false;
	}

	/* Extract length and protocol */
	us_len = LEN_PROTOCOL(puc_rx_buf[LEN_PROTOCOL_HI_OFFSET], puc_rx_buf[LEN_PROTOCOL_LO_OFFSET]);
	uc_type = TYPE_PROTOCOL(puc_rx_buf[TYPE_PROTOCOL_OFFSET]);

	/* Protection for invalid length */
	if (!us_len && !((uc_type == PROTOCOL_MNGP_PRIME_RESET) || (uc_type == PROTOCOL_MNGP_PRIME_REBOOT))) {
		return false;
	}

	/* Evaluate CRC depending on protocol */
	switch (uc_type) {
	case PROTOCOL_MNGP_PRIME_GETQRY:
	case PROTOCOL_MNGP_PRIME_GETRSP:
	case PROTOCOL_MNGP_PRIME_SET:
	case PROTOCOL_MNGP_PRIME_RESET:
	case PROTOCOL_MNGP_PRIME_REBOOT:
	case PROTOCOL_MNGP_PRIME_FU:
	case PROTOCOL_MNGP_PRIME_GETQRY_EN:
	case PROTOCOL_MNGP_PRIME_GETRSP_EN:
		/* Get received CRC 32 */
		puc_tb = &puc_rx_buf[us_msg_size - 4];
		ul_rx_crc = (((uint32_t)puc_tb[0]) << 24) | (((uint32_t)puc_tb[1]) << 16) | (((uint32_t)puc_tb[2]) << 8) | ((uint32_t)puc_tb[3]);
		/* Calculate CRC */
		/* +2 header bytes: included in CRC */
		ul_ev_crc = hal_pcrc_calc(puc_rx_buf, us_len + 2, HAL_PCRC_HT_USI, HAL_PCRC_CRC_TYPE_32, false);
		break;

	case PROTOCOL_SNIF_PRIME:
	case PROTOCOL_ATPL230:
	case PROTOCOL_USER_DEFINED:
		/* Get received CRC 16 */
		puc_tb = &puc_rx_buf[us_msg_size - 2];
		ul_rx_crc = (((uint32_t)puc_tb[0]) << 8) | ((uint32_t)puc_tb[1]);
		/* Calculate CRC */
		/* +2 header bytes: included in CRC */
		ul_ev_crc = hal_pcrc_calc(puc_rx_buf, us_len + 2, HAL_PCRC_HT_USI, HAL_PCRC_CRC_TYPE_16, false);
		break;

	/* Length is up to 2Kb ... use XLEN field */
	case PROTOCOL_PRIME_API:
		/* Get received CRC 8: use XLEN */
		us_len = XLEN_PROTOCOL(puc_rx_buf[LEN_PROTOCOL_HI_OFFSET], puc_rx_buf[LEN_PROTOCOL_LO_OFFSET], puc_rx_buf[XLEN_PROTOCOL_OFFSET]);
		puc_tb = &puc_rx_buf[us_msg_size - 1];
		ul_rx_crc = (uint32_t)puc_tb[0];
		/* Calculate CRC */
		/* +2 header bytes: included in CRC */
		ul_ev_crc = hal_pcrc_calc(puc_rx_buf, us_len + 2, HAL_PCRC_HT_USI, HAL_PCRC_CRC_TYPE_8, false);
		break;

	default:
		return false;
	}

	/* Return CRC ok or not */
	if (ul_rx_crc == ul_ev_crc) {
		return true;
	} else {
		return false;
	}
}

/**
 * \brief     Shifts the transmission buffer one byte to the right, starting
 *            from the byte following the byte to escape, and thus creating
 *            room for the escaped byte.
 *            Note that the check to see if there is enough space for it
 *            must be done previous to calling this function.
 *
 */
static void _usi_shift_buffer_right(uint8_t *puc_tx_buf, uint16_t us_idx, uint16_t us_len)
{
	uint16_t us_orig, i;
	uint8_t uc_backup_chars[2];
	uint8_t uc_backup_char_idx;

	/* Get current buffer index (byte following the character to escape)*/
	us_orig = us_idx;

	/* Start with the second element in the backup char array */
	uc_backup_char_idx = 1;
	uc_backup_chars[uc_backup_char_idx] = puc_tx_buf[us_orig++];
	/* Toggle backup char index */
	uc_backup_char_idx = 1 - uc_backup_char_idx;
	i = 0;
	while (i < us_len) {
		uc_backup_chars[uc_backup_char_idx] = puc_tx_buf[us_orig + i];
		/* Toggle backup char index */
		uc_backup_char_idx = 1 - uc_backup_char_idx;
		/* Write the stored char in its adjacent position to the right */
		puc_tx_buf[us_orig + i] = uc_backup_chars[uc_backup_char_idx];
		i++;
	}
}

/**
 * \brief     Encodes the escape characters and transmits the message
 *
 *  \param    msg       Pointer to data to be transmitted
 *
 *
 *  \return   Result of operation:  USI_OK: Sent
 *                                  USI_STATUS_TX_FAILED: Not sent
 */
static usi_status_t _usi_encode_and_send(x_usi_cmd_t *msg)
{
	uint32_t ul_idx_in_orig;
	uint32_t ul_idx_aux = 0;
	uint32_t ul_crc;
	uint32_t ul_size_coded = 0;
	uint16_t us_len;
	uint16_t us_len_token = 0;
	uint8_t *puc_tx_buf;
	uint8_t *puc_tx_buf_ini;
	uint8_t *puc_next_token = NULL;
	uint8_t *puc_aux_next_token = NULL;
	uint8_t uc_cmd;
	uint8_t uc_delimiter = MSGMARK;
	uint8_t uc_escape    = ESCMARK;
	uint8_t uc_escaped_byte = 0;
	uint8_t uc_p_type;
	int i_sent_chars;
	int _fd_wr;

	us_len = msg->us_len;
	uc_p_type = msg->uc_p_type;

	/* Get ptr to TxBuffer */
	puc_tx_buf = usi_tx_buf.puc_buf;
	puc_tx_buf_ini = puc_tx_buf;

	/* Start Escape */
	ul_idx_in_orig = usi_cfg_param.us_idx_in;
	ul_idx_aux = usi_cfg_param.us_idx_in;
	*puc_tx_buf++ = MSGMARK;
	/* Copy message to TX buffer including header */
	*puc_tx_buf++ = LEN_HI_PROTOCOL(us_len);
	*puc_tx_buf++ = LEN_LO_PROTOCOL(us_len) + TYPE_PROTOCOL(uc_p_type);

	memcpy(puc_tx_buf, msg->puc_buf, us_len);
	/* Adjust XLEN if uc_p_type is internal protocol */
	uc_cmd = msg->puc_buf[0];
	if (uc_p_type == PROTOCOL_PRIME_API) {
		puc_tx_buf[0] = LEN_EX_PROTOCOL(us_len) + CMD_PROTOCOL(uc_cmd);
	}

	puc_tx_buf += us_len;

	/* Update the index to the end of the data to send */
	ul_idx_aux = puc_tx_buf - puc_tx_buf_ini;

	/* Add 1 MSGMARK + 2 header bytes to LEN */
	us_len += 3;

	/* Calculate CRC */
	switch (uc_p_type) {
	case PROTOCOL_MNGP_PRIME_GETQRY:
	case PROTOCOL_MNGP_PRIME_GETRSP:
	case PROTOCOL_MNGP_PRIME_SET:
	case PROTOCOL_MNGP_PRIME_RESET:
	case PROTOCOL_MNGP_PRIME_REBOOT:
	case PROTOCOL_MNGP_PRIME_FU:
	case PROTOCOL_MNGP_PRIME_GETQRY_EN:
	case PROTOCOL_MNGP_PRIME_GETRSP_EN:
		ul_crc = hal_pcrc_calc(&puc_tx_buf_ini[ul_idx_in_orig + 1], msg->us_len + 2, HAL_PCRC_HT_USI, HAL_PCRC_CRC_TYPE_32, false);
		*puc_tx_buf++ = (uint8_t)(ul_crc >> 24);
		*puc_tx_buf++ = (uint8_t)(ul_crc >> 16);
		*puc_tx_buf++ = (uint8_t)(ul_crc >> 8);
		*puc_tx_buf++ = (uint8_t)ul_crc;
		us_len += 4;
		break;

	case PROTOCOL_SNIF_PRIME:
	case PROTOCOL_ATPL230:
	case PROTOCOL_USER_DEFINED:
		ul_crc = hal_pcrc_calc(&puc_tx_buf_ini[ul_idx_in_orig + 1], msg->us_len + 2, HAL_PCRC_HT_USI, HAL_PCRC_CRC_TYPE_16, false);
		*puc_tx_buf++ = (uint8_t)(ul_crc >> 8);
		*puc_tx_buf++ = (uint8_t)(ul_crc);
		us_len += 2;
		break;

	case PROTOCOL_PRIME_API:
	default:
		ul_crc = hal_pcrc_calc(&puc_tx_buf_ini[ul_idx_in_orig + 1], msg->us_len + 2, HAL_PCRC_HT_USI, HAL_PCRC_CRC_TYPE_8, false);
		*puc_tx_buf++ = (uint8_t)(ul_crc);
		us_len += 1;
		break;
	}

	/* Fill tx buffer adding required escapes +1 for the still missing end MSGMARKs */
	ul_size_coded = us_len + 1;
	/* Check if there is still room */
	if (ul_idx_in_orig + ul_size_coded > usi_tx_buf.us_size) {
		/* No Room. Return error */
		return USI_STATUS_TX_BUFFER_OVERFLOW;
	}

	/* Reposition the index to the beginning of the data to send */
	ul_idx_aux = ul_idx_in_orig + 1; /* Skip the initial MSGMARK */
	us_len--; /* Skip the initial MSGMARK */

	puc_tx_buf = puc_tx_buf_ini;
	while (us_len) {
		/* Look for the next MSGMARK present within the data */
		puc_next_token = memchr(&puc_tx_buf[ul_idx_in_orig + ul_idx_aux], uc_delimiter, us_len);
		if (puc_next_token == NULL) {
			/* MSGMARK not found -> look for the next ESCMARK present within the data */
			puc_next_token = memchr(&puc_tx_buf[ul_idx_in_orig + ul_idx_aux], uc_escape, us_len);
			if (puc_next_token != NULL) {
				uc_escaped_byte = ESC_ESCMARK;
			}
		} else {
			/* Check if there is an ESCMARK before the MSGMARK */
			puc_aux_next_token = memchr(&puc_tx_buf[ul_idx_in_orig + ul_idx_aux],
					uc_escape,
					puc_next_token - &puc_tx_buf[ul_idx_in_orig + ul_idx_aux]);
			if (puc_aux_next_token != NULL) {
				uc_escaped_byte = ESC_ESCMARK;
				puc_next_token = puc_aux_next_token;
			} else {
				uc_escaped_byte = ESC_MSGMARK;
			}
		}

		/* Perform the codification of the MSGMARK or the ESCMARK */
		if (puc_next_token != NULL) {
			ul_size_coded++;

			/* Check if there is still room */
			if (ul_idx_in_orig + ul_size_coded > usi_tx_buf.us_size) {
				/* No Room. Reset index and return error */
				return USI_STATUS_TX_BUFFER_OVERFLOW;
			}

			us_len_token = puc_next_token - &puc_tx_buf[ul_idx_in_orig + ul_idx_aux];
			/* Encode the special character */
			ul_idx_aux += us_len_token;
			puc_tx_buf[ul_idx_aux++] = ESCMARK;
			us_len -= us_len_token + 1;

			/* Shift the whole buffer by one byte to make room for the escape character */
			_usi_shift_buffer_right(puc_tx_buf, ul_idx_aux, us_len);

			puc_tx_buf[ul_idx_aux++] = uc_escaped_byte;
		} else { /* No MSGMARK or ESCMARK found. The buffer is complete */
			ul_idx_aux += us_len;
			us_len = 0;
		}
	}

	puc_tx_buf[ul_idx_aux++] = MSGMARK;

	/* Message ready to be sent */
	usi_cfg_param.us_idx_in = ul_idx_aux;
	us_len = ul_idx_aux;
	
	/* Select File Descriptor to write: If param is not valid -> send to port by default in map port */
	if (msg->_fd < 0) {
		_fd_wr = usiMapPorts._fd;
	} else {
		_fd_wr = msg->_fd;
	}

	while (usi_cfg_param.us_idx_in) {
		/* Send chars to device, checking how many have been
		 * really processed by device */
		i_sent_chars = write(_fd_wr, usi_tx_buf.puc_buf, us_len);

		if (i_sent_chars > 0) {
			/* Adjust buffer values depending on sent chars */
			usi_cfg_param.us_idx_in -= i_sent_chars;
		} else {
			/* Discard msg */
			usi_cfg_param.us_idx_in -= us_len;
			/* USI_ERROR: UART/USART error */
			return USI_STATUS_UART_ERROR;
		}
	}

	return USI_STATUS_OK;
}

/**
 * \brief     Shifts the given buffer us_n bytes to the left, starting from
 *            the us_n-th + 1 byte (that will then become the first one).
 *            The us_n last positions of the buffer are padded with 0.
 *
 *  \param    puc_buf  Pointer to the byte to de-escape.
 *  \param    us_n     Number pf bytes to shift.
 *  \param    us_len   Size of the buffer to shift.
 *
 */
static void _usi_shift_buffer_left(uint8_t *puc_buf, int16_t us_n, int16_t us_len)
{
	uint16_t i = us_n;

	/* Start with the us_n-th element in the array */
	while (i < us_len) {
		puc_buf[i - us_n] = puc_buf[i];
		i++;
	}
	/* Zero pad the end of the buffer */
	i = us_len - us_n;
	while (i < us_len) {
		puc_buf[i++] = 0;
	}
}

/** \brief   Decodes the escape sequences and updates the input buffer
 *           accordingly.
 *
 *  \param   puc_start  pointer to the input buffer
 *  \param   puc_end    pointer to the output buffer
 *
 *  \return  Updated length of the decoded array,
 *           0 if error.
 */
static uint16_t _decode_copy(uint8_t *puc_start, uint8_t *puc_end)
{
	uint16_t i = 0;
	uint16_t us_curr_size;

	us_curr_size = puc_end - puc_start - 1;
	if (!us_curr_size) {
		return 0;
	}

	while (i < us_curr_size) {
		if (puc_start[i] == ESCMARK) {
			/* If the escaped byte is the MSGMARK, */
			/* copy it to the destination buffer */
			if (puc_start[i + 1] == ESC_MSGMARK) {
				puc_start[i] = MSGMARK;
				_usi_shift_buffer_left(&puc_start[i + 1], 1, us_curr_size - i);
				us_curr_size--;
				i++;
			} else if (puc_start[i + 1] == ESC_ESCMARK) {
				/* Idem, for ESCMARK */
				puc_start[i] = ESCMARK;
				_usi_shift_buffer_left(&puc_start[i + 1], 1, us_curr_size - i);
				us_curr_size--;
				i++;
			} else { /* Error: unknown escaped character */
				return 0;
			}
		} else {
			/* No escape mark: continue */
			i++;
		}
	}

	return(us_curr_size);
}

/**
 * \brief  This function checks len of the received message
 *
 *  \param  puc_rx_start       Pointer to start msg
 *  \param  us_msg_len         Message len to check
 *  \return true      if message is OK
 *          false     if message is not OK
 */
static bool _check_integrity_len(uint8_t *puc_rx_start, uint16_t us_msg_len)
{
	uint8_t *puc_data;
	uint16_t us_len;
	uint8_t uc_type;

	/* get msg size */
	puc_data = puc_rx_start + 1;

	/* Get buffer and number of bytes */
	if (us_msg_len < 4) {    /* insufficient data */
		return false;
	}

	/* Extract length and protocol */
	us_len = LEN_PROTOCOL(puc_data[LEN_PROTOCOL_HI_OFFSET], puc_data[LEN_PROTOCOL_LO_OFFSET]);
	uc_type = TYPE_PROTOCOL(puc_data[TYPE_PROTOCOL_OFFSET]);

	/* Add CRC bytes depending on protocol */
	switch (uc_type) {
	case PROTOCOL_MNGP_PRIME_GETQRY:
	case PROTOCOL_MNGP_PRIME_GETRSP:
	case PROTOCOL_MNGP_PRIME_SET:
	case PROTOCOL_MNGP_PRIME_RESET:
	case PROTOCOL_MNGP_PRIME_REBOOT:
	case PROTOCOL_MNGP_PRIME_FU:
	case PROTOCOL_MNGP_PRIME_GETQRY_EN:
	case PROTOCOL_MNGP_PRIME_GETRSP_EN:
		us_len += 4;
		break;

	case PROTOCOL_SNIF_PRIME:
	case PROTOCOL_ATPL230:
	case PROTOCOL_USER_DEFINED:
		us_len += 2;
		break;

	/* Length is up to 2Kb ... use XLEN field */
	case PROTOCOL_PRIME_API:
		/* Get received CRC 8: use XLEN */
		us_len = XLEN_PROTOCOL(puc_data[LEN_PROTOCOL_HI_OFFSET], puc_data[LEN_PROTOCOL_LO_OFFSET], puc_data[XLEN_PROTOCOL_OFFSET]);
		us_len++;
		break;

	default:
		return false;
	}

	/* Add header bytes */
	us_len += 2;

	if (us_len == us_msg_len) {
		return true;
	} else {
		return false;
	}
}

/**
 * \brief Function to perform the USI RX process.
 */
void hal_usi_process(void)
{
	uint8_t *puc_rx_start;
	uint8_t *puc_rx_aux;
	uint8_t *puc_first_token;
	uint8_t *puc_last_token;
	uint16_t us_msg_size;
	uint16_t us_msg_size_new = 0;
	uint16_t us_msg_size_pending;

	us_msg_size_pending = usi_rx_buf.us_size;
	puc_rx_aux = usi_rx_buf.puc_buf + us_msg_size_pending;
	puc_rx_start = usi_rx_buf.puc_buf;

	us_msg_size_new = read(usiMapPorts._fd, puc_rx_aux, usiMapPorts.us_rx_size);

	/* Find first byte 0x7E */
	us_msg_size_pending += us_msg_size_new;

	if (us_msg_size_pending) {
		puc_first_token = memchr(puc_rx_start, (uint8_t)MSGMARK, us_msg_size_pending);
		if (puc_first_token) {
			/* Update RX pointer to first decoded data byte */
			puc_rx_aux = puc_first_token + 1;
			/* Process received data */
			puc_last_token = memchr(puc_rx_aux, (uint8_t)MSGMARK, us_msg_size_pending);
			if (puc_last_token) {
				uint16_t us_msg_dec_size;
				/* Decode message */
				us_msg_size = puc_last_token - puc_rx_start + 1;
				us_msg_dec_size = _decode_copy(puc_first_token, puc_last_token);
				/* Check integrity len to resync frames */
				if (sb_check_len) {
					sb_check_len = false;
					/* Check integrity LEN */
					if (!_check_integrity_len(puc_first_token, us_msg_dec_size)) {
						/* ERROR: discard message except last 0x7E. It is the first token of the next frame */
						_usi_shift_buffer_left(puc_rx_start, us_msg_size - 1, us_msg_size_pending);
						usi_rx_buf.us_size = us_msg_size_pending - (us_msg_size - 1);
						return;
					}
				}

				/* Calculate CRC */
				if (_doEoMsg(puc_rx_aux, us_msg_dec_size)) {
					/* CRC is OK: process the message */
					_process_msg(puc_rx_aux);
				}

				/* Update RX pointer to next process */
				_usi_shift_buffer_left(puc_rx_start, us_msg_size, us_msg_size_pending);
				usi_rx_buf.us_size = us_msg_size_pending - us_msg_size;
			} else {
				/* ERROR: Not 0x7E as last byte. Wait and check integrity through msg len in next process */
				sb_check_len = true;
				usi_rx_buf.us_size = us_msg_size_pending;
			}
		} else {
			/* ERROR: Not 0x7E as first byte. Discard RX */
			usi_rx_buf.us_size = 0;
			memset(puc_rx_start, 0, us_msg_size_pending);
		}
	}

}

/**
 * \brief Create RX and TX USI tasks, and timer to update internal counters.
 */
void hal_usi_init(void)
{
	uint8_t i;

	/* Init callback functions list */
	for (i = 0; i < USI_NUMBER_OF_PROTOCOLS; i++) {
		usi_cfg_map_protocols[i].serialization_function = NULL;
	}

	/* Init internal flag */
	sb_check_len = false;
	_fd_socket_loopback = -1;

	usi_tx_buf.puc_buf = puc_txbuf0;
	usi_rx_buf.puc_buf = puc_rxbuf0;
}

/**
 * \brief Open Serial Communication Port
 */
int hal_usi_open(char *sz_tty_name, unsigned ui_baudrate)
{
	/* structure to store the port settings in */
	struct termios port_settings;
	unsigned int ui_br = B115200;

	/* open device file */
	int fd = open(sz_tty_name, O_RDWR | O_NDELAY | O_NONBLOCK);

	if(fd == -1)  {
		return -1;
	}

	/*Check valid baudrates */
	if (ui_baudrate == 115200) {
		ui_br = B115200;
	} else if (ui_baudrate == 230400) {
		ui_br = B230400;
	} else if (ui_baudrate == 57600) {
		ui_br = B57600;
	} else if (ui_baudrate == 38400) {
		ui_br = B38400;
	} else if (ui_baudrate == 19200) {
		ui_br = B19200;
	} else {
		return -1;
	}

	memset(&port_settings,0,sizeof(port_settings));

	/* set baud rates */
	cfsetispeed(&port_settings, ui_br);
	cfsetospeed(&port_settings, ui_br);

	port_settings.c_iflag = 0;
	port_settings.c_oflag = 0;
	port_settings.c_lflag = 0;
	port_settings.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
	port_settings.c_cflag &= ~CSTOPB;
	port_settings.c_cflag &= ~CSIZE;
	port_settings.c_cflag |= CS8;
	port_settings.c_cflag |= CREAD;
	port_settings.c_cflag |= CLOCAL; // 8n1, see termios.h for more information

	/* apply the settings to the port */
	tcsetattr(fd, TCSANOW, &port_settings);
	tcflush(fd, TCIOFLUSH);

	/* Config USI internal buffer pointers */
	usi_cfg_param.us_idx_in = 0;
	usiMapPorts._fd = fd;
	usiMapPorts.us_rx_size = sizeof(puc_rxbuf0);
	usiMapPorts.us_tx_size = sizeof(puc_txbuf0);
	usi_tx_buf.us_size = sizeof(puc_txbuf0);

	return(fd);
}

/** @brief  Function to transmit data through USI
 *
 *  @param    msg    Pointer to message to be transmitted.
 *
 *  @return   USI operation result
 */
usi_status_t hal_usi_send_cmd(void *msg)
{
	x_usi_cmd_t * cmd_msg;
	uint16_t us_available_len;
	uint8_t uc_protocol_idx;

	cmd_msg = (x_usi_cmd_t *)msg;

	/* Len protection */
	if (cmd_msg->us_len == 0) {
		return USI_STATUS_FORMAT_ERROR;
	}

	/* Get port index from protocol */
	uc_protocol_idx = _usi_prot_id2idx((usi_protocol_t)cmd_msg->uc_p_type);
	if (uc_protocol_idx  == USI_INVALID_PROTOCOL_IDX) {
		return USI_STATUS_PROTOCOL_NOT_FOUND;
	}

	/* Get available length in buffer */
	us_available_len = usi_tx_buf.us_size - usi_cfg_param.us_idx_in;

	/* First checking, available length at least equal to minimum required space */
	if (us_available_len < (cmd_msg->us_len + MIN_OVERHEAD)) {
		return USI_STATUS_RX_BUFFER_OVERFLOW;
	}

	return (_usi_encode_and_send(cmd_msg));
}

/**
 * \brief Sets a callback function for the serialization of a given protocol
 *
 * \param protocol_id ID of the protocol to be serialized.
 * \param p_handler   Serializarion callback function pointer.
 * \param serial_port Port through which the protocol communication shall be
 * driven.
 */
usi_status_t hal_usi_set_callback(usi_protocol_t protocol_id, uint8_t (*p_handler)(uint8_t *puc_rx_msg, uint16_t us_len))
{
	uint8_t uc_prot_idx;

	uc_prot_idx = _usi_prot_id2idx(protocol_id);

	if (uc_prot_idx  == USI_INVALID_PROTOCOL_IDX) {
		return(USI_STATUS_PROTOCOL_NOT_FOUND);
	}

	usi_cfg_map_protocols[uc_prot_idx].serialization_function = p_handler;

	return(USI_STATUS_OK);
}

void hal_usi_loopback(int _fd_redirect)
{
	_fd_socket_loopback = _fd_redirect;
}

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
