#ifndef IFACEADP_API_H
#define IFACEADP_API_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

void ifaceAdp_api_init();
void ifaceAdp_select_api(uint8_t adp_app_id);

#endif // IFACEADP_API_H
