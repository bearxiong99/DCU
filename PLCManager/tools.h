#ifndef TOOLS_H_INCLUDED
#define TOOLS_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#define RESET_GPIO_ID          4  // S19:PA4
#define ERASE_GPIO_ID          2  // S21:PA2

#define RESET_GPIO_ENABLE      0
#define RESET_GPIO_DISABLE     1

#define ERASE_GPIO_ENABLE      1
#define ERASE_GPIO_DISABLE     0


void tools_init(void);
void tools_plc_reset(void);
void tools_plc_erase(void);
void tools_ppp0_down(void);
void tools_ppp0_up(void);
int tools_ppp0_check(void);
uint16_t tools_extract_u16(void *vptr_value);
int tools_get_timestamp_ms(void);

#endif /* TOOLS_H_INCLUDED */
