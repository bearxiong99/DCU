#ifndef FU_MNG_H_INCLUDED
#define FU_MNG_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

//#define FU_MNF_DEBUG_CONSOLE

void fu_mng_init(int _app_id);
int fu_mng_start(void);
void fu_mng_process(void);

#endif /* FU_MNG_H_INCLUDED */
