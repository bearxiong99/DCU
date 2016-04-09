#ifndef _PRIME_MNG_H_INCLUDE_
#define _PRIME_MNG_H_INCLUDE_

#include <unistd.h>
#include <stdint.h>

#define MAX_PRIMEMNG_BUFFER_SIZE              1500

void prime_manager_init(int _app_id, int _serial_fd);
void prime_manager_callback(socket_ev_info_t *_ev_info);

#endif /* _PRIME_MNG_H_INCLUDE_ */
