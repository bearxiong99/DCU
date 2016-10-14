#ifndef __HTTP_MNG__
#define __HTTP_MNG__

#include <unistd.h>
#include <stdint.h>
#include "socket_handler.h"

//#define HTTP_DEBUG_CONSOLE

#define VERSION_HI	                     0x01
#define VERSION_LOW                      0x01

#define TRUE  1
#define FALSE 0

void http_mng_init(void);
void http_mng_process(void);
void http_mng_send_cmd(uint8_t uc_cmd, void *pv_data);
void http_mng_callback(socket_ev_info_t *_ev_info);

#endif /* __HTTP_MNG__ */
