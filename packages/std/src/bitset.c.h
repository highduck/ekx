#include <ek/bitset.h>

uint32_t bitset_index(uint32_t position) {
    return position >> 5u; // div 32
}

uint32_t bitset_words(size_t bits) {
    // div 8 bits
    return ((bits + 0x1F) >> 5u) & 0xFFFFFFFF;
}

uint32_t bitset_mask(uint32_t position) {
    return (uint32_t)(1u << (position & 0x1Fu)); // div 32
}

bool bitset_get(const uint32_t* bitset, uint32_t position) {
    const uint32_t idx = bitset_index(position);
    const uint32_t mask = bitset_mask(position);
    return (bitset[idx] & mask) != 0;
}

void bitset_set(uint32_t* bitset, uint32_t position) {
    const uint32_t idx = bitset_index(position);
    const uint32_t mask = bitset_mask(position);
    bitset[idx] |= mask;
}

void bitset_unset(uint32_t* bitset, uint32_t position) {
    const uint32_t idx = bitset_index(position);
    const uint32_t mask = bitset_mask(position);
    bitset[idx] &= ~mask;
}

void bitset_flip(uint32_t* bitset, uint32_t position) {
    const uint32_t idx = bitset_index(position);
    const uint32_t mask = bitset_mask(position);
    bitset[idx] ^= mask;
}

