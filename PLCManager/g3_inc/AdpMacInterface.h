/**********************************************************************************************************************/
/** \addtogroup AdpMac
 * @{
 **********************************************************************************************************************/

/**********************************************************************************************************************/
/** This file contains the declarations of the MAC Proxy module primitives.
 *
 * The AdpMac is an adaptation layer between the ADP and MAC
 ***********************************************************************************************************************
 *
 * @file
 *
 **********************************************************************************************************************/

#ifndef __ADP_MAC_INTERFACE_H__
#define __ADP_MAC_INTERFACE_H__

#include <AdpApiTypes.h>

#define MAC_FRAME_UNSECURED 0x00
#define MAC_FRAME_SECURED_ENC_MIC_32  0x05

#define MAC_FRAME_NOT_ACKED 0x00
#define MAC_FRAME_ACKED     0x01

#define MAC_BROADCAST_ADDR   0xFFFF

struct TGroupMasterKey;
struct TMacPibValue;

enum EAdpMac_Modulation {
  MOD_ROBO, MOD_BPSK, MOD_DBPSK, MOD_QPSK, MOD_DQPSK, MOD_8PSK, MOD_D8PSK, MOD_16QAM, MOD_UNKNOWN = 255
};

struct TAdpMac_NeighbourDescriptor {
  uint16_t m_u16ShortAddress;
  enum EAdpMac_Modulation m_eModulation;
  uint8_t m_u8ActiveTones;
  uint8_t m_u8SubCarriers;
  uint8_t m_u8Lqi;
};

struct TAdpMac_DataRequest {
  uint8_t m_u8SrcAddrSize;
  struct TAdpAddress m_DstDeviceAddress;
  uint16_t m_u16MsduLength;
  uint8_t m_Msdu[400];
  uint8_t m_u8TxOptions;
  uint8_t m_u8QualityOfService;
  uint8_t m_u8SecurityLevel;
  uint8_t m_u8KeyIndex;
};

struct TAdpMac_DataIndication {
  struct TAdpAddress m_SrcDeviceAddress;
  struct TAdpAddress m_DstDeviceAddress;
  uint16_t m_u16MsduLength;
  uint8_t m_Msdu[400];
  uint8_t m_u8MpduLinkQuality;
  uint8_t m_u8DSN;
  uint32_t m_u32Timestamp;
  uint8_t m_u8QualityOfService;
  uint8_t m_u8SecurityLevel;
  uint8_t m_u8KeyIndex;
  enum EAdpMac_Modulation m_eRecvModulation; // modulation used for this message
  uint8_t m_u8RecvActiveTones; // number of active tones used for this message
  uint8_t m_u8RecvSubCarriers; // number of carriers used for this message
  enum EAdpMac_Modulation m_eComputedModulation; // modulation for this message (as computed during the reception)
  uint8_t m_u8ComputedActiveTones; // number of active tones for this message (as computed during the reception)
  uint8_t m_u8ComputedSubCarriers; // number of carriers for this message (as computed during the reception)
};

struct TAdpMac_ResetConfirm {
  uint8_t m_u8Status;
};

struct TAdpMac_ScanConfirm {
  uint8_t m_u8Status;
};

struct TAdpMac_SetConfirm {
  uint8_t m_u8Status;
  uint32_t m_u32AttributeId;
  uint16_t m_u16AttributeIndex;
};

struct TAdpMac_GetConfirm {
  uint8_t m_u8Status;
  uint32_t m_u32AttributeId;
  uint16_t m_u16AttributeIndex;
  uint8_t m_u8AttributeLength;
  uint8_t m_au8AttributeValue[40]; // should be able to store the biggest MAC attr
};

struct TAdpMac_StartConfirm {
  uint8_t m_u8Status;
};

struct TAdpMac_CommStatusIndication {
  uint16_t m_u16PanId;
  struct TAdpAddress m_SrcDeviceAddress;
  struct TAdpAddress m_DstDeviceAddress;
  uint8_t m_u8Status;
  uint8_t m_u8SecurityLevel;
  uint8_t m_u8KeyIndex;
};

/**********************************************************************************************************************/
/** Initializes the MAC layer
 ***********************************************************************************************************************
 * @param u8Band: the PLC band to be used: CENELEC_A, CENELEC_B, FCC, ARIB
 **********************************************************************************************************************/
void AdpMac_Initialize(enum TAdpBand band);

/**********************************************************************************************************************/
/** MAC event handler
 **********************************************************************************************************************/
void AdpMac_EventHandler(void);

/**********************************************************************************************************************/
/** The MCPS-DATA.confirm primitive reports the results of a MCPS-DATA.request
 ***********************************************************************************************************************
 * @param u8MsduHandle The handle associated with the MSDU being confirmed.
 * @param u8Status The status of the last MSDU transmission.
 * @param u32Timestamp Optional. The time, in symbols, at which the data were transmitted
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_DataConfirm)(uint8_t u8Status, void *pUserData);

/**********************************************************************************************************************/
/** The MCPS-DATA.request
 ***********************************************************************************************************************
 * @param pDataRequest
 * @param callback_DataConfirm
 **********************************************************************************************************************/
void AdpMac_DataRequest(struct TAdpMac_DataRequest *pDataRequest, void *pUserData, AdpMac_Callback_DataConfirm callback_DataConfirm);

/**********************************************************************************************************************/
/** The MCPS-DATA.indication
 ***********************************************************************************************************************
 * @param pDataIndication
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_DataIndication)(struct TAdpMac_DataIndication *pDataIndication);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_GetNeighbourDescriptorSync(uint16_t u16ShortAddr, struct TAdpMac_NeighbourDescriptor *pDescriptor);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_GetExtendedAddressSync(struct TAdpExtendedAddress *pExtendedAddress);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_GetShortAddressSync(uint16_t *pu16ShortAddress);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
uint8_t AdpMac_GetMibSync(uint32_t u32AttributeId, uint16_t u16AttributeIndex, struct TMacPibValue *pValue);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_SetConfirm)(struct TAdpMac_SetConfirm *pSetConfirm);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_SetPanIdSync(uint16_t u16PanId);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_SetGroupMasterKeySync(const struct TGroupMasterKey *pMasterKey);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_SetShortAddressSync(uint16_t u16ShortAddress);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_SetRcCoordSync(uint16_t u16RcCoord);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_SetExtendedAddressSync(const struct TAdpExtendedAddress *pExtendedAddress);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
bool AdpMac_SecurityResetSync(void);

/**********************************************************************************************************************/
/**
 **********************************************************************************************************************/
uint8_t AdpMac_SetMibSync(uint32_t u32AttributeId, uint16_t u16AttributeIndex, uint8_t u8AttributeLength,
  const uint8_t *pAttributeValue);

/**********************************************************************************************************************/
/** The MLME-RESET.confirm primitive reports the results of the reset operation.
 ***********************************************************************************************************************
 * @param u8Status The result of the reset operation.
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_ResetConfirm)(struct TAdpMac_ResetConfirm *pResetConfirm);

/**********************************************************************************************************************/
/** The MLME-RESET.request primitive allows the next higher layer to request that the MLME performs a reset operation.
 *
 ***********************************************************************************************************************
 * @param bSetDefaultPIB If TRUE, the MAC sublayer is reset, and all MAC PIB attributes are set to their default values.
 * 						If FALSE, the MAC sublayer is reset, but all MAC PIB attributes retain their values prior to the
 * 						generation of the MLME-RESET.request primitive.
 **********************************************************************************************************************/
void AdpMac_ResetRequest(bool bSetDefaultPIB, bool bKeepParams, AdpMac_Callback_ResetConfirm callback_ResetConfirm);

/**********************************************************************************************************************/
/** The MLME-SCAN.confirm primitive reports the result of the channel scan request.
 ***********************************************************************************************************************
 * @param pScanConfirm
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_ScanConfirm)(struct TAdpMac_ScanConfirm *pScanConfirm);

/**********************************************************************************************************************/
/** The MLME-BEACON-NOTIFY.indication primitive is used to send parameters contained within a beacon frame received
 * by the MAC sublayer tothe next higher layer. The primitive also sends a measure of the LQI and the time the beacon
 * frame was received.
 ***********************************************************************************************************************
 * @param pPanDescriptor
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_BeaconNotifyIndication)(struct TAdpPanDescriptor *pPanDescriptor);

/**********************************************************************************************************************/
/** The MLME-SCAN.request primitive is used to initiate a channel scan over a given list of channels. A device can use
 ***********************************************************************************************************************
 * @param u8ScanDuration A value used to calculate the length of time to spend scanning each channel
 **********************************************************************************************************************/
void AdpMac_ScanRequest(uint8_t u8ScanDuration, AdpMac_Callback_ScanConfirm callback_ScanConfirm,
  AdpMac_Callback_BeaconNotifyIndication callback_BeaconNotifyIndication);

/**********************************************************************************************************************/
/** The MLME-START.confirm primitive reports the results of the attempt to start using a new superframe configuration.
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_StartConfirm)(struct TAdpMac_StartConfirm *pStartConfirm);

/**********************************************************************************************************************/
/** The MLME-START.request primitive allows the PAN coordinator to initiate a new PAN
 ***********************************************************************************************************************
 * @param u16PANId The PAN identifier to be used by the device.
 **********************************************************************************************************************/
void AdpMac_StartRequest(uint16_t u16PanId, AdpMac_Callback_StartConfirm callback_StartConfirm);

/**********************************************************************************************************************/
/** The MLME-COMM-STATUS.indication primitive allows the MLME to indicate a communications status.
 ***********************************************************************************************************************
 * @param pCommStatusIndication
 **********************************************************************************************************************/
typedef void (*AdpMac_Callback_CommStatusIndication)(struct TAdpMac_CommStatusIndication *pCommStatusIndication);

#endif

/**********************************************************************************************************************/
/** @}
 **********************************************************************************************************************/
