#ifndef EK_BITSET_H
#define EK_BITSET_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ek_bitset_index(uint32_t position);

uint32_t ek_bitset_byte_size(uint32_t bits);

uint64_t ek_bitset_mask(uint32_t position);

bool ek_bitset_get(const uint64_t* bitset, uint32_t position);

void ek_bitset_set(uint64_t* bitset, uint32_t position);

void ek_bitset_unset(uint64_t* bitset, uint32_t position);

void ek_bitset_flip(uint64_t* bitset, uint32_t position);

#ifdef __cplusplus
}
#endif

#endif // EK_BITSET_H
