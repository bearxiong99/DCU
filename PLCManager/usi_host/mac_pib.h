/**
 * \file
 *
 * \brief MAC_PIB: PRIME MAC information base
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

#ifndef MAC_PIB_H_INCLUDE
#define MAC_PIB_H_INCLUDE

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
 * \weakgroup prime_mac_group
 * @{
 */

/** \brief PHY statistical PIB attributes */
/* @{ */
#define PIB_PHY_STATS_CRC_INCORRECT             0x00A0
#define PIB_PHY_STATS_CRC_FAIL_COUNT            0x00A1
#define PIB_PHY_STATS_TX_DROP_COUNT             0x00A2
#define PIB_PHY_STATS_RX_DROP_COUNT             0x00A3
#define PIB_PHY_STATS_RX_TOTAL_COUNT            0x00A4
#define PIB_PHY_STATS_BLK_AVG_EVM               0x00A5
#define PIB_PHY_EMA_SMOOTHING                   0x00A8
/* @} */

/** \brief PHY implementation PIB attributes */
/* @{ */
#define PIB_PHY_TX_QUEUE_LEN                    0x00B0
#define PIB_PHY_RX_QUEUE_LEN                    0x00B1
#define PIB_PHY_TX_PROCESSING_DELAY             0x00B2
#define PIB_PHY_RX_PROCESSING_DELAY             0x00B3
#define PIB_PHY_AGC_MIN_GAIN                    0x00B4
#define PIB_PHY_AGC_STEP_VALUE                  0x00B5
#define PIB_PHY_AGC_STEP_NUMBER                 0x00B6
/* @} */

/** \brief MAC variable PIB attributes */
/* @{ */
#define PIB_MAC_MIN_SWITCH_SEARCH_TIME          0x0010
#define PIB_MAC_MAX_PROMOTION_PDU               0x0011
#define PIB_MAC_PROMOTION_PDU_TX_PERIOD         0x0012
#define PIB_MAC_BEACONS_PER_FRAME               0x0013
#define PIB_MAC_SCP_MAX_TX_ATTEMPTS             0x0014
#define PIB_MAC_CTL_RE_TX_TIMER                 0x0015
#define PIB_MAC_SCP_RBO                         0x0016
#define PIB_MAC_SCP_CH_SENSE_COUNT              0x0017
#define PIB_MAC_MAX_CTL_RE_TX                   0x0018
#define PIB_MAC_EMA_SMOOTHING                   0x0019
/* @} */

/** \brief MAC functional PIB attributes */
/* @{ */
#define PIB_MAC_LNID                            0x0020
#define PIB_MAC_LSID                            0x0021
#define PIB_MAC_SID                             0x0022
#define PIB_MAC_SNA                             0x0023
#define PIB_MAC_STATE                           0x0024
#define PIB_MAC_SCP_LENGTH                      0x0025
#define PIB_MAC_NODE_HIERARCHY_LEVEL            0x0026
#define PIB_MAC_BEACON_SLOT_COUNT               0x0027
#define PIB_MAC_BEACON_RX_SLOT                  0x0028
#define PIB_MAC_BEACON_TX_SLOT                  0x0029
#define PIB_MAC_BEACON_RX_FRECUENCY             0x002A
#define PIB_MAC_BEACON_TX_FRECUENCY             0x002B
#define PIB_MAC_MAC_CAPABILITES                 0x002C
/* @} */

/** \brief MAC statistical PIB attributes */
/* @{ */
#define PIB_MAC_TX_DATAPKT_COUNT                0x0040
#define PIB_MAC_RX_DATAPKT_COUNT                0x0041
#define PIB_MAC_TX_CTRLPKT_COUNT                0x0042
#define PIB_MAC_RX_CTRLPKT_COUNT                0x0043
#define PIB_MAC_CSMA_FAIL_COUNT                 0x0044
#define PIB_MAC_CSMA_CH_BUSY_COUNT              0x0045
/* @} */

/** \brief MAC list PIB attributes */
/* @{ */
#define PIB_MAC_LIST_REGISTER_DEVICES           0x0050
#define PIB_MAC_LIST_ACTIVE_CONN                0x0051
#define PIB_MAC_LIST_MCAST_ENTRIES              0x0052
#define PIB_MAC_LIST_SWITCH_TABLE               0x0053
#define PIB_MAC_LIST_DIRECT_CONN                0x0054
#define PIB_MAC_LIST_DIRECT_TABLE               0x0055
#define PIB_MAC_LIST_AVAILABLE_SWITCHES         0x0056
#define PIB_MAC_LIST_PHY_COMM                   0x0057
/* @} */

/** \brief MAC action PIB attributes */
/* @{ */
#define PIB_MAC_ACTION_TX_DATA                  0x0060
#define PIB_MAC_ACTION_CONN_CLOSE               0x0061
#define PIB_MAC_ACTION_REG_REJECT               0x0062
#define PIB_MAC_ACTION_PRO_REJECT               0x0063
#define PIB_MAC_ACTION_UNREGISTER               0x0064
//P1_4
#define PIB_MAC_ACTION_PROMOTE                  0x0065
#define PIB_MAC_ACTION_DEMOTE                   0x0066
#define PIB_MAC_ACTION_REJECT                   0x0067
#define PIB_MAC_ACTION_ALIVETIME                0x0068
#define PIB_MAC_ACTION_BROADCASTDATABURST       0x006A
#define PIB_MAC_ACTION_MGMT_CON                 0x006B
#define PIB_MAC_ACTION_MGMT_MUL                 0x006C
#define PIB_MAC_ACTION_UNREGISTERBN             0x006D
#define PIB_MAC_ACTION_CONNCLOSEBN              0x006E
#define PIB_MAC_ACTION_SEGMENTED_432            0x006F
#define PIB_MAC_ACTION_APPEMU_DATABURST         0x0080
#define PIB_MAC_ACTION_MGMT_DATABURST           0x0081
/* @} */

/** \brief Management Plane firmware upgrade PIB attributes */
/* @{ */
#define PIB_FU_APP_FWDL_RUNNING                 0x0070
#define PIB_FU_APP_FWDL_RX_PKT_COUNT            0x0071
/* @} */

/** \brief MAC application PIB attributes */
/* @{ */
#define PIB_MAC_APP_FW_VERSION                  0x0075
#define PIB_MAC_APP_VENDOR_ID                   0x0076
#define PIB_MAC_APP_PRODUCT_ID                  0x0077
/* @} */

/** \brief MAC certification PIB attributes */
/* @{ */
#define PIB_CERTIFICATION_MODE                  0x8120
#define PIB_CERTIFICATION_SEND_MSG              0x8121
/* @} */

/** \brief MAC manufacturing test process (MTP) PIB attributes */
/* @{ */
#define PIB_MTP_PHY_CFG_LOAD_THRESHOLD_1        0x8003
#define PIB_MTP_PHY_CFG_LOAD_THRESHOLD_2        0x8004
#define PIB_MTP_PHY_TX_TIME                     0x8085
#define PIB_MTP_PHY_RMS_CALC_CORRECTED          0x8086
#define PIB_MTP_PHY_EXECUTE_CALIBRATION         0x8087
#define PIB_MTP_PHY_RX_PARAMS                   0x8088
#define PIB_MTP_PHY_TX_PARAMS                   0x8089
#define PIB_PHY_TX_CHANNEL                      0x8090
#define PIB_MTP_PHY_ENABLE                      0x808E
#define PIB_MTP_PHY_DRV_AUTO                    0x8301
#define PIB_MTP_PHY_DRV_IMPEDANCE               0x8302
#define PIB_MTP_MAC_WRITE_SNA                   0x8123
#define PIB_ATML_432_NODE_LIST                  0x8250
/* @} */

/** \brief Other vendor specific PIB attributes */
/* @{ */
#define PIB_PHY_SW_VERSION                      0x8080
#define PIB_MAC_EUI_48                          0x8100
#define PIB_PHY_SNIFFER_ENABLED                 0x8106
#define PIB_MAC_INTERNAL_SW_VERSION             0x8126
#define PIB_432_INTERNAL_SW_VERSION             0x8201


/** \brief PRIME 1_4 PIB attributes */
#define PIB14_MAC_VERSION                       0x0001

#define PIB14_MAC_LIST_REG_DEVICES_TABLE        0x0050
#define PIB14_MAC_LIST_ACTIVE_CONN_TABLE        0x0051
#define PIB14_MAC_LIST_MCAST_ENTRIES_TABLE      0x0052
#define PIB14_MAC_LIST_DIRECT_CONN_TABLE        0x0054
#define PIB14_MAC_LIST_DIRECT_TABLE             0x0055
#define PIB14_MAC_LIST_AVAILABLE_SWITCHES_TABLE 0x0056
#define PIB14_MAC_LIST_ACTIVE_CONN_EX_TABLE     0x0058
#define PIB14_MAC_LIST_PHY_COMM_TABLE           0x0059
#define PIB14_MAC_LIST_SWITCH_TABLE             0x005A
#define PIB14_MAC_LIST_APP_LIST_ZC_STATUS_TABLE 0x0078
#define PIB14_ATML_432_NODE_LIST                0x8250

/* @} */

/* @} */

/* @cond 0 */
/**INDENT-OFF**/
#ifdef __cplusplus
}
#endif
/**INDENT-ON**/
/* @endcond */
#endif /* MAC_PIB_H_INCLUDE */
