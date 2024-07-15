#pragma once

#define TIME_ZONE_DEFAULT       (8)

#ifdef __cplusplus
extern "C" {
#endif

void bluethroat_clock_init(void);
void SetTimeZone(int32_t n_time_zone);

#ifdef __cplusplus
}
#endif
