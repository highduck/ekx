#include <ek/hash.h>
#include <ek/assert.h>

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// MurmurHash2, 64-bit versions, by Austin Appleby

// The same caveats as 32-bit MurmurHash2 apply here - beware of alignment
// and endian-ness issues if used across multiple platforms.

// 64-bit hash for 64-bit platforms
// objsize: 0x170-0x321: 433

uint64_t hash_murmur2_64(const void* key, uint32_t len, uint64_t seed) {
    EK_ASSERT(len == 0 || (((uintptr_t) key) % 8) == 0);

    const uint64_t m = 0xc6a4a7935bd1e995ULL;
    const int r = 47;

    uint64_t h = seed ^ (len * m);

    const uint64_t* data = (const uint64_t*) key;
    const uint64_t* end = data + (len / 8);

    while (data != end) {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char* data2 = (const unsigned char*) data;

    switch (len & 7) {
        case 7:
            h ^= ((uint64_t) data2[6]) << 48;
            EK_FALLTHROUGH
        case 6:
            h ^= ((uint64_t) data2[5]) << 40;
            EK_FALLTHROUGH
        case 5:
            h ^= ((uint64_t) data2[4]) << 32;
            EK_FALLTHROUGH
        case 4:
            h ^= ((uint64_t) data2[3]) << 24;
            EK_FALLTHROUGH
        case 3:
            h ^= ((uint64_t) data2[2]) << 16;
            EK_FALLTHROUGH
        case 2:
            h ^= ((uint64_t) data2[1]) << 8;
            EK_FALLTHROUGH
        case 1:
            h ^= ((uint64_t) data2[0]);
            h *= m;
    }

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

static inline uint32_t rotl32(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

static inline uint32_t fmix32(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

uint32_t hash_murmur3_32(const void* key, uint32_t len, uint32_t seed) {
    // aligned version
    EK_ASSERT(len == 0 || ((uintptr_t) key % 4) == 0);

    const uint32_t* blocks = (const uint32_t*) key;
    const uint32_t nblocks = len / 4u;

    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    for (uint32_t i = 0; i < nblocks; ++i) {
        uint32_t k1 = *(blocks++);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    const uint8_t* tail = (const uint8_t*) blocks;
    uint32_t k1 = 0;
    switch (len & 3) {
        case 3:
            k1 ^= tail[2] << 16;
        case 2:
            k1 ^= tail[1] << 8;
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    };

    return fmix32(h1 ^ len);
}

#ifdef __cplusplus
}
#endif
