#ifndef TOOLS_H_INCLUDED
#define TOOLS_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

#define PLC_RESET_GPIO_ID          4  // S19:PA4
#define PLC_ERASE_GPIO_ID          2  // S21:PA2

#define PLC_RESET_GPIO_ENABLE      0
#define PLC_RESET_GPIO_DISABLE     1

#define PLC_ERASE_GPIO_ENABLE      1
#define PLC_ERASE_GPIO_DISABLE     0

#define GPRS_SUPPLY_GPIO_ID        13 // S10:PA13
#define GPRS_DETECT_GPIO_ID        12 // S11:PA12
#define GPRS_PWRKEY_GPIO_ID        11 // S12:PA11

#define GPRS_SUPPLY_GPIO_ENABLE    1
#define GPRS_SUPPLY_GPIO_DISABLE   0

#define GPRS_DETECT_GPIO_ENABLE    1
#define GPRS_DETECT_GPIO_DISABLE   0

#define GPRS_PWRKEY_GPIO_ENABLE    1
#define GPRS_PWRKEY_GPIO_DISABLE   0


void tools_init(void);

void tools_plc_reset(void);
void tools_plc_erase(void);
void tools_plc_down(void);
void tools_plc_up(void);
int tools_plc_check(void);

void tools_gprs_enable(void);
void tools_gprs_reset(void);
bool tools_gprs_detect(void);
void tools_gprs_down(void);
void tools_gprs_up(void);
int tools_gprs_check(void);

uint16_t tools_extract_u16(void *vptr_value);
int tools_get_timestamp_ms(void);

#endif /* TOOLS_H_INCLUDED */
