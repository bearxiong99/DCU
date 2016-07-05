#ifndef __HTTP_MNG__
#define __HTTP_MNG__

#include <unistd.h>
#include <stdint.h>
#include "socket_handler.h"

#define VERSION_HI	                     0x01
#define VERSION_LOW                      0x01

#define TRUE  1
#define FALSE 0





void http_mng_init(void);
void http_mng_process(void);

#endif /* __HTTP_MNG__ */
