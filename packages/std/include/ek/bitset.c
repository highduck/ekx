#include <ek/bitset.h>

uint32_t ek_bitset_index(uint32_t position) {
    return position >> 6u; // div 64
}

uint32_t ek_bitset_byte_size(uint32_t bits) {
    // add (64 - 1) to find ceil size
    bits += 63u;
    // divide by 64
    bits >>= 6u;
    // multiply by `sizeof(uint64_t)` bytes (8 bytes per 1 word)
    bits <<= 3u;
    return bits;
}

uint64_t ek_bitset_mask(uint32_t position) {
    return ((uint64_t) 1) << (position & 63u);
}

bool ek_bitset_get(const uint64_t* bitset, uint32_t position) {
    const uint32_t idx = ek_bitset_index(position);
    const uint64_t mask = ek_bitset_mask(position);
    return (bitset[idx] & mask) != 0;
}

void ek_bitset_set(uint64_t* bitset, uint32_t position) {
    const uint32_t idx = ek_bitset_index(position);
    const uint64_t mask = ek_bitset_mask(position);
    bitset[idx] |= mask;
}

void ek_bitset_unset(uint64_t* bitset, uint32_t position) {
    const uint32_t idx = ek_bitset_index(position);
    const uint64_t mask = ek_bitset_mask(position);
    bitset[idx] &= ~mask;
}

void ek_bitset_flip(uint64_t* bitset, uint32_t position) {
    const uint32_t idx = ek_bitset_index(position);
    const uint64_t mask = ek_bitset_mask(position);
    bitset[idx] ^= mask;
}

