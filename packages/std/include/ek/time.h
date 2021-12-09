#ifndef EK_TIME_H
#define EK_TIME_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

void ek_time_init(void);

double ek_time_now(void);

uint64_t ek_ticks(uint64_t * prev_ticks);

double ek_ticks_to_sec(uint64_t ticks);

// unix UTC in seconds
uint64_t ek_time_utc(void);

uint32_t ek_time_seed32(void);

#ifdef __cplusplus
}
#endif

#endif //EK_TIME_H
