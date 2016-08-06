
#ifndef BS_FUNCTIONS_H
#define BS_FUNCTIONS_H

#include <stdbool.h>

#define BS_MAX_JOIN_TIME  250000

//#define LOG_BOOTSTRAP(a)   printf a
#define LOG_BOOTSTRAP(a)   (void)0

/* PAN ID */
extern uint16_t g_u16NetworkId;

struct TAddress
{
	uint8_t m_u8AddrLength;
	union {
		uint16_t m_u16ShortAddr;
		uint8_t m_u8ExtendedAddr[8];
	};
};

// Status codes related to HostInterface processing
enum ELbsStatus
{
  LBS_STATUS_SUCCESS = 0,
  LBS_STATUS_NOT_ALLOWED,
  LBS_STATUS_UNKNOWN_COMMAND,
  LBS_STATUS_INVALID_PARAMETER
};

// Indications to be notified to the upper layers
enum lbp_indications
{
  LBS_NONE = 0,
  LBS_KICK
};

#define MAC_SET_REQUEST_VALUE_LEN    16
struct TCbMsgMacSetRequest {
	uint32_t m_u32AttributeId;
	uint16_t m_u16AttributeIndex;
	uint8_t m_u8AttributeLength;
	uint8_t m_pu8AttributeValue[MAC_SET_REQUEST_VALUE_LEN];
};

// HiMsgMacSetConfirm parameters definition
struct TCbMsgMacSetConfirm {
	uint8_t m_u8Status;
	uint32_t m_u32AttributeId;
	uint16_t m_u16AttributeIndex;
};

struct TCbMsgAdpInitializeConfirm {
        uint8_t m_u8Status;
};

struct TCbMsgAdpNetworkStartRequest {
	uint16_t m_u16PanId;
};

#define ADP_SET_REQUEST_VALUE_LEN    28
struct TCbMsgAdpSetRequest {
	uint32_t m_u32AttributeId;
	uint16_t m_u16AttributeIndex;
	uint8_t m_u8AttributeLength;
	uint8_t m_pu8AttributeValue[ADP_SET_REQUEST_VALUE_LEN];
};

#define APD_LBP_REQUEST_BUFF_LEN  104 // Max.nsdu=96, max.AddrLen=8.
struct TCbMsgAdpLbpRequest {
	uint8_t m_u8NsduHandle;
	uint8_t m_u8MaxHops;
	bool m_bDiscoverRoute;
	uint8_t m_u8QualityOfService;
	bool m_bSecurityEnable;
	uint8_t m_u8DstAddrLength; // 2 or 8 bytes
	uint16_t m_u16NsduLength;
	uint8_t m_buffer[APD_LBP_REQUEST_BUFF_LEN];
};

// HiMsgAdpDataConfirm parameters definition
struct TCbMsgAdpDataConfirm {
	uint8_t m_u8Status;
	uint8_t m_u8NsduHandle;
};

struct TCbMsgMacResetConfirm {
	uint8_t m_u8Status;
};

// HiMsgAdpNetworkStartConfirm parameters definition
struct TCbMsgAdpNetworkStartConfirm {
	uint8_t m_u8Status;
};

// HiMsgAdpSetConfirm parameters definition
struct TCbMsgAdpSetConfirm {
	uint8_t m_u8Status;
	uint32_t m_u32AttributeId;
	uint16_t m_u16AttributeIndex;
};

// HiMsgAdpLbpConfirm parameters definition
struct TCbMsgAdpLbpConfirm {
	uint8_t m_u8Status;
	uint8_t m_u8NsduHandle;
};

typedef struct{
    unsigned char u8EAPIdentifier;
    struct TEapPskContext g_PskContext;
    uint16_t currentShortAddr;
    uint16_t initialShortAddr;
    uint8_t extAddr[8];
    uint8_t pending;
} t_context;

// Bitmap definition for us_bitmap
#define LBDS_LIST_LBD_ACTIVE 0x01
// In the device list (g_lbds_list), the short address is specified by the
// index in the list plus an offste given by initialShortAddr.
// are allowed.
typedef struct {
	uint8_t puc_extended_address[ADP_ADDRESS_64BITS];
} lbds_list_entry_t;

#define LBP_REKEYING_ON    1
#define LBP_REKEYING_OFF   0
#define LBP_REKEYING_PHASE_DISTRIBUTE    0
#define LBP_REKEYING_PHASE_ACTIVATE      1

uint16_t get_lbds_count(void);
bool is_null_address(uint8_t * puc_extended_address);
uint16_t get_lbd_address(uint16_t i);
uint8_t device_is_in_list(uint16_t us_short_address);
void remove_lbds_list_entry(uint16_t us_short_address);
void activate_lbds_list_entry(uint16_t us_short_address);
uint16_t get_new_address(void);
bool add_lbds_list_entry(const uint8_t *puc_extended_address, uint16_t us_short_address);
bool get_ext_addr_by_short (uint16_t us_short_address, uint8_t *puc_extended_address);
bool get_short_addr_by_ext (uint8_t *puc_extended_address, uint16_t *pus_short_address);

void _lbp_device_accepted(struct TAdpExtendedAddress *p_au8LBPEUI64Address);
t_context *_add_context(void);
t_context *_get_context(void);

void registered_cb(void);

void lbp_init_functions(void);
void initialize_bootstrap_message(void);
uint8_t add_to_blacklist(uint8_t *puc_address);
uint8_t remove_from_blacklist(uint16_t us_index);
enum lbp_indications ProcessLBPMessage(unsigned char *pNsdu, unsigned short u16NsduLen);
void Process_Joining0(struct TAdpExtendedAddress pLBPEUI64Address);
uint8_t process_accepted_GMK_activation(struct TAdpExtendedAddress au8LBPEUI64Address);
uint16_t Encode_kick_to_LBD(uint8_t *p_ext_addr, uint16_t u16MessageLength, uint8_t *pMessageBuffer);
uint16_t Encode_decline(uint8_t *p_ext_addr);
uint16_t get_current_short_address(void);
void set_current_short_address(uint16_t us_short_addr);
uint16_t get_initial_short_address(void);
bool set_initial_short_address(uint16_t us_short_addr);
unsigned char *GetData(void);
unsigned short GetLength(void);
struct TAdpExtendedAddress *get_current_ext_address(void);
const uint8_t *GetGMK(void);
void set_gmk(uint8_t *puc_new_gmk);
void set_rekey_gmk(uint8_t *puc_new_gmk);
void set_ids(const uint8_t *puc_ids, uint8_t uc_size);
void set_psk(uint8_t *puc_new_psk);
void lbp_set_rekeying(uint8_t on_off);
uint16_t lbp_get_rekeying(void);

#endif /* BS_FUNCTIONS_H */
