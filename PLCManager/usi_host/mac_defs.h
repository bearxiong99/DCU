/**
 * \file
 *
 * \brief MAC_DEFS: PRIME MAC definitions
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

#ifndef MAC_DEFS_H_INCLUDE
#define MAC_DEFS_H_INCLUDE

/* System includes */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
extern "C" {
#endif
/**INDENT-ON**/
/* @endcond */

/**
 * \ingroup prime_ng_group
 * \defgroup prime_mac_group PRIME MAC Layer
 *
 * This module provides configuration and utils for
 * the MAC layer of PRIME.
 * @{
 */

/** \brief MAC invalid values */
/* @{ */
#define MAC_INVALID_SID                         0xFF
#define MAC_INVALID_LNID                        0x3FFF
#define MAC_INVALID_LSID                        0xFF
#define MAC_BROADCAST_LNID                      0x3FFF
#define MAC_MULTICAST_LNID                      0x3FFE
#define MAC_INVALID_HANDLER                     0xFFFF
#define MAC_INVALID_LCID                        0xFFFF
/* @} */

/** PRIME version customer values */
typedef struct {
	char fw_version[16];
	char fw_model[16];
	char fw_vendor[16];
	char pib_version[16];
	uint16_t pib_vendor;
	uint16_t pib_model;
} mac_version_info_t;

/** PLME result values */
typedef enum {
	PLME_RESULT_SUCCESS     = 0,
	PLME_RESULT_FAILED      = 1,
	PLME_RESULT_REJECTED    = 2,
	PLME_RESULT_BADATTR     = 2,
} plme_result_t;

/** MLME result values */
typedef enum {
	MLME_RESULT_DONE        = 0,
	MLME_RESULT_FAILED      = 1,
	MLME_RESULT_REJECTED    = 1,
	MLME_RESULT_TIMEOUT     = 2,
	MLME_RESULT_NOSNA       = 8,
	MLME_RESULT_NOSWITCH    = 9,
	MLME_RESULT_REDUNDANT   = 10,
	MLME_RESULT_BADATTR     = 11,
	MLME_RESULT_OUTOFRANGE  = 12,
	MLME_RESULT_READONLY    = 13
} mlme_result_t;

/** Connection types */
typedef enum {
	MAC_CONNECTION_INVALID_TYPE         = 0,
	MAC_CONNECTION_IPV4_AR_TYPE         = 1,
	MAC_CONNECTION_IPV4_UNICAST_TYPE    = 2,
	MAC_CONNECTION_CL_432_TYPE          = 3,
	MAC_CONNECTION_MNGT_TYPE            = 4,
	MAC_CONNECTION_IPV6_AR_TYPE         = 5,
	MAC_CONNECTION_IPV6_UNICAST_TYPE    = 6,
} connection_type_t;

/** MAC SAP result values for MAC_ESTABLISH.confirm primitive */
typedef enum {
	MAC_ESTABLISH_CONFIRM_RESULT_SUCCESS            = 0,
	MAC_ESTABLISH_CONFIRM_RESULT_REJECT             = 1,
	MAC_ESTABLISH_CONFIRM_RESULT_TIMEOUT            = 2,
	MAC_ESTABLISH_CONFIRM_RESULT_NO_BANDWIDTH       = 3,
	MAC_ESTABLISH_CONFIRM_RESULT_NO_SUCH_DEVICE     = 4,
	MAC_ESTABLISH_CONFIRM_RESULT_REDIRECT_FAILED    = 5,
	MAC_ESTABLISH_CONFIRM_RESULT_NOT_REGISTERED     = 6,
	MAC_ESTABLISH_CONFIRM_RESULT_NO_MORE_LCIDS      = 7,
	MAC_ESTABLISH_CONFIRM_RESULT_PROCCESS_ACTIVE    = 0x80
} mac_establish_confirm_result_t;

/** Values for the answer parameter in MAC_ESTABLISH.response primitive */
typedef enum {
	MAC_ESTABLISH_RESPONSE_ANSWER_ACCEPT    = 0,
	MAC_ESTABLISH_RESPONSE_ANSWER_REJECT    = 1,
} mac_establish_response_answer_t;

/** Values for the reason parameter in MAC_RELEASE.indication primitive */
typedef enum {
	MAC_RELEASE_INDICATION_REASON_SUCCESS   = 0,
	MAC_RELEASE_INDICATION_REASON_ERROR     = 1,
} mac_release_indication_reason_t;

/** Values for the answer parameter in MAC_RELEASE.response primitive */
typedef enum {
	MAC_RELEASE_RESPONSE_ACCEPT = 0,
} mac_release_response_answer_t;

/** Values for the result parameter in MAC_RELEASE.confirm primitive */
typedef enum {
	MAC_RELEASE_CONFIRM_RESULT_SUCCESS          = 0,
	MAC_RELEASE_CONFIRM_RESULT_TIMEOUT          = 2,
	MAC_RELEASE_CONFIRM_RESULT_NOT_REGISTERED   = 6,
	MAC_RELEASE_CONFIRM_RESULT_PROCCESS_ACTIVE  = 0x80,
	MAC_RELEASE_CONFIRM_RESULT_BAD_HANDLER      = 0x81,
	MAC_RELEASE_CONFIRM_RESULT_NOT_OPEN_CONN    = 0x82,
	MAC_RELEASE_CONFIRM_RESULT_ERROR_SENDING    = 0x83,
	MAC_RELEASE_CONFIRM_RESULT_BAD_FLOW_MODE    = 0x84,
} mac_release_confirm_result_t;

/** Values of the Result parameter in MAC_DATA.confirm primitive */
typedef enum {
	MAC_DATA_SUCCESS = 0,
	MAC_DATA_TIMEOUT = 2,
	MAC_DATA_ERROR_SENDING = 0x80
} mac_data_result_t;

/** Type of join request */
typedef enum {
	REQUEST_MULTICAST       = 0,
	REQUEST_BROADCAST       = 1,
} mac_join_mode_t;

/** Values for the result parameter in MAC_JOIN.confirm primitive */
typedef enum {
	JOIN_CONFIRM_SUCCESS    = 0,
	JOIN_CONFIRM_FAILURE    = 1,
	JOIN_CONFIRM_TIMEOUT    = 2,
	NOT_REGISTERED          = 6,
} mac_join_confirm_result_t;

/** Values for the result parameter in MAC_JOIN.response primitive */
typedef enum {
	JOIN_RESPONSE_ACCEPT    = 0,
	JOIN_RESPONSE_REJECT    = 1,
} mac_join_response_answer_t;

/** Values for the result parameter in MAC_LEAVE.confirm primitive */
typedef enum {
	LEAVE_CONFIRM_ACCEPT    = 0,
	LEAVE_CONFIRM_TIMEOUT   = 1,
	LEAVE_CONFIRM_RESULT_PROCCESS_ACTIVE  = 0x80,
	LEAVE_CONFIRM_RESULT_BAD_HANDLER      = 0x81,
	LEAVE_CONFIRM_RESULT_NOT_OPEN_CONN    = 0x82,
	LEAVE_CONFIRM_RESULT_ERROR_SENDING    = 0x83,
	LEAVE_CONFIRM_RESULT_BAD_FLOW_MODE    = 0x84,
} mac_leave_confirm_result_t;

/** Certification test modes prime 1.3 and prime 1.4 */
typedef enum {
	PRIME1_3_FRAME,
	PRIME1_4_TYPE_A_FRAME,
	PRIME1_4_TYPE_B_FRAME,
	PRIME1_4_PHY_BC_FRAME,
} cert_prime_test_frame_t;

/** Certification test parameters */
typedef struct {
	uint16_t us_cert_messg_count;
	uint8_t uc_cert_modulation;
	uint8_t us_cert_signal_att;
	uint8_t us_cert_duty_cycle;
	cert_prime_test_frame_t us_cert_prime_version;
} mlme_certification_parameter_t;

/** Certification modes */
typedef enum {
	NO_CERTIFICATION_MODE      = 0,
	PHY_CERTIFICATION_MODE     = 1,
	MAC_CERTIFICATION_MODE     = 2,
	PHY_CERTIFICATION_MODE_1_4 = 3,
} cert_mode_t;

/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
#endif /* MAC_DEFS_H_INCLUDE */
