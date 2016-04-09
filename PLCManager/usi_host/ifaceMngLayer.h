#ifndef mngIfaceh
#define mngIfaceh

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*mngp_rsp_cb_t)(uint8_t* ptrMsg, uint16_t len);

/// Protocol ID to serialize (USI)
#define MNGP_PRIME					0x00
#define MNGP_PRIME_GETQRY			0x00
#define MNGP_PRIME_GETRSP			0x01
#define MNGP_PRIME_SET				0x02
#define MNGP_PRIME_RESET			0x03
#define MNGP_PRIME_REBOOT			0x04
#define MNGP_PRIME_FU				0x05
#define MNGP_PRIME_EN_PIBQRY		0x06
#define MNGP_PRIME_EN_PIBRSP		0x07

//Item Enhanced header id
#define MNGP_PRIME_LISTQRY			0x0E
#define MNGP_PRIME_LISTRSP			0x0F

/* Define this depending on Base USI node buffers*/
#define  MNGP_PRIME_ENHANCED_LIST_MAX_RECORDS 30

void mngLay_Init(void);
void mngLay_NewMsg(uint8_t cmd);
uint8_t mngLay_AddGetPibQuery(uint16_t pib, uint8_t index);
uint8_t mngLay_AddSetPib(uint16_t pib, uint16_t length, uint8_t* msg);
uint8_t mngLay_AddGetPibListEnQuery(uint16_t pib, uint8_t maxRecords, uint8_t* iterator);
uint8_t mngLay_AddResetStats(uint16_t pib, uint8_t index);
uint8_t mngLay_AddFUMsg(uint16_t length, uint8_t* msg);
uint8_t mngLay_BridgeMsg(uint16_t length, uint8_t* msg);
uint8_t mngLay_SendMsg(uint8_t app_id);
uint8_t mngLay_receivedCmd(uint8_t* ptrMsg, uint16_t len);
void mngLay_SetRspCallback(uint8_t app_id, void (*sap_handler)(uint8_t* ptrMsg, uint16_t len));


#ifdef __cplusplus
}
#endif

#endif
