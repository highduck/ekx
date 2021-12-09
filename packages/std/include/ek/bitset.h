#ifndef EK_BITSET_H
#define EK_BITSET_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t bitset_index(uint32_t position);

uint32_t bitset_words(size_t bits);

uint32_t bitset_mask(uint32_t position);

bool bitset_get(const uint32_t* bitset, uint32_t position);

void bitset_set(uint32_t* bitset, uint32_t position);

void bitset_unset(uint32_t* bitset, uint32_t position);

void bitset_flip(uint32_t* bitset, uint32_t position);

#ifdef __cplusplus
}
#endif

#endif // EK_BITSET_H
