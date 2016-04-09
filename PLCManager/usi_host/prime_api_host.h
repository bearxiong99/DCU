#ifndef PRIME_API_H
#define PRIME_API_H

#include "mac_defs.h"
#include "cl_432_defs.h"
#include "prime_api_defs_host.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LEN_ID_EUI48    6

/* \name PRIME API commands */
/* @{ */
typedef enum {
    prime_cl_null_establish_request_cmd         = 0x01,
        prime_cl_null_establish_ind_cmd         = 0x02,
        prime_cl_null_establish_cfm_cmd         = 0x03,
        prime_cl_null_establish_response_cmd 	= 0x04,
        prime_cl_null_release_request_cmd       = 0x05,
        prime_cl_null_release_ind_cmd           = 0x06,
        prime_cl_null_release_cfm_cmd           = 0x07,
        prime_cl_null_release_response_cmd      = 0x08,
        prime_cl_null_join_request_cmd          = 0x09,
        prime_cl_null_join_ind_cmd              = 0x0A,
        prime_cl_null_join_response_cmd         = 0x0B,
        prime_cl_null_join_cfm_cmd              = 0x0C,
        prime_cl_null_leave_request_cmd         = 0x0D,
        prime_cl_null_leave_cfm_cmd             = 0x0E,
        prime_cl_null_leave_ind_cmd             = 0x0F,
        prime_cl_null_data_request_cmd          = 0x10,
        prime_cl_null_data_cfm_cmd              = 0x11,
        prime_cl_null_data_ind_cmd              = 0x12,
        prime_cl_null_plme_reset_request_cmd 	= 0x13,
        prime_cl_null_plme_reset_cfm_cmd        = 0x14,
        prime_cl_null_plme_sleep_request_cmd 	= 0x15,
        prime_cl_null_plme_sleep_cfm_cmd        = 0x16,
        prime_cl_null_plme_resume_request_cmd 	= 0x17,
        prime_cl_null_plme_resume_cfm_cmd       = 0x18,
        prime_cl_null_plme_testmode_request_cmd = 0x19,
        prime_cl_null_plme_testmode_cfm_cmd 	= 0x1A,
        prime_cl_null_plme_get_request_cmd      = 0x1B,
        prime_cl_null_plme_get_cfm_cmd          = 0x1C,
        prime_cl_null_plme_set_request_cmd      = 0x1D,
        prime_cl_null_plme_set_cfm_cmd          = 0x1E,
        prime_cl_null_mlme_register_request_cmd = 0x1F,
        prime_cl_null_mlme_register_cfm_cmd 	= 0x20,
        prime_cl_null_mlme_register_ind_cmd 	= 0x21,
        prime_cl_null_mlme_unregister_request_cmd = 0x22,
        prime_cl_null_mlme_unregister_cfm_cmd 	= 0x23,
        prime_cl_null_mlme_unregister_ind_cmd 	= 0x24,
        prime_cl_null_mlme_promote_request_cmd 	= 0x25,
        prime_cl_null_mlme_promote_cfm_cmd      = 0x26,
        prime_cl_null_mlme_promote_ind_cmd      = 0x27,
        prime_cl_null_mlme_demote_request_cmd 	= 0x28,
        prime_cl_null_mlme_demote_cfm_cmd       = 0x29,
        prime_cl_null_mlme_demote_ind_cmd       = 0x2A,
        prime_cl_null_mlme_reset_request_cmd 	= 0x2B,
        prime_cl_null_mlme_reset_cfm_cmd        = 0x2C,
        prime_cl_null_mlme_get_request_cmd      = 0x2D,
        prime_cl_null_mlme_get_cfm_cmd          = 0x2E,
        prime_cl_null_mlme_list_get_request_cmd = 0x2F,
        prime_cl_null_mlme_list_get_cfm_cmd 	= 0x30,
        prime_cl_null_mlme_set_request_cmd      = 0x31,
        prime_cl_null_mlme_set_cfm_cmd          = 0x32,
        prime_cl_432_establish_request_cmd      = 0x33,
        prime_cl_432_establish_cfm_cmd          = 0x34,
        prime_cl_432_release_request_cmd        = 0x35,
        prime_cl_432_release_cfm_cmd            = 0x36,
        prime_cl_432_dl_data_request_cmd        = 0x37,
        prime_cl_432_dl_data_ind_cmd            = 0x38,
        prime_cl_432_dl_data_cfm_cmd            = 0x39,
        prime_cl_432_dl_join_ind_cmd            = 0x3A,
        prime_cl_432_dl_leave_ind_cmd           = 0x3B,
        prime_cl_null_redirect_response_cmd 	= 0x3C,
        prime_bmng_fup_clear_target_list_request_cmd = 0x3D,
        prime_bmng_fup_add_target_request_cmd 	= 0x3E,
        prime_bmng_fup_set_fw_data_request_cmd 	= 0x3F,
        prime_bmng_fup_set_upg_options_request_cmd = 0x40,
        prime_bmng_fup_init_file_tx_request_cmd = 0x41,
        prime_bmng_fup_data_frame_request_cmd 	= 0x42,
        prime_bmng_fup_check_crc_request_cmd 	= 0x43,
        prime_bmng_fup_abort_fu_request_cmd 	= 0x44,
        prime_bmng_fup_start_fu_request_cmd 	= 0x45,
        prime_bmng_fup_set_match_rule_request_cmd = 0x46,
        prime_bmng_fup_get_version_request_cmd 	= 0x47,
        prime_bmng_fup_get_state_request_cmd 	= 0x48,
        prime_bmng_fup_ack_cmd                  = 0x49,
        prime_bmng_fup_status_ind_cmd           = 0x4A,
        prime_bmng_fup_error_ind_cmd            = 0x4B,
        prime_bmng_fup_version_ind_cmd          = 0x4C,
        prime_bmng_fup_kill_ind_cmd             = 0x4D,
        prime_bmng_fup_set_signature_data_request_cmd = 0x4E,
        prime_bmng_network_event_cmd            = 0x4F,
        prime_bmng_pprof_get_request_cmd        = 0x50,
        prime_bmng_pprof_set_request_cmd        = 0x51,
        prime_bmng_pprof_reset_request_cmd      = 0x52,
        prime_bmng_pprof_reboot_request_cmd     = 0x53,
        prime_bmng_pprof_get_enhanced_request_cmd = 0x54,
        prime_bmng_pprof_ack_cmd                = 0x55,
        prime_bmng_pprof_get_response_cmd       = 0x56,
        prime_bmng_pprof_get_enhanced_response_cmd = 0x57,
        PRIME_API_CMD_ERROR
} prime_api_cmd_t;
/* @} */


/* \name CL NULL functions interface */
/* @{ */

/* CL NULL set callback function */
void prime_cl_null_set_callbacks(uint8_t prime_app_id, prime_cl_null_callbacks_t *px_prime_cbs);

/* CL NULL_ESTABLISH_PRIMITIVES */
void prime_cl_null_establish_request(uint8_t *puc_eui48, uint8_t uc_type, uint8_t *puc_data, uint16_t us_data_len, uint8_t uc_arq, uint8_t uc_cfbytes);
void prime_cl_null_establish_response(uint16_t us_con_handle, mac_establish_response_answer_t uc_answer, uint8_t *puc_data, uint16_t us_data_len);
void prime_cl_null_redirect_response(uint16_t us_con_handle, uint8_t *puc_eui48, uint8_t *puc_data, uint16_t us_data_len );

/* CL NULL_RELEASE_PRIMITIVES */
void prime_cl_null_release_request(uint16_t us_con_handle);
void prime_cl_null_release_response(uint16_t us_con_handle, mac_release_response_answer_t uc_answer);

/* CL NULL JOIN PRIMITIVES */
void prime_cl_null_join_request(mac_join_mode_t us_broadcast, uint16_t us_con_handle, uint8_t *puc_eui48, connection_type_t uc_con_type,
                                uint8_t *puc_data, uint16_t us_data_len);
void prime_cl_null_join_response(uint16_t us_con_handle, uint8_t *puc_eui48, mac_join_response_answer_t uc_answer);

/* CL NULL LEAVE PRIMITIVES */
void prime_cl_null_leave_request(uint16_t us_con_handle, uint8_t *puc_eui48);

/* CL NULL DATA PRIMITIVES */
void prime_cl_null_data_request(uint16_t us_con_handle, uint8_t *puc_data, uint16_t us_data_len, uint8_t uc_prio);

/* CL NULL PLME PRIMITIVES */
void prime_cl_null_plme_reset_request(void);
void prime_cl_null_plme_sleep_request(void);
void prime_cl_null_plme_resume_request(void);
void prime_cl_null_plme_testmode_request(uint8_t uc_enable, uint8_t uc_mode, uint8_t uc_modulation, uint8_t uc_pwr_level);
void prime_cl_null_plme_get_request(uint16_t us_pib_attrib);
void prime_cl_null_plme_set_request(uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size);


/* CL NULL MLME PRIMITIVES */
void prime_cl_null_mlme_register_request(uint8_t *puc_sna, uint8_t uc_sid);
void prime_cl_null_mlme_unregister_request(void);
void prime_cl_null_mlme_unregister_request_base(unsigned char * _pucEui48);
void prime_cl_null_mlme_promote_request(void);
void prime_cl_null_mlme_demote_request(void);
void prime_cl_null_mlme_reset_request(void);
void prime_cl_null_mlme_get_request(uint16_t us_pib_attrib);
void prime_cl_null_mlme_list_get_request(uint16_t us_pib_attrib);
void prime_cl_null_mlme_set_request(uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size);
void prime_cl_null_mlme_action_request(uint16_t us_pib_attrib, void *pv_pib_value, uint8_t uc_pib_size);
/* @} */

/* \name DL432 interface */
/* @{ */
/* CL NULL set callback function */
void prime_cl_432_set_callbacks(uint8_t prime_app_id, prime_cl_432_callbacks_t *px_prime_cbs);

void prime_cl_432_establish_request(uint8_t *puc_device_id, uint8_t uc_device_id_len);
void prime_cl_432_release_request(uint16_t us_dst_address);
void prime_cl_432_dl_data_request(uint8_t uc_dst_lsap, uint8_t uc_src_lsap, uint16_t us_dst_address, dl_432_buffer_t *px_buff, uint16_t us_lsdu_len,
        uint8_t uc_link_class);
void prime_cl_432_dl_set_data_ind_cb(void (*sap_handler)(uint8_t uc_dst_lsap, uint8_t uc_src_lsap, uint16_t us_dst_address, uint16_t src_address, uint8_t *data,
        uint16_t us_lsdu_len, uint8_t uc_link_class));
void set_cl_432_dl_join_ind_cb (prime_cl_432_dl_join_ind_cb_t _fn_cb);
void set_cl_432_dl_leave_ind_cb (prime_cl_432_dl_leave_ind_cb_t _fn_cb);
/* @} */




/* \name Base Management interface */
/* @{ */
/* CL NULL set callback function */
void bmng_set_callbacks(uint8_t prime_app_id, prime_bmng_callbacks_t *px_fup_cbs);

void bmng_fup_clear_target_list_request();
void bmng_fup_add_target_request(uint8_t * puc_eui48);
void bmng_fup_set_fw_data_request(uint8_t  uc_vendor_len, uint8_t * puc_vendor, uint8_t  uc_model_len, uint8_t * puc_model,uint8_t  uc_version_len, uint8_t * puc_version  );
void bmng_fup_set_upg_options_request(uint8_t uc_arq_en, uint8_t uc_page_size, uint8_t uc_mult_en, uint32_t ui_delay, uint32_t ui_timer);
void bmng_fup_init_file_tx_request(uint16_t us_num_frames, uint32_t ui_file_size, uint16_t us_frame_size, uint32_t ui_crc);
void bmng_fup_data_frame_request(uint16_t us_frame_num, uint16_t us_len, uint8_t *puc_data);
void bmng_fup_check_crc_request();
void bmng_fup_abort_fu_request(uint8_t * puc_eui48);
void bmng_fup_start_fu_request(uint8_t uc_enable);
void bmng_fup_set_match_rule_request(uint8_t uc_rule);
void bmng_fup_get_version_request(uint8_t * puc_eui48);
void bmng_fup_get_state_request(uint8_t * puc_eui48);
void bmng_fup_ack_ind(void (*sap_ack_handler)(uint8_t uc_cmd, uint8_t uc_ack, uint16_t us_data));
void bmng_fup_status_ind(void (*sap_status_handler)(uint8_t uc_status, uint32_t uc_pages, uint8_t* puc_eui48));
void bmng_fup_error_ind(void (*sap_error_handler)(uint8_t uc_error_code, uint8_t* puc_eui48));
void bmng_fup_version_ind(void (*sap_version_handler)(uint8_t* puc_eui48,
                                                       uint8_t uc_vendor_len, uint8_t * puc_vendor,
                                                       uint8_t uc_model_len,  uint8_t * puc_model,
                                                       uint8_t uc_version_len, uint8_t * puc_version ));
void bmng_fup_kill_ind(void (*sap_kill_handler)(uint8_t* puc_eui48));
void bmng_fup_signature_cfg_cmd(uint8_t uc_sig_algo, uint16_t us_sig_len );
void bmng_network_event_ind(void(*sap_net_event_handler)(bmng_net_event_t *px_net_event));

void bmng_pprof_get_request(uint8_t * puc_eui48, uint16_t us_pib, uint8_t uc_index);
void bmng_pprof_set_request(uint8_t * puc_eui48, uint16_t us_pib, uint32_t ul_value);
void bmng_pprof_reboot(uint8_t * puc_eui48);
void bmng_pprof_reset(uint8_t * puc_eui48);
void bmng_pprof_get_enhanced(uint8_t * puc_eui48,uint16_t us_pib,uint8_t uc_max_records,uint16_t us_iterator);
/* @} */



/*Received command function. It calls the appropiate callback*/
uint8_t ifacePrime_api_ReceivedCmd(uint8_t* ptrMsg, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // PRIME_API_H
