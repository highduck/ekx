#ifndef EK_MURMUR_H
#define EK_MURMUR_H

#include <ek/pre.h>

#define EK_MURMUR_DEFAULT_SEED 0x01000193

#ifdef __cplusplus
extern "C" {
#endif

// MurMurHash2 (64-bit, aligned)
uint64_t ek_murmur64(const void* key, uint32_t len, uint64_t seed);

#ifdef __cplusplus
}
#endif

#endif // EK_MURMUR_H
