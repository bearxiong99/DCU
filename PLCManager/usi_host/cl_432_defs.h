/**
 * \file
 *
 * \brief CL_432_DEFS: CONV 432 layer
 *
 * Copyright (c) 2015 Atmel Corporation. All rights reserved.
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
#ifndef CL_432_DEFS_H_INCLUDE
#define CL_432_DEFS_H_INCLUDE

#include <stdint.h>

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \ingroup prime_ng_group
 * \defgroup prime_sscs_432_group PRIME SSCS 4-32
 *
 * This module provides configuration and utils for the
 * SSCS 4-32 in PRIME.
 *
 * @{
 */

/** Length of the LPDU */
#define LPDU_HEADER                    3

/** \brief LSDU Data length */
/** \note It must be smaller than PRIME_MACSAP_DATA_SIZE */
/* @{ */
#define MAX_LENGTH_432_DATA            (1024 - LPDU_HEADER)
/* @} */

/** RESULTS values for convergence layer primitives */
typedef enum {
	DL_432_RESULT_SUCCESS = 0,
	DL_432_RESULT_REJECT = 1,
	DL_432_RESULT_TIMEOUT = 2,
	DL_432_RESULT_NOT_REGISTERED = 6
} dl_432_result_t;

/** LSDU part of DL message */
#define lsdu                           dl.buff

/** Buffer defined for reception/transmission */
typedef union {
	uint8_t lpdu[MAX_LENGTH_432_DATA + LPDU_HEADER];

	struct {
		uint8_t control;
		uint8_t dsap;
		uint8_t lsap;
		uint8_t buff[MAX_LENGTH_432_DATA];
	} dl;
} dl_432_buffer_t;

/** \brief Addresses defined in 432 layer */
/* @{ */
#define CL_432_INVALID_ADDRESS                 (0xFFFF)
#define CL_432_BROADCAST_ADDRESS               (0x0FFF)
/* @} */

/** \brief Transmission errors defined in 432 layer */
/* @{ */
#define CL_432_TX_STATUS_SUCCESS  0
#define CL_432_TX_STATUS_TIMEOUT  2
#define CL_432_TX_STATUS_ERROR_BAD_ADDRESS     (0xC0)
#define CL_432_TX_STATUS_ERROR_BAD_HANLDER     (0xC1)
#define CL_432_TX_STATUS_PREVIOUS_COMM         (0xC2)
/* @} */

/* @} */

/* / @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* / @endcond */
#endif
