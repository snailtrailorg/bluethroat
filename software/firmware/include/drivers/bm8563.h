#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void bm_8563_init(void);
bool bm_8563_read_time(struct tm * ps_time);

#ifdef __cplusplus
}
#endif
