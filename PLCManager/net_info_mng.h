#ifndef NET_INFO_MNG_H_INCLUDED
#define NET_INFO_MNG_H_INCLUDED

#include "socket_handler.h"

//#define NET_INFO_DEBUG_CONSOLE

#define MAX_NET_INFO_SOCKET_SIZE      1000

#define RESET_GPIO_ID          4  // S19:PA4
#define ERASE_GPIO_ID          2  // S21:PA2

#define RESET_GPIO_ENABLE      0
#define RESET_GPIO_DISABLE     1

#define ERASE_GPIO_ENABLE      1
#define ERASE_GPIO_DISABLE     0

typedef struct sx_coord_data {
	bool b_is_valid;
	uint8_t puc_ext_addr[8];
} x_coord_data_t;

typedef struct {
	uint16_t us_short_addr;
	uint8_t puc_ext_addr[8];
} x_dev_addr_t;

enum net_info_mng_webcmds {
	WEBCMD_UPDATE_DASHBOARD = 0x30,
	WEBCMD_INVALD
};

enum net_info_mng_lnxcmds {
	LNXCMS_UPDATE_DASHBOARD = 0x40,
	LNXCMS_UPDATE_PATHLIST,
	LNXCMS_UPDATE_ROUNDTIME,
	LNXCMS_UPDATE_THROUGHPUT,
	LNXCMS_INVALD
};

enum net_info_adp_event_id
{
	NET_INFO_ADP_DATA_REQ = 1,
	NET_INFO_ADP_DATA_CFM,
	NET_INFO_ADP_DATA_IND,
	NET_INFO_ADP_DISCOVERY_REQ,
	NET_INFO_ADP_DISCOVERY_CFM,
	NET_INFO_ADP_DISCOVERY_IND,
	NET_INFO_ADP_NET_START_REQ,
	NET_INFO_ADP_NET_START_CFM,
	NET_INFO_ADP_JOIN_REQ,
	NET_INFO_ADP_JOIN_CFM,
	NET_INFO_ADP_JOIN_IND,
	NET_INFO_ADP_LEAVE_REQ,
	NET_INFO_ADP_LEAVE_CFM,
	NET_INFO_ADP_LEAVE_IND,
	NET_INFO_ADP_RESET_REQ,
	NET_INFO_ADP_RESET_CFM,
	NET_INFO_ADP_NET_STATUS_IND,
	NET_INFO_ADP_BUFFER_IND,
	NET_INFO_ADP_ROUTE_DISCOVERY_REQ,
	NET_INFO_ADP_ROUTE_DISCOVERY_CFM,
	NET_INFO_ADP_PATH_DISCOVERY_REQ,
	NET_INFO_ADP_PATH_DISCOVERY_CFM,
	NET_INFO_ADP_LBP_REQ,
	NET_INFO_ADP_LBP_CFM,
	NET_INFO_ADP_LBP_IND,
	NET_INFO_ADP_SET_CFM,
	NET_INFO_ADP_MAC_SET_CFM,
	NET_INFO_ADP_GET_CFM,
	NET_INFO_ADP_MAC_GET_CFM
};

// List of ADP supported MIB attributes.
enum EAdpPibAttribute {
	ADP_IB_SECURITY_LEVEL = 0x00000000,
	ADP_IB_PREFIX_TABLE = 0x00000001,
	ADP_IB_BROADCAST_LOG_TABLE_ENTRY_TTL = 0x00000002,
	ADP_IB_METRIC_TYPE = 0x00000003,
	ADP_IB_LOW_LQI_VALUE = 0x00000004,
	ADP_IB_HIGH_LQI_VALUE = 0x00000005,
	ADP_IB_RREP_WAIT = 0x00000006,
	ADP_IB_CONTEXT_INFORMATION_TABLE = 0x00000007,
	ADP_IB_COORD_SHORT_ADDRESS = 0x00000008,
	ADP_IB_RLC_ENABLED = 0x00000009,
	ADP_IB_ADD_REV_LINK_COST = 0x0000000A,
	ADP_IB_BROADCAST_LOG_TABLE = 0x0000000B,
	ADP_IB_ROUTING_TABLE = 0x0000000C,
	ADP_IB_UNICAST_RREQ_GEN_ENABLE = 0x0000000D,
	ADP_IB_GROUP_TABLE = 0x0000000E,
	ADP_IB_MAX_HOPS = 0x0000000F,
	ADP_IB_DEVICE_TYPE = 0x00000010,
	ADP_IB_NET_TRAVERSAL_TIME = 0x00000011,
	ADP_IB_ROUTING_TABLE_ENTRY_TTL = 0x00000012,
	ADP_IB_KR = 0x00000013,
	ADP_IB_KM = 0x00000014,
	ADP_IB_KC = 0x00000015,
	ADP_IB_KQ = 0x00000016,
	ADP_IB_KH = 0x00000017,
	ADP_IB_RREQ_RETRIES = 0x00000018,
	ADP_IB_RREQ_RERR_WAIT = 0x00000019,
	ADP_IB_WEAK_LQI_VALUE = 0x0000001A,
	ADP_IB_KRT = 0x0000001B,
	ADP_IB_SOFT_VERSION = 0x0000001C,
	ADP_IB_SNIFFER_MODE = 0x0000001D,
	ADP_IB_BLACKLIST_TABLE = 0x0000001E,
	ADP_IB_BLACKLIST_TABLE_ENTRY_TTL = 0x0000001F,
	ADP_IB_MAX_JOIN_WAIT_TIME = 0x00000020,
	ADP_IB_PATH_DISCOVERY_TIME = 0x00000021,
	ADP_IB_ACTIVE_KEY_INDEX = 0x00000022,
	ADP_IB_DISABLE_DEFAULT_ROUTING = 0x000000F0,
	// manufacturer
	ADP_IB_MANUF_REASSEMBY_TIMER = 0x800000C0,
	ADP_IB_MANUF_IPV6_HEADER_COMPRESSION = 0x800000C1,
	ADP_IB_MANUF_EAP_PRESHARED_KEY = 0x800000C2,
	ADP_IB_MANUF_EAP_NETWORK_ACCESS_IDENTIFIER = 0x800000C3,
	ADP_IB_MANUF_BROADCAST_SEQUENCE_NUMBER = 0x800000C4,
	ADP_IB_MANUF_REGISTER_DEVICE = 0x800000C5,
	ADP_IB_MANUF_DATAGRAM_TAG = 0x800000C6,
	ADP_IB_MANUF_RANDP = 0x800000C7,
	ADP_IB_MANUF_ROUTING_TABLE_COUNT = 0x800000C8,
	ADP_IB_MANUF_DISCOVER_SEQUENCE_NUMBER = 0x800000C9,
	ADP_IB_MANUF_FORCED_NO_ACK_REQUEST = 0x800000CA,
	ADP_IB_MANUF_LQI_TO_COORD = 0x800000CB,
	ADP_IB_MANUF_BROADCAST_ROUTE_ALL = 0x800000CC,
	ADP_IB_MANUF_KEEP_PARAMS_AFTER_KICK_LEAVE = 0x800000CD,
	ADP_IB_MANUF_ADP_MAC_INTERNAL_VERSION = 0x800000CE,
	ADP_IB_MANUF_CIRCULAR_ROUTES_DETECTED = 0x800000CF,
	ADP_IB_MANUF_LAST_CIRCULAR_ROUTE_ADDRESS = 0x800000D0,
	ADP_IB_MANUF_IPV6_ULA_DEST_SHORT_ADDRESS = 0x800000D1,
	ADP_IB_MANUF_MAX_REPAIR_RESEND_ATTEMPTS = 0x800000D2,
	ADP_IB_MANUF_DISABLE_AUTO_RREQ = 0x800000D3,
	ADP_IB_MANUF_ALL_NEIGHBORS_BLACKLISTED_COUNT = 0x800000D5,
	ADP_IB_MANUF_QUEUED_ENTRIES_REMOVED_TIMEOUT_COUNT = 0x800000D6,
	ADP_IB_MANUF_QUEUED_ENTRIES_REMOVED_ROUTE_ERROR_COUNT = 0x800000D7
};

// List of ADP MAC supported MIB attributes.
enum EMacPibAttribute {
  MAC_PIB_ACK_WAIT_DURATION = 0x00000040,
  MAC_PIB_BSN = 0x00000049,
  MAC_PIB_DSN = 0x0000004C,
  MAC_PIB_MAX_BE = 0x00000047,
  MAC_PIB_MAX_CSMA_BACKOFFS = 0x0000004E,
  MAC_PIB_MAX_FRAME_RETRIES = 0x00000059,
  MAC_PIB_MIN_BE = 0x0000004F,
  MAC_PIB_PAN_ID = 0x00000050,
  MAC_PIB_SECURITY_ENABLED = 0x0000005D,
  MAC_PIB_SHORT_ADDRESS = 0x00000053,
  MAC_PIB_PROMISCUOUS_MODE = 0x00000051,
  MAC_PIB_TIMESTAMP_SUPPORTED = 0x0000005C,
  MAC_PIB_KEY_TABLE = 0x00000071,
  MAC_PIB_FRAME_COUNTER = 0x00000077,
  MAC_PIB_HIGH_PRIORITY_WINDOW_SIZE = 0x00000100,
  MAC_PIB_TX_DATA_PACKET_COUNT = 0x00000101,
  MAC_PIB_RX_DATA_PACKET_COUNT = 0x00000102,
  MAC_PIB_TX_CMD_PACKET_COUNT = 0x00000103,
  MAC_PIB_RX_CMD_PACKET_COUNT = 0x00000104,
  MAC_PIB_CSMA_FAIL_COUNT = 0x00000105,
  MAC_PIB_CSMA_NO_ACK_COUNT = 0x00000106,
  MAC_PIB_RX_DATA_BROADCAST_COUNT = 0x00000107,
  MAC_PIB_TX_DATA_BROADCAST_COUNT = 0x00000108,
  MAC_PIB_BAD_CRC_COUNT = 0x00000109,
  MAC_PIB_NEIGHBOUR_TABLE = 0x0000010A,
  MAC_PIB_FREQ_NOTCHING = 0x0000010B,
  MAC_PIB_CSMA_FAIRNESS_LIMIT = 0x0000010C,
  MAC_PIB_TMR_TTL = 0x0000010D,
  MAC_PIB_NEIGHBOUR_TABLE_ENTRY_TTL = 0x0000010E,
  MAC_PIB_RC_COORD = 0x0000010F,
  MAC_PIB_TONE_MASK = 0x00000110,
  MAC_PIB_BEACON_RANDOMIZATION_WINDOW_LENGTH = 0x00000111,
  MAC_PIB_A = 0x00000112,
  MAC_PIB_K = 0x00000113,
  MAC_PIB_MIN_CW_ATTEMPTS = 0x00000114,
  MAC_PIB_CENELEC_LEGACY_MODE = 0x00000115,
  MAC_PIB_FCC_LEGACY_MODE = 0x00000116,
  // manufacturer specific
  // provides access to device table
  MAC_PIB_MANUF_DEVICE_TABLE = 0x80000000,
  // Extended address of this node.
  MAC_PIB_MANUF_EXTENDED_ADDRESS = 0x80000001,
  // provides access to neighbour table by short address (transmitted as index)
  MAC_PIB_MANUF_NEIGHBOUR_TABLE_ELEMENT = 0x80000002,
  // returns the maximum number of tones used by the band
  MAC_PIB_MANUF_BAND_INFORMATION = 0x80000003,
  // Short address of the coordinator.
  MAC_PIB_MANUF_COORD_SHORT_ADDRESS = 0x08000004,
  // Maximal payload supported by MAC.
  MAC_PIB_MANUF_MAX_MAC_PAYLOAD_SIZE = 0x08000005,
  // Resets the device table upon a GMK activation.
  MAC_PIB_MANUF_SECURITY_RESET = 0x08000006,
  // Forces Modulation Scheme in every transmitted frame
  // 0 - Not forced, 1 - Force Differential, 2 - Force Coherent
  MAC_PIB_MANUF_FORCED_MOD_SCHEME = 0x08000007,
  // Forces Modulation Type in every transmitted frame
  // 0 - Not forced, 1 - Force BPSK_ROBO, 2 - Force BPSK, 3 - Force QPSK, 4 - Force 8PSK
  MAC_PIB_MANUF_FORCED_MOD_TYPE = 0x08000008,
  // Forces ToneMap in every transmitted frame
  // {0} - Not forced, other value will be used as tonemap
  MAC_PIB_MANUF_FORCED_TONEMAP = 0x08000009,
  // Forces Modulation Scheme bit in Tone Map Response
  // 0 - Not forced, 1 - Force Differential, 2 - Force Coherent
  MAC_PIB_MANUF_FORCED_MOD_SCHEME_ON_TMRESPONSE = 0x0800000A,
  // Forces Modulation Type bits in Tone Map Response
  // 0 - Not forced, 1 - Force BPSK_ROBO, 2 - Force BPSK, 3 - Force QPSK, 4 - Force 8PSK
  MAC_PIB_MANUF_FORCED_MOD_TYPE_ON_TMRESPONSE = 0x0800000B,
  // Forces ToneMap field Tone Map Response
  // {0} - Not forced, other value will be used as tonemap field
  MAC_PIB_MANUF_FORCED_TONEMAP_ON_TMRESPONSE = 0x0800000C,
  // Gets Modulation Scheme of last received frame
  MAC_PIB_MANUF_LAST_RX_MOD_SCHEME = 0x0800000D,
  // Gets Modulation Scheme of last received frame
  MAC_PIB_MANUF_LAST_RX_MOD_TYPE = 0x0800000E,
  // Indicates whether an LBP frame for other destination has been received
  MAC_PIB_MANUF_LBP_FRAME_RECEIVED = 0x0800000F,
  // Indicates whether an LBP frame for other destination has been received
  MAC_PIB_MANUF_LNG_FRAME_RECEIVED = 0x08000010,
  // Indicates whether an Beacon frame from other nodes has been received
  MAC_PIB_MANUF_BCN_FRAME_RECEIVED = 0x08000011,
  // Gets number of valid elements in the Neighbour Table
  MAC_PIB_MANUF_NEIGHBOUR_TABLE_COUNT = 0x08000012,
  // Gets number of discarded packets due to Other Destination
  MAC_PIB_MANUF_RX_OTHER_DESTINATION_COUNT = 0x08000013,
  // Gets number of discarded packets due to Invalid Frame Lenght
  MAC_PIB_MANUF_RX_INVALID_FRAME_LENGTH_COUNT = 0x08000014,
  // Gets number of discarded packets due to MAC Repetition
  MAC_PIB_MANUF_RX_MAC_REPETITION_COUNT = 0x08000015,
  // Gets number of discarded packets due to Wrong Addressing Mode
  MAC_PIB_MANUF_RX_WRONG_ADDR_MODE_COUNT = 0x08000016,
  // Gets number of discarded packets due to Unsupported Security
  MAC_PIB_MANUF_RX_UNSUPPORTED_SECURITY_COUNT = 0x08000017,
  // Gets number of discarded packets due to Wrong Key Id
  MAC_PIB_MANUF_RX_WRONG_KEY_ID_COUNT = 0x08000018,
  // Gets number of discarded packets due to Invalid Key
  MAC_PIB_MANUF_RX_INVALID_KEY_COUNT = 0x08000019,
  // Gets number of discarded packets due to Wrong Frame Counter
  MAC_PIB_MANUF_RX_WRONG_FC_COUNT = 0x0800001A,
  // Gets number of discarded packets due to Decryption Error
  MAC_PIB_MANUF_RX_DECRYPTION_ERROR_COUNT = 0x0800001B,
  // Gets number of discarded packets due to Segment Decode Error
  MAC_PIB_MANUF_RX_SEGMENT_DECODE_ERROR_COUNT = 0x0800001C,
  // Enables MAC Sniffer
  MAC_PIB_MANUF_ENABLE_MAC_SNIFFER = 0x0800001D,
  // Gets or sets a parameter in Phy layer. Index will be used to contain PHY parameter ID.
  // Check 'enum EPhyParam' below for available Phy parameter IDs
  MAC_PIB_MANUF_PHY_PARAM = 0x08000020
};

void net_info_mng_init(int _app_id);
void net_info_mng_process(void);
//void net_info_mng_callback(socket_ev_info_t *_ev_info);
void net_info_webcmd_process(uint8_t* buf);

#endif /* NET_INFO_MNG_H_INCLUDED */
