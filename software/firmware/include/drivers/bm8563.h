#include <stdbool.h>
#include <stdint.h>
#include <time.h>

void bm_8563_init(void);
bool bm_8563_read_time(struct tm * ps_time);
