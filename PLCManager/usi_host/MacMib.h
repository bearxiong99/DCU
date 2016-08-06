#ifndef MAC_MIB_H_
#define MAC_MIB_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <MacDefs.h>
#include <pal.h>
#include "config.h"

#define MAC_MAX_NEIGHBOUR_TABLE_ENTRIES     G3_MAX_DEV
#define MAC_MAX_DEVICE_TABLE_ENTRIES        (128)
#define MAC_KEY_TABLE_ENTRIES               (2)
#define MAC_INTERNAL_DATA_ENTRIES           (2)

struct TMacTxCoef {
  uint8_t m_au8TxCoef[6];
};

struct TNeighbourEntry {
  TShortAddress m_nShortAddress;
  struct TPhyToneMap m_ToneMap;
  uint8_t m_nModulationType : 3;
  uint8_t m_nTxGain : 4;
  uint8_t m_nTxRes : 1;
  struct TMacTxCoef m_TxCoef;
  uint8_t m_nModulationScheme : 1;
  uint8_t m_nPhaseDifferential : 3;
  uint8_t m_u8Lqi;
  uint16_t m_u16TmrValidTime;
  uint16_t m_u16NeighbourValidTime;
};

struct TDeviceTableEntry {
  TPanId m_nPanId;
  struct TMacAddress m_Address;
  uint8_t m_u8Dsn;
  uint32_t m_u32FrameCounter;
};

struct TMacMib {
  uint8_t m_u8HighPriorityWindowSize;
  uint32_t m_u32TxDataPacketCount;
  uint32_t m_u32RxDataPacketCount;
  uint32_t m_u32TxCmdPacketCount;
  uint32_t m_u32RxCmdPacketCount;
  uint32_t m_u32CsmaFailCount;
  uint32_t m_u32CsmaNoAckCount;
  uint32_t m_u32RxDataBroadcastCount;
  uint32_t m_u32TxDataBroadcastCount;
  uint32_t m_u32BadCrcCount;
  struct TNeighbourEntry m_aNeighbourTable[MAC_MAX_NEIGHBOUR_TABLE_ENTRIES];
  struct TDeviceTableEntry m_aDeviceTable[MAC_MAX_DEVICE_TABLE_ENTRIES];
  bool m_bFreqNotching;
  uint8_t m_u8CsmaFairnessLimit;
  uint8_t m_u8TmrTtl;
  uint8_t m_u8NeighbourTableEntryTtl;
  uint16_t m_u16RcCoord;
  struct TPhyToneMask m_ToneMask;
  uint8_t m_u8BeaconRandomizationWindowLength;
  uint8_t m_u8A;
  uint8_t m_u8K;
  uint8_t m_u8MinCwAttempts;
  uint8_t m_u8Bsn;
  uint8_t m_u8Dsn;
  uint8_t m_u8MaxBe;
  uint8_t m_u8MaxCsmaBackoffs;
  uint8_t m_u8MaxFrameRetries;
  uint8_t m_u8MinBe;
  bool m_bCoordinator;
  TPanId m_nPanId;
  struct TExtendedAddress m_ExtendedAddress;
  TShortAddress m_nShortAddress;
  bool m_bPromiscuousMode;
  struct TMacSecurityKey m_aKeyTable[MAC_KEY_TABLE_ENTRIES];
  uint32_t m_u32FrameCounter;
  TShortAddress m_nCoordShortAddress;
  uint8_t m_u8ForcedModScheme;
  uint8_t m_u8ForcedModType;
  struct TPhyToneMap m_ForcedToneMap;
  uint8_t m_u8ForcedModSchemeOnTMResponse;
  uint8_t m_u8ForcedModTypeOnTMResponse;
  struct TPhyToneMap m_ForcedToneMapOnTMResponse;
  enum EPhyModulationScheme m_LastRxModScheme;
  enum EPhyModulationType m_LastRxModType;
  bool m_bLBPFrameReceived;
  bool m_bLNGFrameReceived;
};

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
  // Gets number of valid elements in the Neighbour Table
  MAC_PIB_MANUF_NEIGHBOUR_TABLE_COUNT = 0x08000012,
  // Gets or sets a parameter in Phy layer. Index will be used to contain PHY parameter ID.
  // Check 'enum EPhyParam' below for available Phy parameter IDs
  MAC_PIB_MANUF_PHY_PARAM = 0x08000020
};

enum EPhyParam {
  // Phy layer version number. 32 bits.
  PHY_PARAM_VERSION = 0x010c,
  // Correctly transmitted frame count. 32 bits.
  PHY_PARAM_TX_TOTAL = 0x0110,
  // Transmitted bytes count. 32 bits.
  PHY_PARAM_TX_TOTAL_BYTES = 0x0114,
  // Transmission errors count. 32 bits.
  PHY_PARAM_TX_TOTAL_ERRORS = 0x0118,
  // Transmission failure due to already in transmission. 32 bits.
  PHY_PARAM_BAD_BUSY_TX = 0x011C,
  // Transmission failure due to busy channel. 32 bits.
  PHY_PARAM_TX_BAD_BUSY_CHANNEL = 0x0120,
  // Bad len in message (too short - too long). 32 bits.
  PHY_PARAM_TX_BAD_LEN = 0x0124,
  // Message to transmit in bad format. 32 bits.
  PHY_PARAM_TX_BAD_FORMAT = 0x0128,
  // Timeout error in transmission. 32 bits.
  PHY_PARAM_TX_TIMEOUT = 0x012C,
  // Received correctly messages count. 32 bits.
  PHY_PARAM_RX_TOTAL = 0x0130,
  // Received bytes count. 32 bits.
  PHY_PARAM_RX_TOTAL_BYTES = 0x0134,
  // Reception RS errors count. 32 bits.
  PHY_PARAM_RX_RS_ERRORS = 0x0138,
  // Reception Exceptions count. 32 bits.
  PHY_PARAM_RX_EXCEPTIONS = 0x013C,
  // Bad len in message (too short - too long). 32 bits.
  PHY_PARAM_RX_BAD_LEN = 0x0140,
  // Bad CRC in received FCH. 32 bits.
  PHY_PARAM_RX_BAD_CRC_FCH = 0x0144,
  // CRC correct but invalid protocol. 32 bits.
  PHY_PARAM_RX_FALSE_POSITIVE = 0x0148,
  // Received message in bad format. 32 bits.
  PHY_PARAM_RX_BAD_FORMAT = 0x014C,
  // Time between noise captures (in ms). 32 bits.
  PHY_PARAM_TIME_BETWEEN_NOISE_CAPTURES = 0x0158,
  // Auto detect impedance
  PHY_PARAM_CFG_AUTODETECT_BRANCH = 0x0161,
  // Manual impedance configuration
  PHY_PARAM_CFG_IMPEDANCE = 0x0162,
  // Indicate if notch filter is active or not. 8 bits.
  PHY_PARAM_RRC_NOTCH_ACTIVE = 0x0163,
  // Index of the notch filter. 8 bits.
  PHY_PARAM_RRC_NOTCH_INDEX = 0x0164,
  // Enable periodic noise autodetect and adaptation. 8 bits.
  PHY_PARAM_ENABLE_AUTO_NOISE_CAPTURE = 0x0166,
  // Noise detection timer reloaded after a correct reception. 8 bits.
  PHY_PARAM_DELAY_NOISE_CAPTURE_AFTER_RX = 0x0167,
  // Disable PLC Tx/Rx. 8 bits.
  PHY_PARAM_PLC_DISABLE = 0x016A,
  // Indicate noise power in dBuV for the noisier carrier
  PHY_PARAM_NOISE_PEAK_POWER = 0x016B,
  // LQI value of the last received message
  PHY_PARAM_LAST_MSG_LQI = 0x016C
};

#define MAC_PIB_MAX_VALUE_LENGTH (31)

struct TMacPibValue {
  uint8_t m_u8Length;
  uint8_t m_au8Value[MAC_PIB_MAX_VALUE_LENGTH];
};

void MacMibReset(void);
void MacMibInitialize(void);
enum EMacStatus MacMibGet(enum EMacPibAttribute eAttribute, uint16_t u16Index, struct TMacPibValue *pValue);
enum EMacStatus MacMibSet(enum EMacPibAttribute eAttribute, uint16_t u16Index, const struct TMacPibValue *pValue);

struct TNeighbourEntry * MibGetNeighbourEntry(TShortAddress nShortAddress);
struct TDeviceTableEntry * MibGetDeviceEntry(TPanId nPanId, struct TMacAddress *pAddress, bool *pbCreated);

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
