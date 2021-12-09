#include <ek/rnd.h>
#include <ek/assert.h>

// Mersenne Twister tempering

#define TEMPERING_MASK_B    0x9D2C5680
#define TEMPERING_MASK_C    0xEFC60000

static uint32_t ek_temper(uint32_t x) {
    x ^= x >> 11;
    x ^= (x << 7) & TEMPERING_MASK_B;
    x ^= (x << 15) & TEMPERING_MASK_C;
    x ^= x >> 18;
    return x;
}

// simple PRNG from libc with u32 state
uint32_t ek_rand1(uint32_t* state) {
    uint32_t x = *state;
    x = x * 1103515245 + 12345;
    *state = x;
    return ek_temper(x) >> 1;
//    return x >> 1;
}

// simple PRNG from libc with u64 state
uint32_t ek_rand2(uint64_t* state) {
    uint64_t x = *state;
    x = 6364136223846793005ULL * x + 1;
    *state = x;
    return x >> 33;
}

// generate normalized f32 values in range [0; 1) from random u32
float ek_norm_f32_from_u32(uint32_t value) {
    uint32_t exponent = 127;
    uint32_t mantissa = value & ((1 << 23) - 1);
    union {
        uint32_t u32;
        float f32;
    } bits;
    bits.u32 = (exponent << 23) | mantissa;
    const float f = bits.f32 - 1.0f;
    EK_ASSERT_R2(f >= 0.0f);
    EK_ASSERT_R2(f < 1.0f);
    return f;
    }
