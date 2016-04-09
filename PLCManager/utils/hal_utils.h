/**
 * \file
 *
 * \brief HAL_USI: PLC Service Universal Serial Interface
 *
 *
 */

#ifndef HAL_UTILS_H_INCLUDE
#define HAL_UTILS_H_INCLUDE

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

#include "stdint.h"
#include "stdbool.h"

/** Serial Protocols */
typedef enum {
	PROTOCOL_MNGP_PRIME           = 0x00,
	PROTOCOL_MNGP_PRIME_GETQRY    = 0x00,
	PROTOCOL_MNGP_PRIME_GETRSP    = 0x01,
	PROTOCOL_MNGP_PRIME_SET       = 0x02,
	PROTOCOL_MNGP_PRIME_RESET     = 0x03,
	PROTOCOL_MNGP_PRIME_REBOOT    = 0x04,
	PROTOCOL_MNGP_PRIME_FU        = 0x05,
	PROTOCOL_MNGP_PRIME_GETQRY_EN = 0x06,
	PROTOCOL_MNGP_PRIME_GETRSP_EN = 0x07,
	PROTOCOL_MNGP_PRIME_LISTQRY   = 0x0E,
	PROTOCOL_MNGP_PRIME_LISTRSP   = 0x0F,
	PROTOCOL_SNIF_PRIME           = 0x13,
	PROTOCOL_ATPL230              = 0x22,
	PROTOCOL_PRIME_API            = 0x30,
	PROTOCOL_INTERNAL             = 0x3F,
	PROTOCOL_USER_DEFINED         = 0xFE,
	PROTOCOL_INVALID              = 0xFF
} usi_protocol_t;

/** Number of USI supported protocols */
#define USI_NUMBER_OF_PROTOCOLS            6
/** Invalid protocol index */
#define USI_INVALID_PROTOCOL_IDX           0xFF

/** USI operation results */
typedef enum {
	USI_STATUS_PROTOCOL_NOT_FOUND,
	USI_STATUS_PROTOCOL_NOT_REGISTERED,
	USI_STATUS_TX_BUFFER_OVERFLOW,
	USI_STATUS_TX_BUSY,
	USI_STATUS_TX_BLOCKED,
	USI_STATUS_RX_BUFFER_OVERFLOW,
	USI_STATUS_RX_BLOCKED,
	USI_STATUS_UART_ERROR,
	USI_STATUS_FORMAT_ERROR,
	USI_STATUS_OK,
	USI_STATUS_INVALID
} usi_status_t;

/** CRC types */
enum HAL_PCRC_CRC_types {
	HAL_PCRC_CRC_TYPE_8 = 0,
	HAL_PCRC_CRC_TYPE_16 = 1,
	HAL_PCRC_CRC_TYPE_24 = 2,
	HAL_PCRC_CRC_TYPE_32 = 3,
};

/** Header type: GENERIC PACKET */
#define HAL_PCRC_HT_GENERIC                0
/** Header type: PROMOTION PACKET */
#define HAL_PCRC_HT_PROMOTION              1
/** Header type: BEACON PACKET */
#define HAL_PCRC_HT_BEACON                 2
/** Header type: USI message */
#define HAL_PCRC_HT_USI                    3
/* @} */

/** Invalid CRC */
#define HAL_PCRC_CRC_INVALID               0xFFFFFFFF

/** Message Structure to communicate with USI layer */
typedef struct {
	/** Communication File Descriptor */
	int _fd;
	/** Protocol Type */
	uint8_t uc_p_type;
	/** Pointer to data buffer */
	uint8_t *puc_buf;
	/** Length of data */
	uint16_t us_len;
} x_usi_cmd_t;

/** \brief Types to manage commands */
typedef uint8_t (*pf_usi_get_cmd)(void);
typedef void (*pf_usi_set_cmd)(uint8_t);

void hal_usi_init(void);
int hal_usi_open(char *sz_tty_name, unsigned ui_baudrate);
void hal_usi_process(void);
usi_status_t hal_usi_set_callback(usi_protocol_t protocol_id, uint8_t (*p_handler)(uint8_t *puc_rx_msg, uint16_t us_len));
usi_status_t hal_usi_send_cmd(void *msg);
void hal_usi_loopback(int _fd_redirect);

uint32_t hal_pcrc_calc(uint8_t *puc_buf, uint32_t ul_len, uint8_t uc_header_type, uint8_t uc_crc_type, bool b_v14_mode);
uint32_t hal_pcrc_calc_fu(uint8_t *puc_buf, uint32_t ul_len, uint32_t ul_crc_init);
void hal_pcrc_config_sna(uint8_t *puc_sna);


/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
#endif /* HAL_UTILS_H_INCLUDE */
