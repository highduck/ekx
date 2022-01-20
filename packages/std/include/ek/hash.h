#ifndef EK_MURMUR_H
#define EK_MURMUR_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_MURMUR2_64_DEFAULT_SEED 0x01000193

// MurMurHash2 (64-bit, aligned)
uint64_t hash_murmur2_64(const void* key, uint32_t len, uint64_t seed);
uint32_t hash_murmur3_32(const void* key, uint32_t len, uint32_t seed);

// inline hash calculation
// refs: https://github.com/haipome/fnv
// FNV-1a hash

#define HASH_FNV32_INIT (0x811C9DC5u)
#define HASH_FNV64_INIT (0xCBF29CE484222325ull)

static inline uint64_t hash_fnv64(const char* str, uint64_t hash) {
    while (*str) {
        hash ^= (uint64_t) *(str++);
        hash += (hash << 1) + (hash << 4) + (hash << 5) +
                (hash << 7) + (hash << 8) + (hash << 40);
    }
    return hash;
}

static inline uint32_t hash_fnv32(const char* str, uint32_t hash) {
    while (*str) {
        hash ^= (uint32_t) *(str++);
        hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
    }
    return hash;
}

// if you modify seed value, you should also modify it in `ekx` build tool project
#define H_SEED HASH_FNV32_INIT
typedef uint32_t string_hash_t;
const char* hsp_get(string_hash_t hv);

// hash_fnv32 inlined seed
static inline uint32_t hsp_hash(const char* str) {
    uint32_t hash = H_SEED;
    while (*str) {
        hash ^= (uint32_t) *(str++);
        hash += (hash << 1) + (hash << 4) + (hash << 7) + (hash << 8) + (hash << 24);
    }
    return hash;
}

#ifdef NDEBUG

#define H(S) (hsp_hash(S))

#else

string_hash_t hsp_hash_debug(const char* str);
#define H(S) (hsp_hash_debug(S))

#endif

#ifdef __cplusplus
}
#endif

#endif // EK_MURMUR_H
