#include <string.h>

#include "ifacePrime_api.h"
#include "prime_api_host.h"
#include "prime_api_defs_host.h"
#include "hal_utils.h"
#include "cl_432_defs.h"

/* Pointers to callback functions to be establish -> plcmanager.h : PLC_MNG_MAX_APP_ID = 8 */
#define PRIME_API_MAX_NUM           8
#define PRIME_API_INVALID           0xFF
static prime_api_cbs_t sx_prime_api[PRIME_API_MAX_NUM];
static uint8_t suc_prime_app_id_req;

/* buffer used to serialization */
static uint8_t uc_serial_buf[MAX_LENGTH_432_DATA];

/* USI Cmd */
x_usi_cmd_t prime_api_msg;

static cl_null_data_pointer sx_data_pointer_table[NUM_ELEMENTS_POINTER_TABLE];

/**
 * \brief CL NULL Set callback functions
 * \param px_prime_cbs  Pointer to mac callbacks struct
 */
void prime_cl_null_set_callbacks(uint8_t prime_app_id, prime_cl_null_callbacks_t *px_prime_cbs)
{
	prime_api_cbs_t *px_prime_api;

	if (prime_app_id < PRIME_API_MAX_NUM) {
		px_prime_api = &sx_prime_api[prime_app_id];
		px_prime_api->prime_cl_null_establish_ind_cb = px_prime_cbs->prime_cl_null_establish_ind_cb;
		px_prime_api->prime_cl_null_establish_cfm_cb = px_prime_cbs->prime_cl_null_establish_cfm_cb;
		px_prime_api->prime_cl_null_release_ind_cb = px_prime_cbs->prime_cl_null_release_ind_cb;
		px_prime_api->prime_cl_null_release_cfm_cb = px_prime_cbs->prime_cl_null_release_cfm_cb;
		px_prime_api->prime_cl_null_join_ind_cb = px_prime_cbs->prime_cl_null_join_ind_cb;
		px_prime_api->prime_cl_null_join_cfm_cb = px_prime_cbs->prime_cl_null_join_cfm_cb;
		px_prime_api->prime_cl_null_leave_cfm_cb = px_prime_cbs->prime_cl_null_leave_cfm_cb;
		px_prime_api->prime_cl_null_leave_ind_cb = px_prime_cbs->prime_cl_null_leave_ind_cb;
		px_prime_api->prime_cl_null_data_cfm_cb = px_prime_cbs->prime_cl_null_data_cfm_cb;
		px_prime_api->prime_cl_null_data_ind_cb = px_prime_cbs->prime_cl_null_data_ind_cb;
		px_prime_api->prime_cl_null_plme_reset_cfm_cb = px_prime_cbs->prime_cl_null_plme_reset_cfm_cb;
		px_prime_api->prime_cl_null_plme_sleep_cfm_cb = px_prime_cbs->prime_cl_null_plme_sleep_cfm_cb;
		px_prime_api->prime_cl_null_plme_resume_cfm_cb = px_prime_cbs->prime_cl_null_plme_resume_cfm_cb;
		px_prime_api->prime_cl_null_plme_testmode_cfm_cb = px_prime_cbs->prime_cl_null_plme_testmode_cfm_cb;
		px_prime_api->prime_cl_null_plme_get_cfm_cb = px_prime_cbs->prime_cl_null_plme_get_cfm_cb;
		px_prime_api->prime_cl_null_plme_set_cfm_cb = px_prime_cbs->prime_cl_null_plme_set_cfm_cb;
		px_prime_api->prime_cl_null_mlme_register_cfm_cb = px_prime_cbs->prime_cl_null_mlme_register_cfm_cb;
		px_prime_api->prime_cl_null_mlme_register_ind_cb = px_prime_cbs->prime_cl_null_mlme_register_ind_cb;
		px_prime_api->prime_cl_null_mlme_unregister_cfm_cb = px_prime_cbs->prime_cl_null_mlme_unregister_cfm_cb;
		px_prime_api->prime_cl_null_mlme_unregister_ind_cb = px_prime_cbs->prime_cl_null_mlme_unregister_ind_cb;
		px_prime_api->prime_cl_null_mlme_promote_cfm_cb = px_prime_cbs->prime_cl_null_mlme_promote_cfm_cb;
		px_prime_api->prime_cl_null_mlme_promote_ind_cb = px_prime_cbs->prime_cl_null_mlme_promote_ind_cb;
		px_prime_api->prime_cl_null_mlme_demote_cfm_cb = px_prime_cbs->prime_cl_null_mlme_demote_cfm_cb;
		px_prime_api->prime_cl_null_mlme_demote_ind_cb = px_prime_cbs->prime_cl_null_mlme_demote_ind_cb;
		px_prime_api->prime_cl_null_mlme_reset_cfm_cb = px_prime_cbs->prime_cl_null_mlme_reset_cfm_cb;
		px_prime_api->prime_cl_null_mlme_get_cfm_cb = px_prime_cbs->prime_cl_null_mlme_get_cfm_cb;
		px_prime_api->prime_cl_null_mlme_list_get_cfm_cb = px_prime_cbs->prime_cl_null_mlme_list_get_cfm_cb;
		px_prime_api->prime_cl_null_mlme_set_cfm_cb = px_prime_cbs->prime_cl_null_mlme_set_cfm_cb;
	}
}


/*
 * \brief Request a connection establishment
 *
 * \param puc_eui48     Address of the node to which this connection will be addressed
 * \param uc_type       Convergence Layer type of the connection
 * \param puc_data      Data associated with the connection establishment procedure
 * \param us_data_len   Length of the data in bytes
 * \param uc_arq        Flag to indicate whether or not the ARQ mechanism should be used for this connection
 * \param uc_cfbytes    Flag to indicate whether or not the connection should use the contention or contention-free channel access scheme
 *
 */
void prime_cl_null_establish_request(uint8_t *puc_eui48, uint8_t uc_type, uint8_t *puc_data, uint16_t us_data_len, uint8_t uc_arq, uint8_t uc_cfbytes)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_establish_request_cmd;
    memcpy(puc_msg, puc_eui48, LEN_ID_EUI48);
    puc_msg += LEN_ID_EUI48;
    *puc_msg++ = uc_type;
    *puc_msg++ = (uint8_t)(us_data_len >> 8);
    *puc_msg++ = (uint8_t)(us_data_len & 0xFF);
    memcpy(puc_msg,puc_data,us_data_len);
    puc_msg += us_data_len;
    *puc_msg++ = uc_arq;
    *puc_msg++ = uc_cfbytes;

    /* Send to USI */
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief Response to a connection establishment indication
 *
 * \param us_con_handle Uniquely identifier of the connection
 * \param uc_answer     Action to be taken for this connection establishment
 * \param puc_data      Data associated with the connection establishment procedure
 * \param us_data_len   Length of the data in bytes
 *
 */
void prime_cl_null_establish_response(uint16_t us_con_handle, mac_establish_response_answer_t uc_answer, uint8_t *puc_data, uint16_t us_data_len)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_establish_response_cmd;
    *puc_msg++ = (uint8_t)(us_con_handle >> 8);
    *puc_msg++ = (uint8_t)(us_con_handle & 0xFF);
    *puc_msg++ = (uint8_t)uc_answer;
    *puc_msg++ = (uint8_t)(us_data_len >> 8);
    *puc_msg++ = (uint8_t)(us_data_len & 0xFF);
    memcpy(puc_msg,puc_data,us_data_len);
    puc_msg += us_data_len;

    /* Send to USI */
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief Response to a connection establishment indication
 *
 * \param us_con_handle Uniquely identifier of the connection

 * \param puc_data      Data associated with the connection establishment procedure
 * \param us_data_len   Length of the data in bytes
 *
 */
void prime_cl_null_redirect_response(uint16_t us_con_handle, uint8_t *puc_eui48, uint8_t *puc_data, uint16_t us_data_len)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_redirect_response_cmd;
    *puc_msg++ = (uint8_t)(us_con_handle >> 8);
    *puc_msg++ = (uint8_t)(us_con_handle & 0xFF);
    memcpy(puc_msg,puc_eui48,6);
    puc_msg += 6;
    *puc_msg++ = (uint8_t)(us_data_len >> 8);
    *puc_msg++ = (uint8_t)(us_data_len & 0xFF);
    memcpy(puc_msg,puc_data,us_data_len);
    puc_msg += us_data_len;

    /* Send to USI */
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}



/**
 * \brief Initiate the release process of a connection
 *
 * \param us_con_handle Uniquely identifier of the connection
 *
 */
void prime_cl_null_release_request(uint16_t us_con_handle)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_release_request_cmd;
    *puc_msg++ = (uint8_t)(us_con_handle >> 8);
    *puc_msg++ = (uint8_t)(us_con_handle & 0xFF);

    /* Send to USI */
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief Response to a connection release process
 *
 * \param us_con_handle Uniquely identifier of the connection
 * \param uc_answer     Action to be taken for this connection release procesudre
 *
 */
void prime_cl_null_release_response(uint16_t us_con_handle, mac_release_response_answer_t uc_answer)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_release_response_cmd;
    *puc_msg++ = (uint8_t)(us_con_handle >> 8);
    *puc_msg++ = (uint8_t)(us_con_handle & 0xFF);
    *puc_msg++ = (uint8_t)uc_answer;

    /* Send to USI */
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * MAC Join Request
 * - us_broadcast:      Join type (broadcast or multicast connection)
 * - us_con_handle:     Unique identifier of the connection (only used for base node)
 * - puc_eui48:         Address of the node to which this join is being requested (only used for base node)
 * - uc_con_type:       Connection type
 * - puc_data:          Data associated with the join request procedure
 * - us_data_len:       Length of the data in bytesn
 */
void prime_cl_null_join_request(mac_join_mode_t uc_broadcast, uint16_t us_conn_handle, uint8_t *puc_eui48, connection_type_t uc_con_type, uint8_t *puc_data, uint16_t us_data_len)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_join_request_cmd;
    *puc_msg++ = (uint8_t) uc_broadcast;
    *puc_msg++ = (uint8_t)(us_conn_handle >> 8);
    *puc_msg++ = (uint8_t)(us_conn_handle & 0xFF);
    memcpy(puc_msg,puc_eui48,6);
    puc_msg += 6;
    *puc_msg++ = (uint8_t) uc_con_type;
    *puc_msg++ = (uint8_t)(us_data_len >> 8);
    *puc_msg++ = (uint8_t)(us_data_len & 0xFF);
    memcpy(puc_msg,puc_data,us_data_len);
    puc_msg += us_data_len;

    /* Send to USI */
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * MAC Join Response
 * - us_con_handle:        Unique identifier of the connection
 * - puc_eui48:            Address of the node which requested the multicast group join (only used for base node)
 * - uc_answer:            Action to be taken for this join request
 */
void prime_cl_null_join_response(uint16_t us_con_handle, uint8_t *puc_eui48, mac_join_response_answer_t uc_answer)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_join_response_cmd;
    *puc_msg++ = (uint8_t)(us_con_handle >> 8);
    *puc_msg++ = (uint8_t)(us_con_handle & 0xFF);
    if(puc_eui48){
        *puc_msg++ = puc_eui48[0];
        *puc_msg++ = puc_eui48[1];
        *puc_msg++ = puc_eui48[2];
        *puc_msg++ = puc_eui48[3];
        *puc_msg++ = puc_eui48[4];
        *puc_msg++ = puc_eui48[5];
    } else {
        *puc_msg++ = 0;
        *puc_msg++ = 0;
        *puc_msg++ = 0;
        *puc_msg++ = 0;
        *puc_msg++ = 0;
        *puc_msg++ = 0;
    }

    *puc_msg++ = (uint8_t)uc_answer;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * MAC Leave Request
 * - us_con_handle:        Unique identifier of the connection
 * - puc_eui48:            Address of the node to be removed from the multicast group (only used for base node)
 */
void prime_cl_null_leave_request(uint16_t us_con_handle, uint8_t *puc_eui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_leave_request_cmd;
    *puc_msg++ = (uint8_t)(us_con_handle >> 8);
    *puc_msg++ = (uint8_t)(us_con_handle & 0xFF);
    memcpy(puc_msg,puc_eui48,6);
    puc_msg += 6;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief Initiate the transmission process of data over a connection
 *
 * \param us_con_handle    Specifies the connection to be used for the data transmission
 * \param puc_data         Pointer to data to be transmitted through this connection
 * \param us_data_len      Length of the data in bytes
 * \param uc_prio          Priority of the data to be sent when using the CSMA access scheme
 */
void prime_cl_null_data_request(uint16_t us_con_handle, uint8_t *puc_data, uint16_t us_data_len, uint8_t uc_prio)
{
    uint8_t *puc_msg;
    int index;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_data_request_cmd;
    *puc_msg++ = (uint8_t)(us_con_handle >> 8);
    *puc_msg++ = (uint8_t)(us_con_handle & 0xFF);
    *puc_msg++ = (uint8_t)(us_data_len >> 8);
    *puc_msg++ = (uint8_t)(us_data_len & 0xFF);
    memcpy(puc_msg,puc_data,us_data_len);
    puc_msg += us_data_len;
    *puc_msg++ = uc_prio;


    /* Find a free element of table */
    for (index = 0; index < NUM_ELEMENTS_POINTER_TABLE; index++)
    {
        if (!sx_data_pointer_table[index].valid)
        {
            /* Store connection handlder and data pointer in table */
        	sx_data_pointer_table[index].con_handle = us_con_handle;
        	sx_data_pointer_table[index].puc_data = puc_data;
        	sx_data_pointer_table[index].valid = true;
            break;
        }
    }

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief PLME reset request
 *
 */
void prime_cl_null_plme_reset_request(void)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_plme_reset_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;

    hal_usi_send_cmd(&prime_api_msg);
}



/**
 * \brief PLME sleep request
 *
 */
void prime_cl_null_plme_sleep_request(void)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_plme_sleep_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief PLME resumr request
 *
 */
void prime_cl_null_plme_resume_request(void)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_plme_resume_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief PLME testmode request
 *
 * \param uc_enable         Start/Stop test mode
 * \param uc_mode           Transmission mode
 * \param uc_modulation     Transmission modulation
 * \param uc_pwr_level      Transmission power level
 *
 */
void prime_cl_null_plme_testmode_request(uint8_t uc_enable, uint8_t uc_mode, uint8_t uc_modulation, uint8_t uc_pwr_level)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_plme_testmode_request_cmd;
    *puc_msg++ = uc_enable;
    *puc_msg++ = uc_mode;
    *puc_msg++ = uc_modulation;
    *puc_msg++ = uc_pwr_level;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief PLME get request
 *
 * \param us_pib_attrib      PIB attribute
 *
 */
void prime_cl_null_plme_get_request(uint16_t us_pib_attrib)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_plme_get_request_cmd;
    *puc_msg++ = (uint8_t)(us_pib_attrib >> 8);
    *puc_msg++ = (uint8_t)(us_pib_attrib & 0xFF);

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}



/**
 * \brief PLME set request
 *
 * \param us_pib_attrib      PIB attribute
 * \param pv_pib_value       PIB attribute value
 * \param uc_pib_size        PIB attribute value size
 */
void prime_cl_null_plme_set_request(uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_plme_set_request_cmd;
    *puc_msg++ = (uint8_t)(us_pib_attrib >> 8);
    *puc_msg++ = (uint8_t)(us_pib_attrib & 0xFF);
    *puc_msg++ = uc_pib_size;
    memcpy(puc_msg,(uint8_t *)pv_pib_value,uc_pib_size);
    puc_msg += uc_pib_size;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}



/**
 * \brief MLME register request
 *
 * \param puc_sna      Pointer to SNA
 * \param uc_sid       Switch Identifier
 *
 */
void prime_cl_null_mlme_register_request(uint8_t *puc_sna, uint8_t uc_sid)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_register_request_cmd;
    if (puc_sna == NULL)
    {
        memset(puc_msg, 0xFF, LEN_ID_EUI48);
    }
    else
    {
        memcpy(puc_msg, puc_sna, LEN_ID_EUI48);
    }

    puc_msg += LEN_ID_EUI48;
    *puc_msg++ = uc_sid;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief MLME unregister request
 *
 */
void prime_cl_null_mlme_unregister_request(void)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_unregister_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void prime_cl_null_mlme_unregister_request_base(unsigned char * _pucEui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_unregister_request_cmd;
    memcpy(puc_msg,(uint8_t *)_pucEui48,LEN_ID_EUI48);
    puc_msg += LEN_ID_EUI48;
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief MLME promote request
 *
 */
void prime_cl_null_mlme_promote_request(void)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_promote_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief MLME demote request
 *
 */
void prime_cl_null_mlme_demote_request(void)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_demote_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief MLME reset request
 *
 */
void prime_cl_null_mlme_reset_request(void)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_reset_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief MLME get request
 *
 * \param us_pib_attrib      PIB attribute
 *
 */
void prime_cl_null_mlme_get_request(uint16_t us_pib_attrib)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_get_request_cmd;
    *puc_msg++ = (uint8_t)(us_pib_attrib >> 8);
    *puc_msg++ = (uint8_t)(us_pib_attrib & 0xFF);

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief MLME list get request
 *
 * \param us_pib_attrib      PIB attribute
 *
 */
void prime_cl_null_mlme_list_get_request(uint16_t us_pib_attrib)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_list_get_request_cmd;
    *puc_msg++ = (uint8_t)(us_pib_attrib >> 8);
    *puc_msg++ = (uint8_t)(us_pib_attrib & 0xFF);

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief MLME set request
 *
 * \param us_pib_attrib      PIB attribute
 * \param pv_pib_value       PIB attribute value
 * \param uc_pib_size        PIB attribute value size
 */
void prime_cl_null_mlme_set_request(uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_set_request_cmd;
    *puc_msg++ = (uint8_t)(us_pib_attrib >> 8);
    *puc_msg++ = (uint8_t)(us_pib_attrib & 0xFF);
    *puc_msg++ = uc_pib_size;
    memcpy(puc_msg,(uint8_t *)pv_pib_value,uc_pib_size);
    puc_msg += uc_pib_size;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief MLME ACTION request
 *
 * \param us_pib_attrib      ACTION PIB attribute
 * \param pv_pib_value       PIB attribute value
 * \param uc_pib_size        PIB attribute value size
 */
/*void prime_cl_null_mlme_action_request(uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size)
{
    uint8_t *puc_msg;

    //Insert parameters
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_null_mlme_set_request_cmd; //ACTION
    *puc_msg++ = (uint8_t)(us_pib_attrib >> 8);
    *puc_msg++ = (uint8_t)(us_pib_attrib & 0xFF);
    *puc_msg++ = uc_pib_size;
    memcpy(puc_msg,(uint8_t *)pv_pib_value,uc_pib_size);
    puc_msg += uc_pib_size;

    //Send to USI
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;

    hal_usi_send_cmd(&prime_api_msg);
}*/

/**
 * \brief Initialize a cl_432 connection process
 *
 * \param puc_device_id          Pointer to the device identifier data
 * \param uc_device_id_len       Length of the device identfier
 */
void prime_cl_432_establish_request(uint8_t *puc_device_id, uint8_t uc_device_id_len)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_432_establish_request_cmd;
    *puc_msg++ = uc_device_id_len;
    memcpy(puc_msg, puc_device_id, uc_device_id_len);
    puc_msg += uc_device_id_len;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


/**
 * \brief Initializes a cl_432 disconnection process
 *
 * \param us_dst_address   Address to disconnect
 */
void prime_cl_432_release_request(uint16_t us_dst_address)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_432_release_request_cmd;
    *puc_msg++ = (uint8_t)(us_dst_address >> 8);
    *puc_msg++ = (uint8_t)(us_dst_address & 0xFF);

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/**
 * \brief Start a sending data from a 432 connection
 *
 * \param uc_dst_lsap      Destination LSAP
 * \param uc_src_lsap      Source LSAP
 * \param us_dst_address   Destination 432 Address
 * \param px_buff          Pointer to the data buffer
 * \param uc_lsdu_len      Length of the data
 * \param uc_link_class    Link class (non used)
 */
void prime_cl_432_dl_data_request(uint8_t uc_dst_lsap, uint8_t uc_src_lsap, uint16_t us_dst_address, dl_432_buffer_t *px_buff, uint16_t us_lsdu_len,
uint8_t uc_link_class)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_cl_432_dl_data_request_cmd;
    *puc_msg++ = uc_dst_lsap;
    *puc_msg++ = uc_src_lsap;
    *puc_msg++ = (uint8_t)(us_dst_address >> 8);
    *puc_msg++ = (uint8_t)(us_dst_address & 0xFF);
    *puc_msg++ = (uint8_t)(us_lsdu_len >> 8);
    *puc_msg++ = (uint8_t)(us_lsdu_len & 0xFF);
    memcpy(puc_msg, px_buff->dl.buff, us_lsdu_len);
    puc_msg += us_lsdu_len;
    *puc_msg++ = uc_link_class;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

uint8_t _cl_null_establish_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_establish_ind_cb_t prime_cl_null_establish_ind_cb = 0;
	uint8_t *puc_eui48;
	uint8_t *puc_data;
	uint16_t us_con_handle;
	uint16_t us_data_len;
	uint8_t uc_type;
	uint8_t uc_cfbytes;
	uint8_t i;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_establish_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_establish_ind_cb;
    }

    us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
	us_con_handle += *ptrMsg++;
	puc_eui48 = ptrMsg;
	ptrMsg += LEN_ID_EUI48;
	uc_type = *ptrMsg++;
	us_data_len = ((uint16_t)(*ptrMsg++)) << 8;
	us_data_len += *ptrMsg++;
	if (len != 12 + us_data_len) {
		return false;
	}
	puc_data = ptrMsg;
	ptrMsg += us_data_len;
	uc_cfbytes = *ptrMsg++;

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_establish_ind_cb = sx_prime_api[i].prime_cl_null_establish_ind_cb;
    	if (prime_cl_null_establish_ind_cb) {
    		prime_cl_null_establish_ind_cb(us_con_handle, puc_eui48, uc_type, puc_data, us_data_len, uc_cfbytes);
   	    }
    }

    return true;
}

uint8_t _cl_null_establish_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_establish_cfm_cb_t prime_cl_null_establish_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_establish_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_establish_cfm_cb;
    }

    if (prime_cl_null_establish_cfm_cb)
    {
        uint16_t us_con_handle;
        mac_establish_confirm_result_t uc_result;
        uint8_t *puc_eui48;
        uint8_t uc_type;
        uint8_t *puc_data;
        uint16_t us_data_len;

        us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
        us_con_handle += *ptrMsg++;
        uc_result = (mac_establish_confirm_result_t)(*ptrMsg++);
        puc_eui48 = ptrMsg;
        ptrMsg += LEN_ID_EUI48;
        uc_type = *ptrMsg++;
        us_data_len = ((uint16_t)(*ptrMsg++)) << 8;
        us_data_len += *ptrMsg++;
        if(len != 12 + us_data_len)
            return false;
        puc_data = ptrMsg;

        prime_cl_null_establish_cfm_cb(us_con_handle,uc_result,puc_eui48,uc_type,puc_data,us_data_len);
    }
    return true;
}

uint8_t _cl_null_release_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_release_ind_cb_t prime_cl_null_release_ind_cb = 0;
	uint16_t us_con_handle;
    mac_release_indication_reason_t uc_reason;
    uint8_t i;

	if (len != 3) {
		return false;
	}

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_release_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_release_ind_cb;
    }

    us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
	us_con_handle += *ptrMsg++;
	uc_reason = (mac_release_indication_reason_t)(*ptrMsg++);

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_release_ind_cb = sx_prime_api[i].prime_cl_null_release_ind_cb;
    	if (prime_cl_null_release_ind_cb) {
    		prime_cl_null_release_ind_cb(us_con_handle, uc_reason);
   	    }
    }

    return true;
}

uint8_t _cl_null_release_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_release_cfm_cb_t prime_cl_null_release_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_release_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_release_cfm_cb;
    }

    if (prime_cl_null_release_cfm_cb)
    {
        if(len != 3)
            return false;

        uint16_t us_con_handle;
        mac_release_confirm_result_t uc_result;

        us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
        us_con_handle += *ptrMsg++;
        uc_result = (mac_release_confirm_result_t)(*ptrMsg++);

        prime_cl_null_release_cfm_cb(us_con_handle,uc_result);
    }
    return true;
}

uint8_t _cl_null_join_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_join_ind_cb_t prime_cl_null_join_ind_cb = 0;
	uint8_t *puc_eui48;
	uint8_t *puc_data;
	uint16_t us_con_handle;
	uint16_t us_data_len;
	uint8_t uc_con_type;
	uint8_t i;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_join_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_join_ind_cb;
    }

    us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
	us_con_handle += *ptrMsg++;
	puc_eui48 = ptrMsg;
	ptrMsg += LEN_ID_EUI48;
	uc_con_type = *ptrMsg++;
	us_data_len = ((uint16_t)(*ptrMsg++)) << 8;
	us_data_len += *ptrMsg++;

	if (len != 11 + us_data_len) {
		return false;
	}
	puc_data = ptrMsg;

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_join_ind_cb = sx_prime_api[i].prime_cl_null_join_ind_cb;
    	if (prime_cl_null_join_ind_cb) {
    		prime_cl_null_join_ind_cb(us_con_handle, puc_eui48, uc_con_type, puc_data,us_data_len);
   	    }
    }

    return true;
}

uint8_t _cl_null_join_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_join_cfm_cb_t prime_cl_null_join_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_join_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_join_cfm_cb;
    }

    if (prime_cl_null_join_cfm_cb)
    {
        uint16_t us_con_handle;
        mac_join_confirm_result_t uc_result;

        if(len != 3)
            return false;

        us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
        us_con_handle += *ptrMsg++;
        uc_result = (mac_join_confirm_result_t)(*ptrMsg++);


        prime_cl_null_join_cfm_cb(us_con_handle,uc_result);
    }
    return true;
}

uint8_t _cl_null_leave_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_leave_ind_cb_t prime_cl_null_leave_ind_cb = 0;
	uint8_t *puc_eui48;
	uint16_t us_con_handle;
	uint8_t uc_index;
	uint8_t i;

	if (len != 0) {
		return false;
	}

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_leave_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_leave_ind_cb;
    }

    us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
	us_con_handle += *ptrMsg++;
	puc_eui48 = ptrMsg;
	for (uc_index = 0; uc_index < LEN_ID_EUI48; uc_index ++)
	{
		if (puc_eui48[uc_index] != 0xFF)
			break;
		else
		{
			if (uc_index == LEN_ID_EUI48 - 1)
				puc_eui48 = NULL;
		}
	}

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_leave_ind_cb = sx_prime_api[i].prime_cl_null_leave_ind_cb;
    	if (prime_cl_null_leave_ind_cb) {
    		prime_cl_null_leave_ind_cb(us_con_handle, puc_eui48);
   	    }
    }

    return true;
}

uint8_t _cl_null_leave_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_leave_cfm_cb_t prime_cl_null_leave_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_leave_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_leave_cfm_cb;
    }

    if (prime_cl_null_leave_cfm_cb)
    {
        uint16_t us_con_handle;
        mac_leave_confirm_result_t uc_result;

        if(len != 3)
            return false;

        us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
        us_con_handle += *ptrMsg++;
        uc_result = (mac_leave_confirm_result_t)*ptrMsg;

        prime_cl_null_leave_cfm_cb(us_con_handle,uc_result);
    }
    return true;
}

uint8_t _cl_null_data_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_data_cfm_cb_t prime_cl_null_data_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_data_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_data_cfm_cb;
    }

    if (prime_cl_null_data_cfm_cb)
    {
        uint16_t us_con_handle;
        uint32_t ul_temp;
        uint8_t *puc_data;
        uint8_t index;
        uint8_t found = false;
        mac_data_result_t drt_result;

        if(len != 7)
            return false;

        us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
        us_con_handle += *ptrMsg++;
        ul_temp = ((uint32_t)(*ptrMsg++)) << 24;
        ul_temp += ((uint32_t)(*ptrMsg++)) << 16;
        ul_temp += ((uint32_t)(*ptrMsg++)) << 8;
        ul_temp += (uint32_t)(*ptrMsg++);
        drt_result = (mac_data_result_t)*ptrMsg;

        /* Search pointer in table using connection handler */
        for (index = 0; index < NUM_ELEMENTS_POINTER_TABLE; index++)
        {
            if ((sx_data_pointer_table[index].valid) && (sx_data_pointer_table[index].con_handle) == us_con_handle)
            {
                puc_data = sx_data_pointer_table[index].puc_data;
                sx_data_pointer_table[index].valid = false;
                found = true;
                break;
            }
        }
        if (found)
            prime_cl_null_data_cfm_cb(us_con_handle,puc_data,drt_result);
        else
            prime_cl_null_data_cfm_cb(us_con_handle,NULL,drt_result);
    }
    return true;
}

uint8_t _cl_null_data_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_data_ind_cb_t prime_cl_null_data_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_data_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_data_ind_cb;
    }

    if (prime_cl_null_data_ind_cb)
    {
        uint16_t us_con_handle;
        uint8_t *puc_data;
        uint16_t us_data_len;

        us_con_handle = ((uint16_t)(*ptrMsg++)) << 8;
        us_con_handle += *ptrMsg++;
        us_data_len = ((uint16_t)(*ptrMsg++)) << 8;
        us_data_len += *ptrMsg++;
        //if(len != 4 + us_data_len)
        //    return false;
        puc_data = ptrMsg;

        prime_cl_null_data_ind_cb(us_con_handle,puc_data,us_data_len);
    }
    return true;
}

uint8_t _cl_null_plme_reset_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_plme_reset_cfm_cb_t prime_cl_null_plme_reset_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_plme_reset_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_plme_reset_cfm_cb;
    }

    if (prime_cl_null_plme_reset_cfm_cb)
    {
        plme_result_t x_result;

        if(len != 1)
            return false;

        x_result = (plme_result_t)*ptrMsg;

        prime_cl_null_plme_reset_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_plme_sleep_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_plme_sleep_cfm_cb_t prime_cl_null_plme_sleep_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_plme_sleep_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_plme_sleep_cfm_cb;
    }

    if (prime_cl_null_plme_sleep_cfm_cb)
    {
        plme_result_t x_result;

        if(len != 1)
            return false;

        x_result = (plme_result_t)*ptrMsg;

        prime_cl_null_plme_sleep_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_plme_resume_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_plme_resume_cfm_cb_t prime_cl_null_plme_resume_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_plme_resume_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_plme_resume_cfm_cb;
    }

    if (prime_cl_null_plme_resume_cfm_cb)
    {
        plme_result_t x_result;

        if(len != 1)
            return false;

        x_result = (plme_result_t)*ptrMsg;

        prime_cl_null_plme_resume_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_plme_testmode_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_plme_testmode_cfm_cb_t prime_cl_null_plme_testmode_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_plme_testmode_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_plme_testmode_cfm_cb;
    }

    if (prime_cl_null_plme_testmode_cfm_cb)
    {
        plme_result_t x_result;

        if(len != 1)
            return false;

        x_result = (plme_result_t)*ptrMsg;

        prime_cl_null_plme_testmode_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_plme_get_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_plme_get_cfm_cb_t prime_cl_null_plme_get_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_plme_get_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_plme_get_cfm_cb;
    }

    if (prime_cl_null_plme_get_cfm_cb)
    {
        plme_result_t x_status;
        uint16_t us_pib_attrib;
        void *pv_pib_value;
        uint8_t uc_pib_size;

        x_status = (plme_result_t)*ptrMsg++;
        us_pib_attrib = ((uint16_t)(*ptrMsg++)) << 8;
        us_pib_attrib += *ptrMsg++;
        uc_pib_size = *ptrMsg++;
        if(len != 4 + uc_pib_size)
            return false;
        pv_pib_value = (void*)ptrMsg;

        prime_cl_null_plme_get_cfm_cb(x_status,us_pib_attrib,pv_pib_value,uc_pib_size);
    }
    return true;
}

uint8_t _cl_null_plme_set_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_plme_set_cfm_cb_t prime_cl_null_plme_set_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_plme_set_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_plme_set_cfm_cb;
    }

    if (prime_cl_null_plme_set_cfm_cb)
    {
        plme_result_t x_result;

        if(len != 1)
            return false;
        x_result = (plme_result_t)*ptrMsg++;

        prime_cl_null_plme_set_cfm_cb(x_result);
    }
    return true;
}


uint8_t _cl_null_mlme_register_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_register_cfm_cb_t prime_cl_null_mlme_register_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_register_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_register_cfm_cb;
    }

    if (prime_cl_null_mlme_register_cfm_cb)
    {
        mlme_result_t x_result;
        uint8_t *puc_sna;
        uint8_t uc_sid;

        if(len != 8)
            return false;
        x_result = (mlme_result_t)*ptrMsg++;
        puc_sna = ptrMsg;
        ptrMsg += LEN_ID_EUI48;
        uc_sid = *ptrMsg++;

        prime_cl_null_mlme_register_cfm_cb(x_result,puc_sna,uc_sid);
    }
    return true;
}

uint8_t _cl_null_mlme_register_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_register_ind_cb_t prime_cl_null_mlme_register_ind_cb = 0;
	uint8_t *puc_sna;
	uint8_t i;
	uint8_t uc_sid;

	if (len != 0) {
		return false;
	}

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_register_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_register_ind_cb;
    }

    puc_sna = ptrMsg;
	ptrMsg += LEN_ID_EUI48;
	uc_sid = *ptrMsg++;

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_mlme_register_ind_cb = sx_prime_api[i].prime_cl_null_mlme_register_ind_cb;
    	if (prime_cl_null_mlme_register_ind_cb) {
    		prime_cl_null_mlme_register_ind_cb(puc_sna,uc_sid);
   	    }
    }

    return true;
}

uint8_t _cl_null_mlme_unregister_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_unregister_cfm_cb_t prime_cl_null_mlme_unregister_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_unregister_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_unregister_cfm_cb;
    }

    if (prime_cl_null_mlme_unregister_cfm_cb)
    {
        mlme_result_t x_result;

        if(len != 1)
            return false;
        x_result = (mlme_result_t)*ptrMsg++;

        prime_cl_null_mlme_unregister_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_mlme_unregister_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_unregister_ind_cb_t prime_cl_null_mlme_unregister_ind_cb = 0;
	uint8_t i;

	if (len != 0) {
		return false;
	}

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_unregister_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_unregister_ind_cb;
    }

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_mlme_unregister_ind_cb = sx_prime_api[i].prime_cl_null_mlme_unregister_ind_cb;
    	if (prime_cl_null_mlme_unregister_ind_cb) {
    		prime_cl_null_mlme_unregister_ind_cb();
   	    }
    }

    return true;
}

uint8_t _cl_null_mlme_promote_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_promote_cfm_cb_t prime_cl_null_mlme_promote_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_promote_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_promote_cfm_cb;
    }

    if (prime_cl_null_mlme_promote_cfm_cb)
    {
        mlme_result_t x_result;

        if(len != 1)
            return false;
        x_result = (mlme_result_t)*ptrMsg++;

        prime_cl_null_mlme_promote_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_mlme_promote_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_promote_ind_cb_t prime_cl_null_mlme_promote_ind_cb = 0;
	uint8_t i;

	if (len != 0) {
		return false;
	}

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_promote_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_promote_ind_cb;
    }

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_mlme_promote_ind_cb = sx_prime_api[i].prime_cl_null_mlme_promote_ind_cb;
    	if (prime_cl_null_mlme_promote_ind_cb) {
    		prime_cl_null_mlme_promote_ind_cb();
   	    }
    }

    return true;
}

uint8_t _cl_null_mlme_demote_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_demote_cfm_cb_t prime_cl_null_mlme_demote_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_demote_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_demote_cfm_cb;
    }

    if (prime_cl_null_mlme_demote_cfm_cb)
    {
        mlme_result_t x_result;

        if(len != 1)
            return false;
        x_result = (mlme_result_t)*ptrMsg++;

        prime_cl_null_mlme_demote_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_mlme_demote_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_demote_ind_cb_t prime_cl_null_mlme_demote_ind_cb = 0;
	uint8_t i;

	if (len != 0) {
		return false;
	}

	if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_demote_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_demote_ind_cb;
    }

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_null_mlme_demote_ind_cb = sx_prime_api[i].prime_cl_null_mlme_demote_ind_cb;
    	if (prime_cl_null_mlme_demote_ind_cb) {
    		prime_cl_null_mlme_demote_ind_cb();
   	    }
    }

    return true;
}

uint8_t _cl_null_mlme_reset_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_reset_cfm_cb_t prime_cl_null_mlme_reset_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_reset_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_reset_cfm_cb;
    }

    if (prime_cl_null_mlme_reset_cfm_cb)
    {
        mlme_result_t x_result;

        if(len != 1)
            return false;
        x_result = (mlme_result_t)*ptrMsg++;

        prime_cl_null_mlme_reset_cfm_cb(x_result);
    }
    return true;
}

uint8_t _cl_null_mlme_get_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_get_cfm_cb_t prime_cl_null_mlme_get_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_get_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_get_cfm_cb;
    }

    if (prime_cl_null_mlme_get_cfm_cb)
    {
        mlme_result_t x_status;
        uint16_t us_pib_attrib;
        void *pv_pib_value;
        uint8_t uc_pib_size;

        x_status = (mlme_result_t)*ptrMsg++;
        us_pib_attrib = ((uint16_t)(*ptrMsg++)) << 8;
        us_pib_attrib += *ptrMsg++;
        uc_pib_size = *ptrMsg++;
        if(len != 4 + uc_pib_size)
            return false;
        pv_pib_value = (void*)ptrMsg;

        prime_cl_null_mlme_get_cfm_cb(x_status,us_pib_attrib,pv_pib_value,uc_pib_size);
    }
    return true;
}

uint8_t _cl_null_mlme_list_get_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_list_get_cfm_cb_t prime_cl_null_mlme_list_get_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_list_get_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_list_get_cfm_cb;
    }

    if (prime_cl_null_mlme_list_get_cfm_cb)
    {
        mlme_result_t x_status;
        uint16_t us_pib_attrib;
        uint8_t *puc_pib_buff;
        uint16_t us_pib_len;

        x_status = (mlme_result_t)*ptrMsg++;
        us_pib_attrib = ((uint16_t)(*ptrMsg++)) << 8;
        us_pib_attrib += *ptrMsg++;
        us_pib_len = ((uint16_t)(*ptrMsg++)) << 8;
        us_pib_len += *ptrMsg++;
        if(len != 5 + us_pib_len)
            return false;
        puc_pib_buff = ptrMsg;

        prime_cl_null_mlme_list_get_cfm_cb(x_status,us_pib_attrib,puc_pib_buff,us_pib_len);
    }
    return true;
}

uint8_t _cl_null_mlme_set_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_null_mlme_set_cfm_cb_t prime_cl_null_mlme_set_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_null_mlme_set_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_null_mlme_set_cfm_cb;
    }

    if (prime_cl_null_mlme_set_cfm_cb)
    {
        mlme_result_t x_result;

        if(len != 1)
            return false;
        x_result = (mlme_result_t)*ptrMsg++;

        prime_cl_null_mlme_set_cfm_cb(x_result);
    }
    return true;
}


void prime_cl_432_set_callbacks(uint8_t prime_app_id, prime_cl_432_callbacks_t *px_prime_cbs)
{
	prime_api_cbs_t *px_prime_api;

	if (prime_app_id < PRIME_API_MAX_NUM) {
		px_prime_api = &sx_prime_api[prime_app_id];
		px_prime_api->prime_cl_432_establish_cfm_cb = px_prime_cbs->prime_cl_432_establish_cfm_cb;
		px_prime_api->prime_cl_432_release_cfm_cb = px_prime_cbs->prime_cl_432_release_cfm_cb;
		px_prime_api->prime_cl_432_dl_data_ind_cb = px_prime_cbs->prime_cl_432_dl_data_ind_cb;
		px_prime_api->prime_cl_432_dl_data_cfm_cb = px_prime_cbs->prime_cl_432_dl_data_cfm_cb;
		px_prime_api->prime_cl_432_dl_join_ind_cb = px_prime_cbs->prime_cl_432_dl_join_ind_cb;
		px_prime_api->prime_cl_432_dl_leave_ind_cb = px_prime_cbs->prime_cl_432_dl_leave_ind_cb;
	}
}

uint8_t _cl_432_establish_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_432_establish_cfm_cb_t prime_cl_432_establish_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_432_establish_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_432_establish_cfm_cb;
    }

    if (prime_cl_432_establish_cfm_cb)
    {
        uint8_t *puc_device_id;
        uint8_t uc_device_id_len;
        uint16_t us_dst_address;
        uint16_t us_base_address;

        uc_device_id_len = *ptrMsg++;
        if(len != 5 + uc_device_id_len)
            return false;
        puc_device_id = ptrMsg;
        ptrMsg += uc_device_id_len;
        us_dst_address = ((uint16_t)(*ptrMsg++)) << 8;
        us_dst_address += *ptrMsg++;
        us_base_address = ((uint16_t)(*ptrMsg++)) << 8;
        us_base_address += *ptrMsg++;

        prime_cl_432_establish_cfm_cb(puc_device_id,uc_device_id_len,us_dst_address,us_base_address);
    }
    return true;
}

uint8_t _cl_432_release_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_432_release_cfm_cb_t prime_cl_432_release_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_432_release_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_432_release_cfm_cb;
    }

    if (prime_cl_432_release_cfm_cb)
    {
        uint16_t us_dst_address;
        dl_432_result_t uc_result;

        if(len != 3)
            return false;
        us_dst_address = ((uint16_t)(*ptrMsg++)) << 8;
        us_dst_address += *ptrMsg++;
        uc_result = (dl_432_result_t)*ptrMsg++;

        prime_cl_432_release_cfm_cb(us_dst_address,uc_result);
    }
    return true;
}

uint8_t _cl_432_dl_data_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_432_dl_data_ind_cb_t prime_cl_432_dl_data_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_432_dl_data_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_432_dl_data_ind_cb;
    }

    if (prime_cl_432_dl_data_ind_cb) {
        uint8_t *puc_data;
        uint8_t uc_dst_lsap;
        uint8_t uc_src_lsap;
        uint16_t us_dst_address;
        uint16_t src_address;
        uint16_t us_lsdu_len;
        uint8_t uc_link_class;

        uc_dst_lsap = *ptrMsg++;
        uc_src_lsap = *ptrMsg++;
        us_dst_address = ((uint16_t)(*ptrMsg++)) << 8;
        us_dst_address += *ptrMsg++;
        src_address = ((uint16_t)(*ptrMsg++)) << 8;
        src_address += *ptrMsg++;
        us_lsdu_len = ((uint16_t)(*ptrMsg++)) << 8;
        us_lsdu_len += *ptrMsg++;

        if (len != 9 + us_lsdu_len) {
            return false;
        }

        puc_data = ptrMsg;
        ptrMsg += us_lsdu_len;
        uc_link_class = *ptrMsg++;

        prime_cl_432_dl_data_ind_cb(uc_dst_lsap,uc_src_lsap,us_dst_address,src_address,puc_data,us_lsdu_len,uc_link_class);
    }
    return true;
}

uint8_t _cl_432_dl_data_cfm(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_432_dl_data_cfm_cb_t prime_cl_432_dl_data_cfm_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_432_dl_data_cfm_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_432_dl_data_cfm_cb;
    }

    if (prime_cl_432_dl_data_cfm_cb)
    {
        uint8_t uc_dst_lsap;
        uint8_t uc_src_lsap;
        uint16_t us_dst_address;
        uint8_t uc_tx_status;

        if (len != 5) {
            return false;
        }

        uc_dst_lsap = *ptrMsg++;
        uc_src_lsap = *ptrMsg++;
        us_dst_address = ((uint16_t)(*ptrMsg++)) << 8;
        us_dst_address += *ptrMsg++;
        uc_tx_status = *ptrMsg++;

        prime_cl_432_dl_data_cfm_cb(uc_dst_lsap,uc_src_lsap,us_dst_address,uc_tx_status);
    }
    return true;
}

uint8_t _cl_432_dl_leave_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_432_dl_leave_ind_cb_t prime_cl_432_dl_leave_ind_cb = 0;
	uint16_t us_lnid;
	uint8_t i;

	if(len != 2) {
		return false;
	}

	us_lnid = ((uint16_t)(*ptrMsg++)) << 8;
	us_lnid += *ptrMsg++;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	prime_cl_432_dl_leave_ind_cb = sx_prime_api[suc_prime_app_id_req].prime_cl_432_dl_leave_ind_cb;
    }

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_432_dl_leave_ind_cb = sx_prime_api[i].prime_cl_432_dl_leave_ind_cb;
    	if (prime_cl_432_dl_leave_ind_cb) {
    		prime_cl_432_dl_leave_ind_cb(us_lnid);
   	    }
    }

    return true;
}

uint8_t _cl_432_dl_join_ind(uint8_t* ptrMsg, uint16_t len)
{
	prime_cl_432_dl_join_ind_cb_t prime_cl_432_dl_join_ind_cb = 0;
	uint8_t i;
	uint8_t  uc_device_id_len;
	uint16_t us_dst_address;
	uint8_t  puc_device_id[64]; //DLMS Device name.
	uint8_t  puc_mac[6];

	uc_device_id_len = *ptrMsg++;
	memcpy(puc_device_id, ptrMsg, uc_device_id_len);
	ptrMsg        +=uc_device_id_len;
	us_dst_address  = ((uint16_t)(*ptrMsg++)) << 8;
	us_dst_address += *ptrMsg++;
	memcpy(puc_mac, ptrMsg, 6);

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	prime_cl_432_dl_join_ind_cb = sx_prime_api[i].prime_cl_432_dl_join_ind_cb;
    	if (prime_cl_432_dl_join_ind_cb) {
    		prime_cl_432_dl_join_ind_cb(puc_device_id, uc_device_id_len, us_dst_address, puc_mac);
   	    }
    }

    return true;
}

void bmng_set_callbacks(uint8_t prime_app_id, prime_bmng_callbacks_t *px_bmng_cbs)
{
	prime_api_cbs_t *px_prime_api;

	if (prime_app_id < PRIME_API_MAX_NUM) {
		px_prime_api = &sx_prime_api[prime_app_id];
		px_prime_api->bmng_fup_ack_ind_cb = px_bmng_cbs->fup_ack_ind_cb;
		px_prime_api->bmng_fup_status_ind_cb = px_bmng_cbs->fup_status_ind_cb;
		px_prime_api->bmng_fup_error_ind_cb = px_bmng_cbs->fup_error_ind_cb;
		px_prime_api->bmng_fup_version_ind_cb = px_bmng_cbs->fup_version_ind_cb;
		px_prime_api->bmng_fup_kill_ind_cb = px_bmng_cbs->fup_kill_ind_cb;
		px_prime_api->bmng_network_event_ind_cb = px_bmng_cbs->network_event_ind_cb;
		px_prime_api->bmng_pprof_ack_ind_cb = px_bmng_cbs->pprof_ack_ind_cb;
		px_prime_api->bmng_pprof_get_response_cb = px_bmng_cbs->pprof_get_response_cb;
		px_prime_api->bmng_pprof_get_enhanced_response_cb = px_bmng_cbs->pprof_get_enhanced_response_cb;
	}
}

void bmng_fup_clear_target_list_request()
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_clear_target_list_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_add_target_request(uint8_t * puc_eui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_add_target_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_set_fw_data_request(uint8_t  uc_vendor_len, uint8_t * puc_vendor, uint8_t  uc_model_len, uint8_t * puc_model,uint8_t  uc_version_len, uint8_t * puc_version  )
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_set_fw_data_request_cmd;
    *puc_msg++ = uc_vendor_len;
    memcpy(puc_msg,puc_vendor,uc_vendor_len);
    puc_msg+=uc_vendor_len;
    *puc_msg++ = uc_model_len;
    memcpy(puc_msg,puc_model,uc_model_len);
    puc_msg+=uc_model_len;
    *puc_msg++ = uc_version_len;
    memcpy(puc_msg,puc_version,uc_version_len);
    puc_msg+=uc_version_len;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);

    return;
}

void bmng_fup_set_upg_options_request(uint8_t uc_arq_en, uint8_t uc_page_size, uint8_t uc_mult_en, uint32_t ui_delay, uint32_t ui_timer)
{

    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_set_upg_options_request_cmd;
    *puc_msg++ = uc_arq_en;
    *puc_msg++ = uc_page_size;
    *puc_msg++ = uc_mult_en;
    *puc_msg++ = (uint8_t)(ui_delay >>  24);
    *puc_msg++ = (uint8_t)((ui_delay >> 16) & 0xFF);
    *puc_msg++ = (uint8_t)((ui_delay >>  8) & 0xFF);
    *puc_msg++ = (uint8_t)(ui_delay & 0xFF);
    *puc_msg++ = (uint8_t)(ui_timer >>  24);
    *puc_msg++ = (uint8_t)((ui_timer >> 16) & 0xFF);
    *puc_msg++ = (uint8_t)((ui_timer >>  8) & 0xFF);
    *puc_msg++ = (uint8_t)(ui_timer & 0xFF);

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_init_file_tx_request(uint16_t us_num_frames, uint32_t ui_file_size, uint16_t us_frame_size, uint32_t ui_crc)
{

    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_init_file_tx_request_cmd;
    *puc_msg++ = (uint8_t)((us_num_frames >>  8) & 0xFF);
    *puc_msg++ = (uint8_t) (us_num_frames & 0xFF);
    *puc_msg++ = (uint8_t)( ui_file_size >>  24);
    *puc_msg++ = (uint8_t)((ui_file_size >> 16) & 0xFF);
    *puc_msg++ = (uint8_t)((ui_file_size >>  8) & 0xFF);
    *puc_msg++ = (uint8_t)( ui_file_size & 0xFF);
    *puc_msg++ = (uint8_t)((us_frame_size >>  8) & 0xFF);
    *puc_msg++ = (uint8_t) (us_frame_size & 0xFF);
    *puc_msg++ = (uint8_t) (ui_crc >>  24);
    *puc_msg++ = (uint8_t)((ui_crc >> 16) & 0xFF);
    *puc_msg++ = (uint8_t)((ui_crc >>  8) & 0xFF);
    *puc_msg++ = (uint8_t) (ui_crc & 0xFF);

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_data_frame_request(uint16_t us_frame_num, uint16_t us_len, uint8_t * puc_data)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_data_frame_request_cmd;
    *puc_msg++ = (uint8_t)((us_frame_num >>  8) & 0xFF);
    *puc_msg++ = (uint8_t) (us_frame_num & 0xFF);
    *puc_msg++ = (uint8_t)((us_len >>  8) & 0xFF);
    *puc_msg++ = (uint8_t) (us_len & 0xFF);
    memcpy(puc_msg, puc_data, us_len);
    puc_msg+= us_len;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_check_crc_request()
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_check_crc_request_cmd;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_abort_fu_request(uint8_t * puc_eui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_abort_fu_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_start_fu_request(uint8_t uc_enable)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_start_fu_request_cmd;
    *puc_msg++ = uc_enable;
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_set_match_rule_request(uint8_t uc_rule)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_set_match_rule_request_cmd;
    *puc_msg++ = uc_rule;
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_get_version_request(uint8_t * puc_eui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_get_version_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_fup_get_state_request(uint8_t * puc_eui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_get_state_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

uint8_t _prime_fup_ack_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_fup_ack_ind_cb_t fup_ack_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	fup_ack_ind_cb = sx_prime_api[suc_prime_app_id_req].bmng_fup_ack_ind_cb;
    }

    if (fup_ack_ind_cb)
    {
        uint8_t uc_cmd;
        uint8_t uc_ack;
        uint16_t us_data = 0xFFFF;

        if (len < 2)
            return false;
        if (len > 4)
            return false;

        uc_cmd = *ptrMsg++;
        uc_ack = *ptrMsg++;
        if (len > 2) {
            us_data = ((uint16_t)(*ptrMsg++)) << 8;
            us_data += *ptrMsg++;
        }

        fup_ack_ind_cb(uc_cmd,uc_ack,us_data);
    }
    return true;
}

uint8_t _prime_fup_status_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_fup_status_ind_cb_t fup_status_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	fup_status_ind_cb = sx_prime_api[suc_prime_app_id_req].bmng_fup_status_ind_cb;
    }

    if (fup_status_ind_cb)
    {
        uint8_t   uc_status;
        uint32_t  ui_pages;
        uint8_t   puc_eui48[6];


        if (len < 9)
            return false;

        uc_status = *ptrMsg++;
        ui_pages  = *ptrMsg++; ui_pages <<=8;
        ui_pages += *ptrMsg++;
        memcpy(puc_eui48,ptrMsg,6);


        fup_status_ind_cb(uc_status, ui_pages, puc_eui48);
    }
    return true;
}

uint8_t _prime_fup_error_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_fup_error_ind_cb_t fup_error_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	fup_error_ind_cb = sx_prime_api[suc_prime_app_id_req].bmng_fup_error_ind_cb;
    }

    if (fup_error_ind_cb)
    {
        uint8_t   uc_err_code;
        uint8_t   puc_eui48[6];


        if (len < 7)
            return false;

        uc_err_code = *ptrMsg++;
        memcpy(puc_eui48,ptrMsg,6);


        fup_error_ind_cb(uc_err_code, puc_eui48);
    }
    return true;
}

uint8_t _prime_fup_version_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_fup_version_ind_cb_t fup_version_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	fup_version_ind_cb = sx_prime_api[suc_prime_app_id_req].bmng_fup_version_ind_cb;
    }

    if (fup_version_ind_cb)
    {
        uint8_t puc_eui48[LEN_ID_EUI48];
        uint8_t uc_vendor_len;
        uint8_t puc_vendor[32];
        uint8_t uc_model_len;
        uint8_t puc_model[32];
        uint8_t uc_version_len;
        uint8_t puc_version[32];

        //if (len < 6 + uc_vendor_len + uc_model_len + uc_version_len)
        //    return false;

        memcpy(puc_eui48,ptrMsg,6);
        ptrMsg+=6;
        uc_vendor_len =  *ptrMsg++;
        if (uc_vendor_len > 32) uc_vendor_len = 32;
        memcpy(puc_vendor,ptrMsg,uc_vendor_len);
        ptrMsg+=uc_vendor_len;

        uc_model_len =  *ptrMsg++;
        if (uc_model_len > 32) uc_model_len = 32;
        memcpy(puc_model,ptrMsg,uc_model_len);
        ptrMsg+=uc_model_len;

        uc_version_len =  *ptrMsg++;
        if (uc_version_len > 32) uc_version_len = 32;
        memcpy(puc_version,ptrMsg,uc_version_len);
        ptrMsg+=uc_version_len;

        if (len < 6 + uc_vendor_len + uc_model_len + uc_version_len)
                   return false;

        fup_version_ind_cb(puc_eui48, uc_vendor_len, puc_vendor,uc_model_len, puc_model, uc_version_len, puc_version);
    }
    return true;
}


uint8_t _prime_fup_kill_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_fup_kill_ind_cb_t fup_kill_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	fup_kill_ind_cb = sx_prime_api[suc_prime_app_id_req].bmng_fup_kill_ind_cb;
    }

    if (fup_kill_ind_cb)
    {
        uint8_t   puc_eui48[6];

        if (len < 6)
            return false;
        memcpy(puc_eui48,ptrMsg,6);
        fup_kill_ind_cb(puc_eui48);
    }
    return true;
}

void bmng_fup_signature_cfg_cmd(uint8_t uc_sig_algo, uint16_t us_sig_len )
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_fup_set_signature_data_request_cmd;
    *puc_msg++ = uc_sig_algo;
    *puc_msg++ = us_sig_len >> 8;
    *puc_msg++ = us_sig_len & 0xFF;


    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}


uint8_t _bmng_network_ind(uint8_t* ptrMsg, uint16_t len)
{
    bmng_net_event_t px_net_event;
    bmng_network_event_ind_cb_t network_event_ind_cb = 0;
    uint8_t i;

    px_net_event.net_event= *ptrMsg++;
	memcpy(px_net_event.mac,ptrMsg,6);
	ptrMsg+=6;
	px_net_event.sid      = *ptrMsg++;
	px_net_event.lnid     = ((uint16_t)(*ptrMsg++)) << 8;
	px_net_event.lnid    += *ptrMsg++;
	px_net_event.lsid     = *ptrMsg++;
	px_net_event.alvRxcnt = *ptrMsg++;
	px_net_event.alvTxcnt = *ptrMsg++;
	px_net_event.alvTime  = *ptrMsg++;

    for (i = 0; i < PRIME_API_MAX_NUM; i++) {
    	network_event_ind_cb = sx_prime_api[i].bmng_network_event_ind_cb;
    	if (network_event_ind_cb) {
   	        network_event_ind_cb(&px_net_event);
   	    }
    }

    return true;
}

void bmng_pprof_get_request(uint8_t * puc_eui48, uint16_t us_pib, uint8_t uc_index)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_pprof_get_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;
    *puc_msg++ = 0; /* length */
    *puc_msg++ = 3; /* length */
    *puc_msg++ = (uint8_t)(us_pib >> 8);
    *puc_msg++ = (uint8_t)(us_pib & 0xFF);
    *puc_msg++ = uc_index; /* index */
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_pprof_set_request(uint8_t * puc_eui48, uint16_t us_pib, uint32_t ul_value)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_pprof_set_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;
    *puc_msg++ = 0; /* length */
    *puc_msg++ = 6; /* length */
    /* pib attribute */
    *puc_msg++ = (uint8_t)(us_pib >> 8);
    *puc_msg++ = (uint8_t)(us_pib & 0xFF);
    /* pib value */
    *puc_msg++ = (uint8_t)(ul_value >> 24);
    *puc_msg++ = (uint8_t)((ul_value >> 16) & 0xFF);
    *puc_msg++ = (uint8_t)((ul_value >> 8) & 0xFF);
    *puc_msg++ = (uint8_t)(ul_value & 0x00FF);
    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

void bmng_pprof_reboot(uint8_t * puc_eui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_pprof_reboot_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}
void bmng_pprof_reset(uint8_t * puc_eui48)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_pprof_reset_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+=6;

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

/*Item Enhanced header id */
#ifndef MNGP_PRIME_LISTQRY
#define MNGP_PRIME_LISTQRY			0x0E
#define MNGP_PRIME_LISTRSP			0x0F
#endif

void bmng_pprof_get_enhanced(uint8_t * puc_eui48,uint16_t us_pib,uint8_t uc_max_records,uint16_t us_iterator)
{
    uint8_t *puc_msg;

    /*Insert parameters*/
    puc_msg = uc_serial_buf;

    *puc_msg++ = prime_bmng_pprof_get_enhanced_request_cmd;
    memcpy(puc_msg,puc_eui48,6);
    puc_msg+= 6;

    *puc_msg++ = 0; /* length */
    *puc_msg++ = 6; /* length */

    *puc_msg++ = MNGP_PRIME_LISTQRY;
    *puc_msg++ = (uint8_t)(us_pib >> 8);
    *puc_msg++ = (uint8_t)(us_pib & 0xFF);
    *puc_msg++ = uc_max_records;

    memcpy(puc_msg, &us_iterator, 2);

    /*Send to USI*/
    prime_api_msg.uc_p_type = PROTOCOL_PRIME_API;
    prime_api_msg.puc_buf = uc_serial_buf;
    prime_api_msg.us_len = puc_msg - uc_serial_buf;
    prime_api_msg._fd = -1;

    hal_usi_send_cmd(&prime_api_msg);
}

uint8_t _bmng_pprof_get_response_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_pprof_get_response_cb_t pprof_get_response_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	pprof_get_response_cb = sx_prime_api[suc_prime_app_id_req].bmng_pprof_get_response_cb;
    }

    if (pprof_get_response_cb) {
        uint8_t puc_eui48[LEN_ID_EUI48];
        void *pv_pib_value;
        uint16_t us_data_len;

        memcpy(puc_eui48,ptrMsg,6);
        ptrMsg+=6;
        us_data_len = ((uint16_t)(*ptrMsg++)) << 8;
        us_data_len += *ptrMsg++;

        pv_pib_value = (void*)ptrMsg;

        pprof_get_response_cb(puc_eui48, us_data_len, pv_pib_value);
        return true;
    } else {
    	return false;
    }
}

uint8_t _bmng_pprof_get_enhanced_response_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_pprof_get_enhanced_response_cb_t pprof_get_enhanced_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	pprof_get_enhanced_cb = sx_prime_api[suc_prime_app_id_req].bmng_pprof_get_enhanced_response_cb;
    }

    if (pprof_get_enhanced_cb)
    {
        uint8_t puc_eui48[LEN_ID_EUI48];

        void *pv_pib_value;
        uint16_t us_data_len;

        memcpy(puc_eui48,ptrMsg,6);
        ptrMsg+=6;
        us_data_len = ((uint16_t)(*ptrMsg++)) << 8;
        us_data_len += *ptrMsg++;

        pv_pib_value = (void*)ptrMsg;

        pprof_get_enhanced_cb(puc_eui48, us_data_len,pv_pib_value);
        return true;
    } else {
    	return false;
    }
}

uint8_t _bmng_pprof_ack_ind(uint8_t* ptrMsg, uint16_t len)
{
	bmng_pprof_ack_ind_cb_t pprof_ack_ind_cb = 0;

    if (suc_prime_app_id_req < PRIME_API_MAX_NUM) {
    	pprof_ack_ind_cb = sx_prime_api[suc_prime_app_id_req].bmng_pprof_ack_ind_cb;
    }

    if (pprof_ack_ind_cb)
    {
        uint8_t uc_cmd;
        uint8_t uc_resp;

        uc_cmd =  *ptrMsg++;
        uc_resp = *ptrMsg++;

        pprof_ack_ind_cb(uc_cmd,uc_resp);
        return true;
    } else {
    	return false;
    }

}


uint8_t ifacePrime_api_ReceivedCmd(uint8_t* ptrMsg, uint16_t len)
{
    uint8_t uc_cmd;

    uc_cmd = (prime_api_cmd_t)((*ptrMsg++) & 0x7F);
    len --;

    switch (uc_cmd) {
    case prime_cl_null_establish_ind_cmd:
        return _cl_null_establish_ind(ptrMsg,len);
        break;
    case prime_cl_null_establish_cfm_cmd:
        return _cl_null_establish_cfm(ptrMsg,len);
        break;
    case prime_cl_null_release_ind_cmd:
        return _cl_null_release_ind(ptrMsg,len);
        break;
    case prime_cl_null_release_cfm_cmd:
        return _cl_null_release_cfm(ptrMsg,len);
        break;
    case prime_cl_null_join_ind_cmd:
        return _cl_null_join_ind(ptrMsg,len);
        break;
    case prime_cl_null_join_cfm_cmd:
        return _cl_null_join_cfm(ptrMsg,len);
        break;
    case prime_cl_null_leave_cfm_cmd:
        return _cl_null_leave_cfm(ptrMsg,len);
        break;
    case prime_cl_null_leave_ind_cmd:
        return _cl_null_leave_ind(ptrMsg,len);
        break;
    case prime_cl_null_data_cfm_cmd:
        return _cl_null_data_cfm(ptrMsg,len);
        break;
    case prime_cl_null_data_ind_cmd:
        return _cl_null_data_ind(ptrMsg,len);
        break;
    case prime_cl_null_plme_reset_cfm_cmd:
        return _cl_null_plme_reset_cfm(ptrMsg,len);
        break;
    case prime_cl_null_plme_sleep_cfm_cmd:
        return _cl_null_plme_sleep_cfm(ptrMsg,len);
        break;
    case prime_cl_null_plme_resume_cfm_cmd:
        return _cl_null_plme_resume_cfm(ptrMsg,len);
        break;
    case prime_cl_null_plme_testmode_cfm_cmd:
        return _cl_null_plme_testmode_cfm(ptrMsg,len);
        break;
    case prime_cl_null_plme_get_cfm_cmd:
        return _cl_null_plme_get_cfm(ptrMsg,len);
        break;
    case prime_cl_null_plme_set_cfm_cmd:
        return _cl_null_plme_set_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_register_cfm_cmd:
        return _cl_null_mlme_register_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_register_ind_cmd:
        return _cl_null_mlme_register_ind(ptrMsg,len);
        break;
    case prime_cl_null_mlme_unregister_cfm_cmd:
        return _cl_null_mlme_unregister_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_unregister_ind_cmd:
        return _cl_null_mlme_unregister_ind(ptrMsg,len);
        break;
    case prime_cl_null_mlme_promote_cfm_cmd:
        return _cl_null_mlme_promote_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_promote_ind_cmd:
        return _cl_null_mlme_promote_ind(ptrMsg,len);
        break;
    case prime_cl_null_mlme_demote_cfm_cmd:
        return _cl_null_mlme_demote_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_demote_ind_cmd:
        return _cl_null_mlme_demote_ind(ptrMsg,len);
        break;
    case prime_cl_null_mlme_reset_cfm_cmd:
        return _cl_null_mlme_reset_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_get_cfm_cmd:
        return _cl_null_mlme_get_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_list_get_cfm_cmd:
        return _cl_null_mlme_list_get_cfm(ptrMsg,len);
        break;
    case prime_cl_null_mlme_set_cfm_cmd:
        return _cl_null_mlme_set_cfm(ptrMsg,len);
        break;
    case prime_cl_432_establish_cfm_cmd:
        return _cl_432_establish_cfm(ptrMsg,len);
        break;
    case prime_cl_432_release_cfm_cmd:
        return _cl_432_release_cfm(ptrMsg,len);
        break;
    case prime_cl_432_dl_data_ind_cmd:
        return _cl_432_dl_data_ind(ptrMsg,len);
        break;
    case prime_cl_432_dl_data_cfm_cmd:
        return _cl_432_dl_data_cfm(ptrMsg,len);
        break;
    case prime_cl_432_dl_join_ind_cmd:  // = 0x3A, //NOT IMPLEMENTED
        return _cl_432_dl_join_ind(ptrMsg,len);
        break;
    case prime_cl_432_dl_leave_ind_cmd: //0x3B, //NOT IMPLEMENTED
        return _cl_432_dl_leave_ind(ptrMsg,len);
        break;
    case prime_bmng_fup_ack_cmd:
        return _prime_fup_ack_ind(ptrMsg,len);
        break;
    case prime_bmng_fup_status_ind_cmd:
        return _prime_fup_status_ind(ptrMsg,len);
        break;
    case prime_bmng_fup_error_ind_cmd:
        return _prime_fup_error_ind(ptrMsg,len);
        break;
    case prime_bmng_fup_version_ind_cmd:
        return _prime_fup_version_ind(ptrMsg,len);
        break;
    case prime_bmng_fup_kill_ind_cmd:
        return _prime_fup_kill_ind(ptrMsg,len);
        break;
    case prime_bmng_network_event_cmd:
        return _bmng_network_ind(ptrMsg,len);
        break;
    case prime_bmng_pprof_ack_cmd:
        return _bmng_pprof_ack_ind(ptrMsg,len);
        break;
    case prime_bmng_pprof_get_response_cmd :
        return _bmng_pprof_get_response_ind(ptrMsg,len);
        break;
    case prime_bmng_pprof_get_enhanced_response_cmd:
        return _bmng_pprof_get_enhanced_response_ind(ptrMsg,len);
        break;
    default:
        return false;
        break;
    }
}

void ifacePrime_api_init()
{
    memset(&sx_data_pointer_table, 0, sizeof(sx_data_pointer_table));
    memset(&sx_prime_api, 0, sizeof(sx_prime_api));
    suc_prime_app_id_req = PRIME_API_INVALID;

    /* register to USI callback */
    hal_usi_set_callback(PROTOCOL_PRIME_API, ifacePrime_api_ReceivedCmd);
}

void ifacePrime_select_api(uint8_t prime_app_id)
{
	if (prime_app_id < PRIME_API_MAX_NUM) {
		suc_prime_app_id_req = prime_app_id;
	} else {
		suc_prime_app_id_req = PRIME_API_INVALID;
	}
}
