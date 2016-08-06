#ifndef __ADP_API_TYPES_H__
#define __ADP_API_TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define ADP_ADDRESS_16BITS 2
#define ADP_ADDRESS_64BITS 8

struct TAdpExtendedAddress {
  uint8_t m_au8Value[8];
};

struct TAdpAddress {
  uint8_t m_u8AddrSize; // ADP_ADDRESS_16BITS(2) or ADP_ADDRESS_64BITS(8)
  union {
    uint16_t m_u16ShortAddr;
    struct TAdpExtendedAddress m_ExtendedAddress;
  };
};

enum TAdpBand {
  ADP_BAND_CENELEC_A = 0, ADP_BAND_CENELEC_B = 1, ADP_BAND_FCC = 2, ADP_BAND_ARIB = 3
};

/**********************************************************************************************************************/
/** PAN descriptor structure specification
 *
 ***********************************************************************************************************************
 * @param u16PanId The 16-bit PAN identifier.
 * @param u8LinkQuality The 8-bit link quality of LBA.
 * @param u16LbaAddress The 16 bit short address of a device in this PAN to be used as the LBA by the associating device.
 * @param u16RcCoord The estimated route cost from LBA to the coordinator.
 **********************************************************************************************************************/
struct TAdpPanDescriptor {
  uint16_t m_u16PanId;
  uint8_t m_u8LinkQuality;
  uint16_t m_u16LbaAddress;
  uint16_t m_u16RcCoord;
};

/**********************************************************************************************************************/
/** Path discovery
 *
 ***********************************************************************************************************************
 * @param m_u16HopAddress The hop / node address
 * @param m_u8Mns MetricNotSupported: 1 the metric type is not supported by the hop, 0 if supported
 * @param  m_u8LinkCost LinkCost of the node
 **********************************************************************************************************************/
struct THopDescriptor {
  uint16_t m_u16HopAddress;
  uint8_t m_u8Mns;
  uint8_t m_u8LinkCost;
};

/**********************************************************************************************************************/
/** Path discovery
 *
 ***********************************************************************************************************************
 * @param m_u16DstAddr The short unicast destination address of the path discovery.
 * @param m_u16ExpectedOrigAddr The expected originator of the path reply
 * @param m_u16OrigAddr The real originator of the path reply
 * @param m_u8MetricType Path metric type
 * @param m_u8ForwardHopsCount Number of path hops in the forward table
 * @param m_u8ReverseHopsCount Number of path hops in the reverse table
 * @param m_aForwardPath Table with the information of each hop in forward direction (according to m_u8ForwardHopsCount)
 * @param m_aReversePath Table with the information of each hop in reverse direction (according to m_u8ReverseHopsCount)
 **********************************************************************************************************************/
struct TPathDescriptor {
	uint16_t m_u16DstAddr;
	uint16_t m_u16ExpectedOrigAddr;
	uint16_t m_u16OrigAddr;
	uint8_t m_u8MetricType;
	uint8_t m_u8ForwardHopsCount;
	uint8_t m_u8ReverseHopsCount;
	struct THopDescriptor m_aForwardPath[16];
	struct THopDescriptor m_aReversePath[16];
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
	ADP_IB_MANUF_ADP_MAC_INTERNAL_VERSION = 0x800000CE

};

// List of errors returned by the ADP
enum EAdpStatus {
	/// Success
	G3_SUCCESS = 0x00,
	/// Invalid request
	G3_INVALID_REQUEST = 0xA1,
	/// Request failed
	G3_FAILED = 0xA2,
	/// Invalid IPv6 frame
	G3_INVALID_IPV6_FRAME = 0xA3,
	/// Not permited
	G3_NOT_PERMITED = 0xA4,
	/// No route to destination
	G3_ROUTE_ERROR = 0xA5,
	/// Operation timed out
	G3_TIMEOUT = 0xA6,
	/// An attempt to write to a MAC PIB attribute that is in a table failed because the specified table index was out of range.
	G3_INVALID_INDEX = 0xA7,
	/// A parameter in the primitive is either not supported or is out of the valid range.
	G3_INVALID_PARAMETER = 0xA8,
	/// A scan operation failed to find any network beacons.
	G3_NO_BEACON = 0xA9,
	/// A SET/GET request was issued with the identifier of an attribute that is read only.
	G3_READ_ONLY = 0xB0,
	/// A SET/GET request was issued with the identifier of a PIB attribute that is not supported.
	G3_UNSUPPORTED_ATTRIBUTE = 0xB1,
	/// The path discovery has only a part of the path to its desired final destination.
	G3_INCOMPLETE_PATH = 0xB2,
	/// Busy: operation already in progress.
	G3_BUSY = 0xB3,
	/// Not enough resources
	G3_NO_BUFFERS = 0xB4,
	/// Error internal
	G3_ERROR_INTERNAL = 0xFF
};

// List of ADP APIs
enum EAdpApi {
    SERIAL_MSG_ADP_STATUS = 0,
    SERIAL_MSG_ADP_INITIALIZE = 10,
    SERIAL_MSG_ADP_DATA_REQUEST = 11,
    SERIAL_MSG_ADP_DISCOVERY_REQUEST = 12,
    SERIAL_MSG_ADP_NETWORK_START_REQUEST = 13,
    SERIAL_MSG_ADP_NETWORK_JOIN_REQUEST = 14,
    SERIAL_MSG_ADP_NETWORK_LEAVE_REQUEST = 15,
    SERIAL_MSG_ADP_RESET_REQUEST = 16,
    SERIAL_MSG_ADP_SET_REQUEST = 17,
    SERIAL_MSG_ADP_GET_REQUEST = 18,
    SERIAL_MSG_ADP_LBP_REQUEST = 19,
    SERIAL_MSG_ADP_ROUTE_DISCOVERY_REQUEST = 20,
    SERIAL_MSG_ADP_PATH_DISCOVERY_REQUEST = 21,
    SERIAL_MSG_ADP_MAC_SET_REQUEST = 22,
    SERIAL_MSG_ADP_MAC_GET_REQUEST = 23,

	SERIAL_MSG_ADP_DATA_CONFIRM = 30,
	SERIAL_MSG_ADP_DATA_INDICATION = 31,
	SERIAL_MSG_ADP_NETWORK_STATUS_INDICATION = 32,
	SERIAL_MSG_ADP_DISCOVERY_CONFIRM = 33,
	SERIAL_MSG_ADP_NETWORK_START_CONFIRM = 34,
	SERIAL_MSG_ADP_NETWORK_JOIN_CONFIRM = 35,
	SERIAL_MSG_ADP_NETWORK_LEAVE_CONFIRM = 36,
	SERIAL_MSG_ADP_NETWORK_LEAVE_INDICATION = 37,
	SERIAL_MSG_ADP_RESET_CONFIRM = 38,
	SERIAL_MSG_ADP_SET_CONFIRM = 39,
	SERIAL_MSG_ADP_GET_CONFIRM = 40,
	SERIAL_MSG_ADP_LBP_CONFIRM = 41,
	SERIAL_MSG_ADP_LBP_INDICATION = 42,
	SERIAL_MSG_ADP_ROUTE_DISCOVERY_CONFIRM = 43,
	SERIAL_MSG_ADP_PATH_DISCOVERY_CONFIRM = 44,
	SERIAL_MSG_ADP_MAC_SET_CONFIRM = 45,
	SERIAL_MSG_ADP_MAC_GET_CONFIRM = 46,
	SERIAL_MSG_ADP_BUFFER_INDICATION = 47,
	SERIAL_MSG_ADP_DISCOVERY_INDICATION = 48
};

#endif
