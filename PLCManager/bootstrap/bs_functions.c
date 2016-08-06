#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <AdpApi.h>
#include <AdpApiTypes.h>

#include "ProtoLbp.h"
#include "ProtoEapPsk.h"

#include "bs_functions.h"
#include "bs_api.h"
#include "conf_bs.h"

#include "time.h"

//extern uint32_t platform_random_32(void);
//extern uint32_t oss_get_up_time_ms(void);
static time_t sul_bs_start;

unsigned char m_Data[400];
unsigned short m_u16Length;
bool m_bRekey;
struct TAdpExtendedAddress m_LbdAddress;

uint16_t us_blacklist_size = 0;
uint8_t puc_blacklist[MAX_LBDS][ADP_ADDRESS_64BITS];

t_context g_current_context;

uint32_t g_u32Nonce = 0;
unsigned char g_u8CurrGMKId = 0;

static struct TEapPskKey g_EapPskKey =
{
    {0xAB, 0x10, 0x34, 0x11, 0x45, 0x11, 0x1B, 0xC3, 0xC1, 0x2D, 0xE8, 0xFF, 0x11, 0x14, 0x22, 0x04}
};

struct TEapPskNetworkAccessIdentifierS g_IdS;

const struct TEapPskNetworkAccessIdentifierS x_ids_arib = { NETWORK_ACCESS_IDENTIFIER_SIZE_S_ARIB,
			{0x53, 0x4D, 0xAD, 0xB2, 0xC4, 0xD5, 0xE6, 0xFA, 0x53, 0x4D, 0xAD, 0xB2, 0xC4, 0xD5, 0xE6, 0xFA,
			 0x53, 0x4D, 0xAD, 0xB2, 0xC4, 0xD5, 0xE6, 0xFA, 0x53, 0x4D, 0xAD, 0xB2, 0xC4, 0xD5, 0xE6, 0xFA,
			 0x53, 0x4D} };

const struct TEapPskNetworkAccessIdentifierS x_ids_cenelec_fcc =
		{ NETWORK_ACCESS_IDENTIFIER_SIZE_S_CENELEC_FCC,	{0x81, 0x72, 0x63, 0x54, 0x45, 0x36, 0x27, 0x18} };

static uint8_t g_au8CurrGMK[16]  = {0xAF, 0x4D, 0x6D, 0xCC, 0xF1, 0x4D, 0xE7, 0xC1, 0xC4, 0x23, 0x5E, 0x6F, 0xEF, 0x6C, 0x15, 0x1F};
static uint8_t g_au8RekeyGMK[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

/************************************************************************************/
/** LBDs table
 ************************************************************************************/
// LBDs table
lbds_list_entry_t g_lbds_list[MAX_LBDS];
// Active LBDs counter
static uint16_t g_lbds_counter = 0;
static uint16_t g_lbds_list_size = 0;


/************************************************************************************/
/** Parameters transferred
 * Note: the parameters are already encoded on 1 byte (M field and last bit included
 ************************************************************************************/
#define CONF_PARAM_SHORT_ADDR 0x1D
#define CONF_PARAM_GMK 0x27
#define CONF_PARAM_GMK_ACTIVATION 0x2B
#define CONF_PARAM_GMK_REMOVAL 0x2F
#define CONF_PARAM_RESULT 0x31

/**
 * \brief Returns if the short address is valid
 *
 * \param us_short_address  Short address
 *
 * \return true / false.
 */
static bool _is_valid_address(uint16_t us_short_address){
	// Check if the short address is out of the range
	if(us_short_address < g_current_context.initialShortAddr)
		return false;
	else if(us_short_address - g_current_context.initialShortAddr > MAX_LBDS)
		return false;
	else
		return true;
}

/**
 * \brief Returns if the short address is valid
 *
 * \param puc_extended_address  Extended address address
 *
 * \return true / false.
 */
bool is_null_address(uint8_t * puc_extended_address){
	uint8_t i;
	// Check if the short address is null
	for(i = 0; i < ADP_ADDRESS_64BITS; i++)
	{
		// If any of the bytes of the address isn't null, return false.
		if(puc_extended_address[i] != 0x00)
			return false;
	}
	// At this point, the address is null
	return true;
}

/**
 * \brief Returns the number of active LBDs
 *
 * \return number of active LBDs
 */
uint16_t get_lbds_count(void)
{
	return g_lbds_counter;
}

/**
 * \brief Returns the LBD short address in position i of the LBDs list, if it is active
 *
 * \param i index in the LBDs list.
 *
 * \return LBD short address
 *
 * NOTE: Returns 0 if the address is not active.
 */
uint16_t get_lbd_address(uint16_t i)
{
	// Check if the position is free
	if(is_null_address(g_lbds_list[i].puc_extended_address)){
		return 0x0000;
	}
	else
	{
		// The short address is calculated using the index and the initial short address
		return (i + g_current_context.initialShortAddr);
	}
}

/**
 * \brief Returns if the device is active in the LBDs list
 *
 * \param us_short_address short address
 *
 * \return 1 (yes) / 0 (no)
 *
 * NOTE: Returns 0 if the address is not active.
 */
uint8_t device_is_in_list(uint16_t us_short_address){
	// Check if the short address is out of the range
	if(!_is_valid_address(us_short_address))
		return (0);

	// Check if the address of the device is active
	if(is_null_address(g_lbds_list[us_short_address - g_current_context.initialShortAddr].puc_extended_address))
		return (0);
	else
		return (1);
}

/**
 * \brief Deactivates the LBD specified by its short address
 *
 * \param us_short_address short address
 */
void remove_lbds_list_entry(uint16_t us_short_address){
	// Check if the short address is out of the range
	if(!_is_valid_address(us_short_address)){
		LOG_BOOTSTRAP(Log("[BS] Error: attempted to deactivate an address out of range [0x%04x]\n", us_short_address));
		return;
	}
	else{
		// Check if the address is active
		if(!is_null_address(g_lbds_list[us_short_address - g_current_context.initialShortAddr].puc_extended_address)){
			// Deactivate address
			memset(&g_lbds_list[us_short_address - g_current_context.initialShortAddr].puc_extended_address, 0, ADP_ADDRESS_64BITS * sizeof(uint8_t));
			g_lbds_counter--;
		}
		else{
			// The address is not active -> The device hasn't joined
			LOG_BOOTSTRAP(Log("[BS] Error: attempted to deactivate an inactive address [0x%04x]\n", us_short_address));
		}
	}

	return;
}

/**
 * \brief Returns a new address that is not in use.
 *  If the end of the LBDs list is not reached, a new address is given instead
 *  of reusing the addresses that became free. This way, the time to reuse an
 *  address is extended.
 *
 * \return short address
 *
 * NOTE: 0x0000 will be returned if the LBDs list is full
 */
uint16_t get_new_address(void){
	uint16_t us_short_address = 0x0000;

	uint16_t i = 0;

	// If the end of the list is not reached, give the next address & increase list size
	if(g_lbds_list_size < MAX_LBDS){
		us_short_address = g_lbds_list_size + g_current_context.initialShortAddr;
		g_lbds_list_size++;
		return (us_short_address);
	}
	else{
		// End of the list reached: Go through the list to find free positions
		while((i < MAX_LBDS)) {
			// Check if the position is free (null extended address)
			if(is_null_address(g_lbds_list[i].puc_extended_address)){
				// Free position: Return the address (calculated with the index and the initial short address)
				us_short_address = i + g_current_context.initialShortAddr;
				return (us_short_address);
			}
			i++;
		}
	}

	// At this point, there is no free positions in the list. Maximum number of devices reached.
	LOG_BOOTSTRAP(Log("[BS] Device node: 0x%04x cannot be registered. Maximum number of nodes reached: %d\n", g_lbds_counter, MAX_LBDS));
	return us_short_address;
}

/**
 * \brief Adds a LBD to the devices' list. A short address is assigned.
 *
 * \param puc_extended_address extended address
 * \param us_short_address short address
 * \param bitmap with the configuration (specifies if the LBD is active)
 *
 * \return true / false.
 */
bool add_lbds_list_entry(const uint8_t *puc_extended_address, uint16_t us_short_address)
{
	// Check if the short address is out of the range
	if(!_is_valid_address(us_short_address)){
		LOG_BOOTSTRAP(Log("[BS] Error: attempted to activate an address out of range [0x%04x]\n", us_short_address));
		return false;
	}

	uint16_t us_position = us_short_address - g_current_context.initialShortAddr;
	uint16_t us_dummy_short_address;

	// Check if the short address is already in use
	if(!is_null_address(g_lbds_list[us_position].puc_extended_address)){
		LOG_BOOTSTRAP(Log("[BS] Short address already in use [0x%04x], not added to list", us_short_address));
		return false;
	}

	// Check if the extended address is already in use
	if(get_short_addr_by_ext((uint8_t *)puc_extended_address, &us_dummy_short_address)){
		LOG_BOOTSTRAP(Log("[BS] Extended address already in use [%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X]",
				*puc_extended_address, *(puc_extended_address + 1), *(puc_extended_address + 2),
				*(puc_extended_address + 3), *(puc_extended_address + 4), *(puc_extended_address + 5),
				*(puc_extended_address + 6), *(puc_extended_address + 7)));
		return false;
	}

	memcpy(g_lbds_list[us_position].puc_extended_address, puc_extended_address, 8);
	LOG_BOOTSTRAP(Log("[BS] Added address [0x%04x]", us_short_address));
	g_lbds_counter++;
	LOG_BOOTSTRAP(Log("[BS] Total num. devices: %d.\n", g_lbds_counter));

	return true;
}

/**
 * \brief Function to handle joined devices list as a hash, indexed
 *        by short address
 *
 * \param us_short_address
 *
 * \return true if index found, false otherwise.
 */
bool get_ext_addr_by_short (uint16_t us_short_address, uint8_t *puc_extended_address)
{
	uint16_t us_index;
	bool found = false;

	us_index = us_short_address - g_current_context.initialShortAddr;

	if (!is_null_address(g_lbds_list[us_index].puc_extended_address)) {
		memcpy(puc_extended_address, g_lbds_list[us_index].puc_extended_address, ADP_ADDRESS_64BITS);
		found = true;
	} else {
		LOG_BOOTSTRAP(Log("[BS] Error: tried to access joined devices list by not-joined short address."));
	}

	return found;
}

/**
 * \brief Function to handle joined devices list as a hash, indexed
 *        by extended address
 *
 * \param
 *
 * \return true if index found, false otherwise.
 */
bool get_short_addr_by_ext (uint8_t *puc_extended_address, uint16_t *pus_short_address)
{
	uint8_t i = 0;
	uint8_t j = 0;
	bool found = false;

	while (j < g_lbds_list_size) {
		if (!is_null_address(g_lbds_list[j].puc_extended_address)) {
			// Check if the short address matches
			found = true;
			for (i = 0; i < ADP_ADDRESS_64BITS; i++)
			{
				// If any of the bytes of the address isn't null, go on.
				if(puc_extended_address[i] != g_lbds_list[j].puc_extended_address[i]) {
					found = false;
					break;
				}
			}
		}

		if (found) {
			*pus_short_address = j + g_current_context.initialShortAddr;
			break;
		}

		j++;
	}

	if (found) {
		LOG_BOOTSTRAP(Log("[BS] Extended address found in joined devices list with short address 0x%04X.", *pus_short_address));
	} else {
		LOG_BOOTSTRAP(Log("[BS] Error: extended address not found in joined devices list."));
	}

	return found;
}

///**
// * AdpRoutingTable.
// *
// */
//static void AdpRoutingTable(uint16_t data_len, uint8_t *data)
//{
//	AdpSetRequest(0x0000000C, 0, data_len, data);
//}

/**
 * \brief GetData.
 *
 */
unsigned char *GetData(void)
{
	return m_Data;
}

/**
 * \brief GetLength.
 *
 */
unsigned short GetLength(void)
{
	return m_u16Length;
}

/**
 * \brief LBP get rekeying
 *
 */
uint16_t lbp_get_rekeying(void)
{
	return m_bRekey;
}

/**
 * \brief LBP set rekeying
 *
 */
void lbp_set_rekeying(uint8_t on_off)
{
	m_bRekey = on_off;
}

uint16_t get_current_short_address(void)
{
	return g_current_context.currentShortAddr;
}

/**
 * \brief Set current short address
 *
 */
void set_current_short_address(uint16_t us_short_addr)
{
	g_current_context.currentShortAddr = us_short_addr;
}

/**
 * \brief Get initial short address
 *
 */
uint16_t get_initial_short_address(void)
{
	return g_current_context.initialShortAddr;
}

/**
 * \brief Set initial short address
 *
 */
bool set_initial_short_address(uint16_t us_short_addr)
{
	if(us_short_addr == 0)
		return false;
	else if(us_short_addr > (0xFFFF - MAX_LBDS))
		return false;
	else
	{
		g_current_context.initialShortAddr = us_short_addr;
		return true;
	}
}

/**
 * \brief GetLbdAddress.
 *
 */
struct TAdpExtendedAddress *get_current_ext_address(void)
{
	return &m_LbdAddress;
}

/**
 * \brief Get GMK.
 *
 */
const uint8_t *GetGMK(void)
{
	return g_au8CurrGMK;
}

/**
 * Set GMK.
 *
 */
void set_gmk(uint8_t *puc_new_gmk)
{
	if (puc_new_gmk != NULL) {
		memcpy(g_au8CurrGMK, puc_new_gmk, 16);
	}
}

/**
 * Set rekeying GMK.
 *
 */
void set_rekey_gmk(uint8_t *puc_new_gmk)
{
	if (puc_new_gmk != NULL) {
		memcpy(g_au8RekeyGMK, puc_new_gmk, 16);
	}
}

/**
 * Set IDS.
 *
 */
void set_ids(const uint8_t *puc_ids, uint8_t uc_size)
{
	memcpy(g_IdS.m_au8Value, puc_ids, uc_size);
	return;
}

/**
 * Set PSK.
 *
 */
void set_psk(uint8_t *puc_new_psk)
{
	if (puc_new_psk != NULL) {
		memcpy(g_EapPskKey.m_au8Value, puc_new_psk, 16);
	}
}

/**
 * \brief LBP init functions
 *
 */
void lbp_init_functions(void)
{
	uint8_t uc_band_id;
        struct TAdpMacGetConfirm x_pib_confirm;

	g_current_context.currentShortAddr = 0;
	// The first short address is initially set to 1
	g_current_context.initialShortAddr = 1;

        m_bRekey = LBP_REKEYING_OFF;

	/* Initialize LBP blacklist */
	us_blacklist_size = 0;
	memset(puc_blacklist, 0, MAX_LBDS * ADP_ADDRESS_64BITS);

	/* Initialize LBP list */
	g_lbds_counter = 0;
	g_lbds_list_size = 0;
	memset(g_lbds_list, 0, MAX_LBDS * sizeof(lbds_list_entry_t));

	/* Read the band from MIB */
	// CHRIS REVIEW : AdpMacGetRequestSync(MAC_PIB_MANUF_BAND_INFORMATION, 0, &x_pib_confirm);

	/* Properly configure IDS according to band */
	uc_band_id =x_pib_confirm.m_au8AttributeValue[0];

	if (uc_band_id == BAND_ARIB) {
		g_IdS.uc_size = NETWORK_ACCESS_IDENTIFIER_MAX_SIZE_S;
		memcpy(g_IdS.m_au8Value, x_ids_arib.m_au8Value, NETWORK_ACCESS_IDENTIFIER_MAX_SIZE_S);
	} else {
		g_IdS.uc_size = 8;
		memcpy(g_IdS.m_au8Value, x_ids_cenelec_fcc.m_au8Value, 8);
	}
}

/**
 * \brief Initialize bootstrap message
 *
 */
void initialize_bootstrap_message(void)
{
	m_u16Length = 0;
	memset(m_Data, 0, sizeof(m_Data));
}

/**
 * \brief Process_Joining0.
 *
 */
void Process_Joining0(struct TAdpExtendedAddress pLBPEUI64Address)
{
    unsigned char *pMemoryBuffer = m_Data;
    unsigned short u16MemoryBufferLength = sizeof(m_Data);
    struct TEapPskRand randS;
    uint16_t u16DummyShortAddress;

	LOG_BOOTSTRAP(Log("[BS] Process Joining 0."));

	EAP_PSK_Initialize(&g_EapPskKey, &g_current_context.g_PskContext);

    // initialize RandS
    uint8_t i;
    // init rand seed
	srand(time(NULL));
	for(i = 0; i < sizeof(randS.m_au8Value); i++)
	{
		//randS.m_au8Value[i] = platform_random_32() & 0xFF;
		randS.m_au8Value[i] = (uint8_t)rand();
	}

    m_u16Length = EAP_PSK_Encode_Message1(
                           g_current_context.u8EAPIdentifier,
                           &randS,
                           &g_IdS,
                           u16MemoryBufferLength,
                           pMemoryBuffer
                       );

    g_current_context.u8EAPIdentifier++;

    m_u16Length = LBP_Encode_ChallengeRequest(
                           &pLBPEUI64Address,
						   m_u16Length,
                           u16MemoryBufferLength,
                           pMemoryBuffer
                       );

    if (!m_bRekey) {
    	// If extended address is already in list, remove it and give a new short address
    	if (get_short_addr_by_ext (pLBPEUI64Address.m_au8Value, &u16DummyShortAddress)) {
    		remove_lbds_list_entry(u16DummyShortAddress);
    	}

    	// Get a new address for the device. Its extended address will be added to the list when the joining process finishes.
    	g_current_context.currentShortAddr = get_new_address();
    }
}

/**
 * \brief Process accepted GMK activation
 *
 */
uint8_t process_accepted_GMK_activation(struct TAdpExtendedAddress au8LBPEUI64Address)
{
    unsigned char *pMemoryBuffer = m_Data;
    unsigned short u16MemoryBufferLength = sizeof(m_Data);
    uint8_t pdata[3];
    uint8_t uc_result = 1;

	LOG_BOOTSTRAP(Log("[BS] Accepted(GMK-activation)."));

	// prepare the protected data carring the key and short addr
	pdata[0] = CONF_PARAM_GMK_ACTIVATION;
	pdata[1] = 0x01;
	pdata[2] = 0x01; // key id

	m_u16Length = EAP_PSK_Encode_GMK_Activation(
					pdata, /* PCHANNEL data */
					u16MemoryBufferLength,
					pMemoryBuffer);

	// Encode now the LBP message
	m_u16Length = LBP_Encode_AcceptedRequest(
					 &au8LBPEUI64Address,
					 m_u16Length,
					 u16MemoryBufferLength,
					 pMemoryBuffer);

  return(uc_result);
}

/**
 * \brief Process_Joining_EAP_T1.
 *
 */
static uint8_t Process_Joining_EAP_T1(struct TAdpExtendedAddress au8LBPEUI64Address, unsigned short u16EAPDataLength, unsigned char *pEAPData)
{
    struct TEapPskRand randS;
    struct TEapPskRand randP;
    unsigned char *pMemoryBuffer = m_Data;
    unsigned short u16MemoryBufferLength = sizeof(m_Data);
    unsigned char pdata[50];
    unsigned short u16PDataLen = 0;
    unsigned short u16ShortAddr;
    uint8_t uc_result = 1;

	LOG_BOOTSTRAP(Log("[BS] Process Joining EAP T1."));

    if (EAP_PSK_Decode_Message2(u16EAPDataLength, pEAPData, &g_current_context.g_PskContext, &g_IdS, &randS, &randP))
    {
        EAP_PSK_InitializeTEK(&randP, &g_current_context.g_PskContext);

        // encode and send the message T2
				u16ShortAddr = g_current_context.currentShortAddr;

        // prepare the protected data carring the key and short addr
        pdata[u16PDataLen ++] = 0x02; // ext field

        if (!m_bRekey)
        {
            pdata[u16PDataLen ++] = CONF_PARAM_SHORT_ADDR;
            pdata[u16PDataLen ++] = 2;
            pdata[u16PDataLen ++] = (unsigned char) ((u16ShortAddr & 0xFF00) >> 8);
            pdata[u16PDataLen ++] = (unsigned char) (u16ShortAddr & 0x00FF);

            pdata[u16PDataLen ++] = CONF_PARAM_GMK;
            pdata[u16PDataLen ++] = 17;
            pdata[u16PDataLen ++] = 0; // key id
            memcpy(&pdata[u16PDataLen], g_au8CurrGMK, 16); // key
            u16PDataLen += 16;

            pdata[u16PDataLen ++] = CONF_PARAM_GMK_ACTIVATION;
            pdata[u16PDataLen ++] = 1;
            pdata[u16PDataLen ++] = 0; // key id
        }
        else
        {
            pdata[u16PDataLen ++] = CONF_PARAM_GMK;
            pdata[u16PDataLen ++] = 17;
            pdata[u16PDataLen ++] = 1; // key id
            memcpy(&pdata[u16PDataLen], g_au8RekeyGMK, 16); // key
            u16PDataLen += 16;
        }

        m_u16Length = EAP_PSK_Encode_Message3(
                               &g_current_context.g_PskContext,
                               g_current_context.u8EAPIdentifier,
                               &randS,
                               &randP,
                               &g_IdS,
                               g_u32Nonce,
                               PCHANNEL_RESULT_DONE_SUCCESS,
                               u16PDataLen,
                               pdata,
                               u16MemoryBufferLength,
                               pMemoryBuffer
                           );

        g_current_context.u8EAPIdentifier++;
        g_u32Nonce ++;

        // Encode now the LBP message
        m_u16Length = LBP_Encode_ChallengeRequest(
                               &au8LBPEUI64Address,
							   m_u16Length,
                               u16MemoryBufferLength,
                               pMemoryBuffer
                           );
    }
	else
	{
		LOG_BOOTSTRAP(Log("[BS] ERROR: Process_Joining_EAP_T1."));
		uc_result = 0;
	}

    return(uc_result);
}

/**
 * \brief Process_Joining_EAP_T3.
 *
 */
static void Process_Joining_EAP_T3(struct TAdpExtendedAddress  au8LBPEUI64Address, unsigned char *pBootStrappingData, unsigned short u16EAPDataLength,
							unsigned char *pEAPData)
{
    struct TEapPskRand randS;
    unsigned char u8PChannelResult = 0;
    uint32_t u32Nonce = 0;
    unsigned short u16PChannelDataLength = 0;
    unsigned char *pPChannelData = 0L;
    unsigned char *pMemoryBuffer = m_Data;
    unsigned short u16MemoryBufferLength = sizeof(m_Data);

		LOG_BOOTSTRAP(Log("[BS] Process Joining EAP T3."));

    if (EAP_PSK_Decode_Message4(u16EAPDataLength, pEAPData, &g_current_context.g_PskContext,
    							22, pBootStrappingData, &randS, &u32Nonce, &u8PChannelResult,
								&u16PChannelDataLength, &pPChannelData)) {
    	// encode and send the message T2
    	m_u16Length = EAP_PSK_Encode_EAP_Success(
                               g_current_context.u8EAPIdentifier,
                               u16MemoryBufferLength,
                               pMemoryBuffer
                           );

    	g_current_context.u8EAPIdentifier++;

      // Encode now the LBP message
    	m_u16Length = LBP_Encode_AcceptedRequest(
                               &au8LBPEUI64Address,
							   							 m_u16Length,
                               u16MemoryBufferLength,
                               pMemoryBuffer
                           );

		// Write device into routing table. Directly connected.
		//printf("Add device to Routing Table\n");
//		uint16_t g_lbs_u16ShortAddr = 1;
//		route_buf[0] = g_lbs_u16ShortAddr & 0x00ff;
//		route_buf[1] = g_lbs_u16ShortAddr>>8 & 0x00ff;
//		route_buf[2] = g_lbs_u16ShortAddr & 0x00ff;
//		route_buf[3] = g_lbs_u16ShortAddr>>8 & 0x00ff;
//		AdpRoutingTable(9, route_buf);

        _lbp_device_accepted(&au8LBPEUI64Address);

    } else {
    	LOG_BOOTSTRAP(Log("[BS] ERROR: Process_Joining_EAP_T3."));
    }
}

/**
 * \brief Add to blacklist
 *
 */
uint8_t add_to_blacklist(uint8_t *puc_address)
{
	// TO DO: advanced list management (defragment list, etc...)

	uint8_t uc_status = 1;

	if (us_blacklist_size < MAX_LBDS) {
		memcpy(puc_blacklist[us_blacklist_size], puc_address, ADP_ADDRESS_64BITS);
		us_blacklist_size++;
	} else {
		/* Blacklist full - error */
		uc_status = 0;
	}

	return uc_status;
}

/**
 * \brief Remove from blacklist
 *
 */
uint8_t remove_from_blacklist(uint16_t us_index)
{
	uint8_t uc_status = 0;

	// TO DO: advanced list management (defragment list, etc...)
	if (us_index < MAX_LBDS) {
		memset(puc_blacklist[us_index], 0, ADP_ADDRESS_64BITS);
		uc_status = 1;
	}

	return uc_status;
}

/**
 * \brief Device is in blacklist
 *
 */
static uint8_t _dev_is_in_blacklist(uint8_t *puc_address)
{
	uint16_t i = 0;
	uint8_t uc_found = 0;

	while(i < us_blacklist_size) {
		if(!memcmp(puc_address, puc_blacklist[i], ADP_ADDRESS_64BITS)) {
			uc_found = 1;
			break;
		}
		i++;
	}

	return uc_found;
}

/**
 * \brief ProcessLBPMessage.
 *
 */
enum lbp_indications ProcessLBPMessage(unsigned char *pNsdu, unsigned short u16NsduLen)
{
	static uint32_t ul_bs_start = 0;
	static uint8_t uc_bs_in_progress = 0;
	static struct TAdpExtendedAddress ext_address_in_process;
    unsigned char u8MessageType = ((pNsdu[0] & 0xF0) >> 4);
    unsigned char *pBootStrappingData = &pNsdu[10];
    unsigned short u16BootStrappingDataLength = u16NsduLen - 10;

    // Embedded EAP message
    unsigned char u8Code = 0;
    unsigned char u8Identifier = 0;
    unsigned char u8TSubfield = 0;
    unsigned short u16EAPDataLength = 0;
    unsigned char *pEAPData = 0L;

    enum lbp_indications lbp_indication = LBS_NONE;

	memcpy(m_LbdAddress.m_au8Value, &pNsdu[2], 8);

    if (u8MessageType == LBP_JOINING) {

    	LOG_BOOTSTRAP(Log("[BS] Processing incoming LBP_JOINING... LBD Address: 0x%02x%02x%02x%02x%02x%02x%02x%02x.",
						m_LbdAddress.m_au8Value[0], m_LbdAddress.m_au8Value[1], m_LbdAddress.m_au8Value[2], m_LbdAddress.m_au8Value[3],
						m_LbdAddress.m_au8Value[4], m_LbdAddress.m_au8Value[5], m_LbdAddress.m_au8Value[6], m_LbdAddress.m_au8Value[7]));

    	/* Check the bootstrapping data in order to see the progress of the joining process */
        if (u16BootStrappingDataLength == 0) {
        	LOG_BOOTSTRAP(Log("[BS] First joining request."));
        	/* This is the first joining request. Responded only if no other BS was in progress. */
        	if(!uc_bs_in_progress) {
        		/* Check if the joining device is blacklisted */
        		if (_dev_is_in_blacklist(m_LbdAddress.m_au8Value)) {
        			m_u16Length = Encode_decline(m_LbdAddress.m_au8Value);
        		} else {
					//ul_bs_start = oss_get_up_time_ms();
					sul_bs_start = time(NULL);
					uc_bs_in_progress = 1;
					memcpy(ext_address_in_process.m_au8Value, m_LbdAddress.m_au8Value, sizeof(m_LbdAddress.m_au8Value));
					Process_Joining0(m_LbdAddress);
        		}

        	} else {
        		time_t ul_curr_time;

        		ul_curr_time = time(NULL);

        		if (!memcmp(m_LbdAddress.m_au8Value, ext_address_in_process.m_au8Value, sizeof(m_LbdAddress.m_au8Value))) {
        			/* The device has re-started the joining process. Serve it. */
        			Process_Joining0(m_LbdAddress);
        		//} else if (oss_get_up_time_ms() - ul_bs_start > BS_MAX_JOIN_TIME) {
        		} else if (ul_curr_time - sul_bs_start > BS_MAX_JOIN_TIME) {
        			/* Consider the ongoing joining timed out. Proceed with the incoming one. */
            		//ul_bs_start = oss_get_up_time_ms();
					sul_bs_start = time(NULL);
            		LOG_BOOTSTRAP(Log("[BS] Ongoing LBP process aborted. Time out."));
                	memcpy(ext_address_in_process.m_au8Value, m_LbdAddress.m_au8Value, sizeof(m_LbdAddress.m_au8Value));
        			Process_Joining0(m_LbdAddress);
        		} else {
            		LOG_BOOTSTRAP(Log("[BS] Concurrent joining request received. Ignored."));
        		}
        	}
        } else {
        	/* Check if the message comes from the device currently under BS */
        	if((!memcmp(m_LbdAddress.m_au8Value, ext_address_in_process.m_au8Value, sizeof(m_LbdAddress.m_au8Value)))||(m_bRekey)) {
				// check the type of the bootstrap data
				if ((pBootStrappingData[0] & 0x01) != 0x01) {

					LOG_BOOTSTRAP(Log("[BS] Successive joining request."));
					if (EAP_PSK_Decode_Message(
								u16BootStrappingDataLength,
								pBootStrappingData,
								&u8Code,
								&u8Identifier,
								&u8TSubfield,
								&u16EAPDataLength,
								&pEAPData)) {
						if (u8Code == EAP_RESPONSE) {
							if (u8TSubfield == EAP_PSK_T1) {
								if(Process_Joining_EAP_T1(m_LbdAddress, u16EAPDataLength, pEAPData) != 1) {
									/* Abort current BS process */
									LOG_BOOTSTRAP(Log("[BS] LBP protocol error."));
									uc_bs_in_progress = 0;
								}
							} else if (u8TSubfield == EAP_PSK_T3) {
								Process_Joining_EAP_T3(m_LbdAddress, pBootStrappingData, u16EAPDataLength, pEAPData);
								/* At this point the ongoing BS process must be ended */
								/* independently of its result (success or failure)   */
								uc_bs_in_progress = 0;
							} else {
								/* Abort current BS process */
								LOG_BOOTSTRAP(Log("[BS] LBP protocol error."));
								uc_bs_in_progress = 0;
							}
						}
					} else {
						/* Abort current BS process */
						LOG_BOOTSTRAP(Log("[BS] ERROR decoding message."));
						uc_bs_in_progress = 0;
					}
				}
        	} else {
        		LOG_BOOTSTRAP(Log("[BS] Concurrent successive joining received. Ignored."));
        	}
        }
    } else if (u8MessageType == LBP_KICK_FROM_LBD) {
    	/* Call upper layer LEAVE callback */
    	lbp_indication = LBS_KICK;
    } else {
    	LOG_BOOTSTRAP(Log("[BS] ERROR: unknown incoming message."));
    }

    return(lbp_indication);
}

/**
 * \brief Encode kick to LBD.
 *
 */
uint16_t Encode_kick_to_LBD(uint8_t *p_ext_addr, uint16_t u16MessageLength, uint8_t *pMessageBuffer)
{
  uint16_t u16EncodedLength = 0;

  // Check first if the message buffer size if enough
  if (u16MessageLength >= ADP_ADDRESS_64BITS + 2) {
    // start message encoding
    pMessageBuffer[0] = (LBP_KICK_TO_LBD << 4);
    pMessageBuffer[1] = 0; // transaction id is reserved

    memcpy(&pMessageBuffer[2], p_ext_addr, ADP_ADDRESS_64BITS);

    u16EncodedLength = ADP_ADDRESS_64BITS + 2;
  }

  return u16EncodedLength;
}

/**
 * \brief Encode decline
 *
 */
uint16_t Encode_decline(uint8_t *p_ext_addr)
{
  uint8_t *pMessageBuffer = m_Data;
  uint16_t u16MessageLength = sizeof(m_Data);

  uint16_t u16EncodedLength = 0;

  if (u16MessageLength >= ADP_ADDRESS_64BITS + 2) {
    pMessageBuffer[0] = (LBP_DECLINE  << 4);
    pMessageBuffer[1] = 0; // transaction id is reserved

    memcpy(&pMessageBuffer[2], p_ext_addr, ADP_ADDRESS_64BITS);

    u16EncodedLength = ADP_ADDRESS_64BITS + 2;
  }

  return u16EncodedLength;
}
