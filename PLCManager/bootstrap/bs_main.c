#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <AdpApiTypes.h>
#include <AdpApi.h>

#include "ProtoEapPsk.h"

#include "bs_api.h"
#include "bs_functions.h"
#include "conf_bs.h"

uint8_t uc_nsdu_handle = 0;

int g_lbs_join_finished = 0;
int g_lbs_eap_t3_step_ok = 0;
uint8_t g_lbs_eap_t3_addr[ADP_ADDRESS_64BITS];

uint16_t us_rekey_idx = 0;
uint16_t us_rekey_phase = LBP_REKEYING_PHASE_DISTRIBUTE;

/* Buffer and length for non-bootstrap messages (i.e., KICK, etc...) */
uint8_t g_puc_data[400];
uint16_t g_us_length;

static TBootstrapAdpNotifications ss_notifications;

extern lbds_list_entry_t g_lbds_list[MAX_LBDS];

static pf_app_leave_ind_cb_t pf_app_leave_ind_cb;
static pf_app_join_ind_cb_t pf_app_join_ind_cb;

void _lbp_device_accepted(struct TAdpExtendedAddress *p_au8LBPEUI64Address)
{
	// Activate flag new device about to join.
	g_lbs_eap_t3_step_ok = 1;
	memcpy(g_lbs_eap_t3_addr, p_au8LBPEUI64Address->m_au8Value, ADP_ADDRESS_64BITS);
}

static void _set_keying_table(uint8_t *key)
{
	AdpMacSetRequest(MAC_PIB_KEY_TABLE, 0, 16, key);
}

static void _rekeying_process(void)
{
	struct TAddress dstAddr;
	struct TAdpExtendedAddress x_ext_address;

	memcpy(x_ext_address.m_au8Value, g_lbds_list[us_rekey_idx].puc_extended_address, ADP_ADDRESS_64BITS);

	/* GMK distribution phase */
	if (us_rekey_phase == LBP_REKEYING_PHASE_DISTRIBUTE) {
		/* If re-keying in GMK distribution phase */
		/* Send ADPM-LBP.Request(EAPReq(mes1)) to each registered device */
		Process_Joining0(x_ext_address);
	} else { /* GMK activation phase (LBP_REKEYING_PHASE_ACTIVATE) */
		process_accepted_GMK_activation(x_ext_address);
	}

	/* Send the previously prepared message */
	dstAddr.m_u8AddrLength = 2;
	/* The short address is calculated using the index and the initial short address */
	dstAddr.m_u16ShortAddr = us_rekey_idx + get_initial_short_address();

	// CHRIS REVIEW : AdpGetRequestSync(ADP_IB_MAX_HOPS, 0, &getConfirm);

	AdpLbpRequest((struct TAdpAddress const *)&dstAddr,      /* Destination address */
		      GetLength(),                               /* NSDU length */
		      GetData(),                                 /* NSDU */
		      uc_nsdu_handle++,                          /* NSDU handle */
			  MAX_HOPS, //getConfirm.m_au8AttributeValue[0],         /* Max. Hops */
		      true,                                      /* Discover route */
		      0,                                         /* QoS */
		      false);                                    /* Security enable */
}

static void AdpNotification_LbpIndication(struct TAdpLbpIndication *pLbpIndication)
{
	struct TAddress dstAddr;
	uint16_t u16NsduLength;
	uint8_t *pNsdu;
	enum lbp_indications indication = LBS_NONE;

	u16NsduLength = pLbpIndication->m_u16NsduLength;
	pNsdu = &pLbpIndication->m_pNsdu[0];

	LOG_BOOTSTRAP(Log("[BS] pLbpIndication->m_u16NsduLength %d.", pLbpIndication->m_u16NsduLength));
	LOG_BOOTSTRAP(Log("[BS] TAdpLbpIndication: SrcAddr: 0x%04X LinkQualityIndicator: %u SecurityEnabled: %u NsduLength: %u.",
			  pLbpIndication->m_u16SrcAddr, pLbpIndication->m_u8LinkQualityIndicator,
			  pLbpIndication->m_bSecurityEnabled, pLbpIndication->m_u16NsduLength));

	initialize_bootstrap_message();
	indication = ProcessLBPMessage(pNsdu, u16NsduLength);

	if (GetLength() > 0) {
		if (pLbpIndication->m_u16SrcAddr == 0xFFFF) {
			dstAddr.m_u8AddrLength = 8;
			memcpy(dstAddr.m_u8ExtendedAddr, get_current_ext_address(), 8);
		} else {
			dstAddr.m_u8AddrLength = 2;
			dstAddr.m_u16ShortAddr = pLbpIndication->m_u16SrcAddr;
		}

		// CHRIS REVIEW : AdpGetRequestSync(ADP_IB_MAX_HOPS, 0, &getConfirm);

		AdpLbpRequest((struct TAdpAddress const *)&dstAddr,     /* Destination address */
			      GetLength(),                              /* NSDU length */
			      GetData(),                                /* NSDU */
			      uc_nsdu_handle++,                         /* NSDU handle */
				  MAX_HOPS, //getConfirm.m_au8AttributeValue[0],        /* Max. Hops */
			      true,                                     /* Discover route */
			      0,                                        /* QoS */
			      false);                                   /* Security enable */
	}

	/* Upper layer indications */
	if (indication == LBS_KICK) {
		if (pf_app_leave_ind_cb != NULL) {
			pf_app_leave_ind_cb(pLbpIndication->m_u16SrcAddr, pLbpIndication->m_bSecurityEnabled, pLbpIndication->m_u8LinkQualityIndicator,
                                            pLbpIndication->m_pNsdu, pLbpIndication->m_u16NsduLength);
		}

		/* Remove the device from the joined devices list */
		remove_lbds_list_entry(pLbpIndication->m_u16SrcAddr);
	}
}

static void AdpNotification_LbpConfirm(struct TAdpLbpConfirm *pLbpConfirm)
{
	if (pLbpConfirm->m_u8Status == G3_SUCCESS && g_lbs_eap_t3_step_ok == 1) {
		LOG_BOOTSTRAP(Log("[BS] AdpNotification_LbpConfirm (%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X).",
				  g_lbs_eap_t3_addr[0],g_lbs_eap_t3_addr[1],g_lbs_eap_t3_addr[2],g_lbs_eap_t3_addr[3],
				  g_lbs_eap_t3_addr[4],g_lbs_eap_t3_addr[5],g_lbs_eap_t3_addr[6],g_lbs_eap_t3_addr[7]));

		if (lbp_get_rekeying()) {
			/* If a re-keying phase is in progress */
			if (us_rekey_idx < bs_lbp_get_lbds_counter() - 1) {
				us_rekey_idx++;
			} else {
				if (us_rekey_phase == LBP_REKEYING_PHASE_DISTRIBUTE) {
					/* All devices have been provided with the new GMK -> next phase */
					us_rekey_phase = LBP_REKEYING_PHASE_ACTIVATE;
					/* Reset the index to send GMK activation to all joined devices. */
					us_rekey_idx = 0;
				} else {
					/* End of re-keying process */
					us_rekey_idx = 0;
					lbp_set_rekeying(LBP_REKEYING_OFF);
				}
			}

			_rekeying_process();
		} else {
			uint8_t *puc_ext_addr;
			uint16_t us_short_addr;

			/* Normal bootstrap */
			g_lbs_join_finished = 1;

			if (memcmp(g_lbs_eap_t3_addr, get_current_ext_address(), 8) != 0) {
				LOG_BOOTSTRAP(Log("[BS] registered_cb failed. Different MAC\n"));
				return;
			}

			/* After the join process finishes, the entry is added to the list */
			puc_ext_addr = get_current_ext_address()->m_au8Value;
			us_short_addr = get_current_short_address();
			add_lbds_list_entry(puc_ext_addr, us_short_addr);

			/* Upper layer indications */
			if (pf_app_join_ind_cb != NULL) {
				pf_app_join_ind_cb(puc_ext_addr, us_short_addr);
			}
		}
	}

	g_lbs_eap_t3_step_ok = 0;
}

/**
 * bs_init.
 *
 */
void bs_init(void)
{
	lbp_init_functions();

	_set_keying_table((uint8_t *)GetGMK());

	/* Init function pointers */
	pf_app_leave_ind_cb = NULL;
	pf_app_join_ind_cb = NULL;
}

/**
 * bs_get_not_handlers.
 *
 */
TBootstrapAdpNotifications* bs_get_not_handlers(void)
{
	/* Set Notification pointers */
	ss_notifications.fnctAdpLbpConfirm = AdpNotification_LbpConfirm;
	ss_notifications.fnctAdpLbpIndication = AdpNotification_LbpIndication;

	return &ss_notifications;
}

/**
 * bs_lbp_launch_rekeying.
 *
 */
void bs_lbp_launch_rekeying(void)
{
	/* If there are devices that joined the network */
	if (bs_lbp_get_lbds_counter() > 0) {
		/* Start the re-keying process */
		lbp_set_rekeying(LBP_REKEYING_ON);
		initialize_bootstrap_message();
		us_rekey_phase = LBP_REKEYING_PHASE_DISTRIBUTE;
		/* Send the first re-keying process */
		us_rekey_idx = 0;
		_rekeying_process();
		LOG_BOOTSTRAP(Log("[BS] Re-keying launched."));
	} else {
		// Error: no device in the network
		LOG_BOOTSTRAP(Log("[BS] Re-keying NOT launched: no device in the network."));
	}
}

/**
 * bs_lbp_kick_device.
 *
 */
void bs_lbp_kick_device(uint16_t us_short_address)
{
	/* Check if the device had joined the network */
	if(device_is_in_list(us_short_address)) {
		struct TAdpAddress dstAddr;
		uint8_t puc_extended_address[ADP_ADDRESS_64BITS];

		/* Send KICK to the device */
		dstAddr.m_u8AddrSize = 2;
		dstAddr.m_u16ShortAddr = us_short_address;

		get_ext_addr_by_short(us_short_address, puc_extended_address);

		g_us_length = Encode_kick_to_LBD(puc_extended_address, sizeof(g_puc_data), g_puc_data);

		/* If message was properly encoded, send it */
		if (g_us_length) {
			// CHRIS REVIEW : AdpGetRequestSync(ADP_IB_MAX_HOPS, 0, &getConfirm);
			AdpLbpRequest((struct TAdpAddress const *)&dstAddr,     /* Destination address */
				      g_us_length,                              /* NSDU length */
				      g_puc_data,                               /* NSDU */
				      uc_nsdu_handle++,                         /* NSDU handle */
					  MAX_HOPS, //getConfirm.m_au8AttributeValue[0],        /* Max. Hops */
				      true,                                     /* Discover route */
				      0,                                        /* QoS */
				      false);                                   /* Security enable */

			/* Remove the device from the joined devices list */
			remove_lbds_list_entry(us_short_address);
		} else {
			LOG_BOOTSTRAP(Log("[BS] Error encoding KICK_TO_LBD.\n"));
		}
	} else {
		LOG_BOOTSTRAP(Log("[BS] Error: attempted KICK of not joined device [0x%04x]\n", us_short_address));
	}
}

/**
 * bs_lbp_get_lbds_counter.
 *
 */
uint16_t bs_lbp_get_lbds_counter(void)
{
	return get_lbds_count();
}

/**
 * bs_lbp_get_lbds_address.
 *
 */
uint16_t bs_lbp_get_lbds_address(uint16_t i)
{
	return get_lbd_address(i);
}

/**
 * bs_lbp_get_lbds_ex_address.
 *
 */
bool bs_lbp_get_lbds_ex_address(uint16_t us_short_address, uint8_t *puc_extended_address)
{
	return get_ext_addr_by_short(us_short_address, puc_extended_address);
}

/**
 * bs_lbp_get_param.
 *
 */
void bs_lbp_get_param(uint32_t ul_attribute_id, uint16_t us_attribute_idx, struct t_bs_lbp_get_param_confirm *p_get_confirm)
{
	p_get_confirm->ul_attribute_id = ul_attribute_id;
	p_get_confirm->us_attribute_idx = us_attribute_idx;
	p_get_confirm->uc_attribute_length = 0;
	p_get_confirm->uc_status = LBP_STATUS_UNSUPPORTED_PARAMETER;

	if (ul_attribute_id == LBP_IB_DEVICE_LIST) {
		if (us_attribute_idx < MAX_LBDS) {
			uint16_t us_short_address = us_attribute_idx + get_initial_short_address();
			/* Check valid entry */
			if(device_is_in_list(us_short_address)) {
				p_get_confirm->uc_status = LBP_STATUS_OK;
				p_get_confirm->uc_attribute_length = 10;
				p_get_confirm->uc_attribute_value[0] = (uint8_t)(us_short_address & 0x00FF);
				p_get_confirm->uc_attribute_value[1] = (uint8_t)(us_short_address >> 8);
				memcpy(&p_get_confirm->uc_attribute_value[2], g_lbds_list[us_attribute_idx].puc_extended_address, ADP_ADDRESS_64BITS);
			} else {
				/* Empty list entry */
				LOG_BOOTSTRAP(Log("[BS] LBDs entry not valid for index %u", us_attribute_idx));
				p_get_confirm->uc_status = LBP_STATUS_INVALID_VALUE;
				memset(&p_get_confirm->uc_attribute_value[0], 0, 10);
			}
		} else {
			/* Invalid list index */
			LOG_BOOTSTRAP(Log("[BS] Index %u out of range of LBDs list", us_attribute_idx));
			p_get_confirm->uc_status = LBP_STATUS_INVALID_INDEX;
			memset(&p_get_confirm->uc_attribute_value[0], 0, 10);
		}
	} else if (ul_attribute_id == LBP_IB_INITIAL_SHORT_ADDRESS) {
		uint16_t us_initial_short_address = get_initial_short_address();
		p_get_confirm->uc_status = LBP_STATUS_OK;
		p_get_confirm->uc_attribute_length = 2;
		p_get_confirm->uc_attribute_value[0] = (uint8_t)(us_initial_short_address >> 8);
		p_get_confirm->uc_attribute_value[1] = (uint8_t)(us_initial_short_address & 0x00FF);
	} else {
		/* Unknown LBS parameter */
	}
}

/**
 * bs_lbp_set_param.
 *
 */
void bs_lbp_set_param(uint32_t ul_attribute_id, uint16_t us_attribute_idx, uint8_t  uc_attribute_len, const uint8_t *puc_attribute_value,
		      struct t_bs_lbp_set_param_confirm *p_set_confirm)
{
	uint16_t us_tmp_short_addr;

	p_set_confirm->ul_attribute_id = ul_attribute_id;
	p_set_confirm->us_attribute_idx = us_attribute_idx;
	p_set_confirm->uc_status = LBP_STATUS_UNSUPPORTED_PARAMETER;

	switch (ul_attribute_id)
	{
		case LBP_IB_IDS:
			if ((uc_attribute_len == NETWORK_ACCESS_IDENTIFIER_SIZE_S_ARIB) || (uc_attribute_len == NETWORK_ACCESS_IDENTIFIER_SIZE_S_CENELEC_FCC)) {
				set_ids(puc_attribute_value, uc_attribute_len);
				p_set_confirm->uc_status = LBP_STATUS_OK;
			} else {
				p_set_confirm->uc_status = LBP_STATUS_INVALID_LENGTH;
			}
			break;

		case LBP_IB_DEVICE_LIST:
			if (uc_attribute_len == 10) {
				us_tmp_short_addr = ((puc_attribute_value[1] << 8) | puc_attribute_value[0]);
				/* Both short and extended addresses are specified */
				if (add_lbds_list_entry(&puc_attribute_value[2], us_tmp_short_addr)) {
					p_set_confirm->uc_status = LBP_STATUS_OK;
				} else {
					LOG_BOOTSTRAP(Log("[BS] Address: 0x%04x already in use.\n", us_tmp_short_addr));
					p_set_confirm->uc_status = LBP_STATUS_INVALID_VALUE;
				}
			} else {
				/* Wrong parameter size */
				p_set_confirm->uc_status = LBP_STATUS_INVALID_LENGTH;
			}
			break;

		case LBP_IB_INITIAL_SHORT_ADDRESS:
			if (uc_attribute_len == 2) {
				if (get_lbds_count() > 0) {
					// If there are active LBDs, the initial short address cannot be changed
					p_set_confirm->uc_status = LBP_STATUS_NOK;
				} else {
					us_tmp_short_addr = ((puc_attribute_value[1] << 8) | puc_attribute_value[0]);

					if (set_initial_short_address(us_tmp_short_addr)) {
						p_set_confirm->uc_status = LBP_STATUS_OK;
					} else {
						p_set_confirm->uc_status = LBP_STATUS_INVALID_VALUE;
					}
				}
			} else {
				/* Wrong parameter size */
				p_set_confirm->uc_status = LBP_STATUS_INVALID_LENGTH;
			}
			break;

		case LBP_IB_ADD_DEVICE_TO_BLACKLIST:
			if (uc_attribute_len == 8) {
				add_to_blacklist((uint8_t *)puc_attribute_value);
				p_set_confirm->uc_status = LBP_STATUS_OK;
			} else {
				/* Wrong parameter size */
				p_set_confirm->uc_status = LBP_STATUS_INVALID_LENGTH;
			}
			break;

		case LBP_IB_PSK:
			if (uc_attribute_len == 16) {
				set_psk((uint8_t *)puc_attribute_value);
				p_set_confirm->uc_status = LBP_STATUS_OK;
			} else {
				/* Wrong parameter size */
				p_set_confirm->uc_status = LBP_STATUS_INVALID_LENGTH;
			}
			break;

		case LBP_IB_GMK:
			if (uc_attribute_len == 16) {
				set_gmk((uint8_t *)puc_attribute_value);
				p_set_confirm->uc_status = LBP_STATUS_OK;
			} else {
				/* Wrong parameter size */
				p_set_confirm->uc_status = LBP_STATUS_INVALID_LENGTH;
			}
			break;

		case LBP_IB_REKEY_GMK:
			if(uc_attribute_len == 16) {
				set_rekey_gmk((uint8_t *)puc_attribute_value);
				p_set_confirm->uc_status = LBP_STATUS_OK;
			} else {
				/* Wrong parameter size */
				p_set_confirm->uc_status = LBP_STATUS_INVALID_LENGTH;
			}
			break;

		default:
			/* Unknown LBS parameter */
			break;
	}
}

/**
 * bs_lbp_leave_ind_set_cb.
 *
 */
void bs_lbp_leave_ind_set_cb(pf_app_leave_ind_cb_t pf_handler)
{
	pf_app_leave_ind_cb = pf_handler;
}

/**
 * bs_lbp_join_ind_set_cb.
 *
 */
void bs_lbp_join_ind_set_cb(pf_app_join_ind_cb_t pf_handler)
{
	pf_app_join_ind_cb = pf_handler;
}
