#include <string.h>
#include "hal_utils.h"
#include "ifaceMngLayer.h"

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

#define HEAD_LENGTH	2

#define MAX_LENGTH_TX_BUFFER	512

#define LENGTH_PIB					2
#define LENGTH_INDEX				1
#define LENGTH_GET_PIB_QUERY		(LENGTH_PIB + LENGTH_INDEX)

#define TYPE_HEADER(type)			(type & 0x003F)
#define EN_PIBQRY_SHORT_ITERATOR	0
#define EN_PIBQRY_LONG_ITERATOR		1

#define ITERATOR_TYPE_SHIFT			7
#define ITERATOR_TYPE_MASK			0x01
#define ITERATOR_LENGTH_MASK		0x7F

#define LITERATOR_EN_LIST_LEN(iterLen) (iterLen + 4)
#define SLITERATOR_EN_LIST_LEN 			6

/* Transmission buffer */
static x_usi_cmd_t sx_tx_msg;
static uint16_t sul_numCharsTxBuff;
static uint8_t puc_buffTx[MAX_LENGTH_TX_BUFFER];

#define MNGP_API_MAX_NUM           8
#define MNGP_API_INVALID           0xFF
/* Pointer to callback function to be establish */
static mngp_rsp_cb_t pf_mngp_rsp_cb[MNGP_API_MAX_NUM];
static uint8_t suc_mngp_id_req;

static uint8_t _gePibListEnLength(uint8_t *iter)
{
    uint8_t len;
    uint8_t iterType;
    uint8_t iterLength;

    iterType = (iter[0]>>ITERATOR_TYPE_SHIFT)& ITERATOR_TYPE_MASK;

    switch (iterType)
    {
        case EN_PIBQRY_SHORT_ITERATOR:
            len = SLITERATOR_EN_LIST_LEN;
            break;

        case EN_PIBQRY_LONG_ITERATOR:
            iterLength = iter[0]& ITERATOR_LENGTH_MASK;
            len = LITERATOR_EN_LIST_LEN (iterLength);
            break;
        default:
            len=0;
            break;
    }

    return len;

}

uint8_t mngLay_AddGetPibQuery(uint16_t pib, uint8_t index)
{
    uint8_t *ptrBuff;

    // Error conditions
    // if there is other message without send
    if (sx_tx_msg.uc_p_type != MNGP_PRIME_GETQRY) {
    	return FALSE;
    }

    // There is no room in the buffer
    if ((sul_numCharsTxBuff + LENGTH_GET_PIB_QUERY) >= MAX_LENGTH_TX_BUFFER) {
    	return FALSE;
    }

    ptrBuff = &puc_buffTx[sul_numCharsTxBuff];

    *ptrBuff++ = (uint8_t)(pib >> 8);
    *ptrBuff++ = (uint8_t)(pib & 0xFF);
    *ptrBuff++ = index;

    // Update the length of the message
    sul_numCharsTxBuff += 3;

    return TRUE;
}

uint8_t mngLay_AddSetPib(uint16_t pib, uint16_t length, uint8_t* msg)
{
    uint8_t *ptrBuff;

    // Error conditions
    // if there is other message without send
    if (sx_tx_msg.uc_p_type  != MNGP_PRIME_SET) {
    	return FALSE;
    }

    // There is no room in the buffer
    if ((sul_numCharsTxBuff + LENGTH_PIB + length) >= MAX_LENGTH_TX_BUFFER) {
    	return FALSE;
    }

    ptrBuff = &puc_buffTx[sul_numCharsTxBuff];

    *ptrBuff++ = (uint8_t)(pib >> 8);
    *ptrBuff++ = (uint8_t)(pib & 0xFF);
    memcpy(ptrBuff, msg, length);

    // Update the length of the message
    sul_numCharsTxBuff += LENGTH_PIB + length;

    return TRUE;
}

uint8_t mngLay_AddResetStats(uint16_t pib, uint8_t index)
{
    uint8_t *ptrBuff;

    // Error conditions
    // if there is other message without send
    if (sx_tx_msg.uc_p_type != MNGP_PRIME_RESET) {
    	return FALSE;
    }

    // There is no room in the buffer
    if ((sul_numCharsTxBuff + LENGTH_GET_PIB_QUERY) >= MAX_LENGTH_TX_BUFFER) {
    	return FALSE;
    }

    ptrBuff = &puc_buffTx[sul_numCharsTxBuff];

    *ptrBuff++ = (uint8_t)(pib >> 8);
    *ptrBuff++ = (uint8_t)(pib & 0xFF);
    *ptrBuff++ = index;

    // Update the length of the message
    sul_numCharsTxBuff += 3;

    return TRUE;
}

uint8_t mngLay_AddFUMsg(uint16_t length, uint8_t* msg)
{
    uint8_t *ptrBuff;

    // Error conditions
    // if there is other message without send
    if (sx_tx_msg.uc_p_type  != MNGP_PRIME_FU) {
    	return FALSE;
    }

    // There is no room in the buffer
    if ((sul_numCharsTxBuff + length) >= MAX_LENGTH_TX_BUFFER) {
    	return FALSE;
    }

    ptrBuff = &puc_buffTx[sul_numCharsTxBuff];

    memcpy(ptrBuff, msg, length);

    // Update the length of the message
    sul_numCharsTxBuff += length;

    return TRUE;
}

uint8_t mngLay_AddGetPibListEnQuery(uint16_t pib, uint8_t maxRecords, uint8_t* iterator)
{
    uint8_t *ptrBuff;
    uint8_t len;

    // Error conditions
    // if there is other message without send
    if (sx_tx_msg.uc_p_type != MNGP_PRIME_EN_PIBQRY) {
    	return FALSE;
    }

    //get item length
    len = _gePibListEnLength(iterator);

    // There is no room in the buffer
    if ((sul_numCharsTxBuff + len) >= MAX_LENGTH_TX_BUFFER) {
    	return FALSE;
    }

    ptrBuff = &puc_buffTx[sul_numCharsTxBuff];

    *ptrBuff++ = MNGP_PRIME_LISTQRY;
    *ptrBuff++ = (uint8_t)(pib >> 8);
    *ptrBuff++ = (uint8_t)(pib & 0xFF);
    *ptrBuff++ = maxRecords;

    memcpy(ptrBuff, iterator, len-4);

    // Update the length of the message
    sul_numCharsTxBuff += len;

    return TRUE;

}

//uint8_t mngLay_BridgeMsg(uint16_t length, uint8_t* msg)
//{
//    // If there is other message pending to send
//    if (sul_numCharsTxBuff) {
//    	return FALSE;
//    }
//
//    // Look for the type of the message
//    msg++;
//    sx_tx_msg.uc_p_type = (*msg) & 0x3f;
//    msg++;
//
//    // There is no room in the buffer
//    if ((length - 2) >= MAX_LENGTH_TX_BUFFER) {
//    	return FALSE;
//    }
//
//    // Update the length of the message
//    sul_numCharsTxBuff = length - 2;
//
//    memcpy(puc_buffTx, msg, sul_numCharsTxBuff);
//
//    return mngLay_SendMsg();
//}


uint8_t mngLay_SendMsg(uint8_t app_id)
{
	suc_mngp_id_req = app_id;

	sx_tx_msg._fd = -1;
    sx_tx_msg.us_len = sul_numCharsTxBuff;
    sx_tx_msg.puc_buf = puc_buffTx;

    if (!hal_usi_send_cmd(&sx_tx_msg)) {
    	return FALSE;
    }

    // if the transmission is ok, the message is deleted from the buffer
    sul_numCharsTxBuff = 0;
    return TRUE;
}

void mngLay_SetRspCallback(uint8_t app_id, void (*sap_handler)(uint8_t* ptrMsg, uint16_t len))
{
	pf_mngp_rsp_cb[app_id] = sap_handler;
}

uint8_t mngLay_receivedCmd(uint8_t* ptrMsg, uint16_t len)
{
    if (pf_mngp_rsp_cb[suc_mngp_id_req] && (suc_mngp_id_req < MNGP_API_MAX_NUM)) {
    	pf_mngp_rsp_cb[suc_mngp_id_req](ptrMsg,len);
    	suc_mngp_id_req = MNGP_API_INVALID;
    }
    return TRUE;
}

void mngLay_NewMsg(uint8_t cmd)
{
	sul_numCharsTxBuff = 0;
    sx_tx_msg.uc_p_type = cmd;
}

void mngLay_Init(void)
{
	memset(pf_mngp_rsp_cb, 0, sizeof(mngp_rsp_cb_t));
	suc_mngp_id_req = MNGP_API_INVALID;
	sul_numCharsTxBuff = 0;

	/* register to USI callback */
	hal_usi_set_callback(PROTOCOL_MNGP_PRIME, mngLay_receivedCmd);
}
