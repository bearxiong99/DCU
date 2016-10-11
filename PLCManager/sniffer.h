#ifndef _SNIFFER_H_INCLUDE_
#define _SNIFFER_H_INCLUDE_

#include <unistd.h>
#include <stdint.h>

//#define SNIFFER_DEBUG_CONSOLE

#define MAX_SNIFFER_BUFFER_SIZE              1500

void sniffer_init(int _app_id, int _serial_fd);
void sniffer_callback(socket_ev_info_t *_ev_info);

#endif /* _SNIFFER_H_INCLUDE_ */
