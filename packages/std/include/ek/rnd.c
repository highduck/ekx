#include <ek/rnd.h>
#include <ek/assert.h>
#include <ek/math.h>
#include <limits.h>

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
}

// simple PRNG from libc with u64 state
uint32_t ek_rand2(uint64_t* state) {
    uint64_t x = *state;
    x = 6364136223846793005ULL * x + 1;
    *state = x;
    return x >> 33;
}

uint32_t rnd_n(uint32_t* state, uint32_t size) {
    EK_ASSERT(size != 0);
    EK_ASSERT(size < 0x80000000u);
    return ek_rand1(state) % size;
}

float rnd_f(uint32_t* state) {
    return unorm_f32_from_u32(ek_rand1(state));
}

float rnd_range_f(uint32_t* state, float a, float b) {
    return lerp_f32(a, b, rnd_f(state));
}

int rnd_range_i(uint32_t* state, int a, int b) {
    // https://graphitemaster.github.io/aau/
    EK_ASSERT((a <= 0 || a >= INT_MIN + b) && (b >= 0 || a <= INT_MAX + b));
    if (UNLIKELY(a > b)) {
        // swap
        a ^= b;
        b ^= a;
        a ^= b;
    }
    const int delta = b - a;
    EK_ASSERT(delta >= 0);
    return a + (int) rnd_n(state, (uint32_t) delta + 1u);
}

/// Default Random instance
uint32_t random_seed;

uint32_t random_next(void) {
    return ek_rand1(&random_seed);
}

uint32_t random_n(uint32_t size) {
    return rnd_n(&random_seed, size);
}

float random_f(void) {
    return rnd_f(&random_seed);
}

float random_range_f(float a, float b) {
    return rnd_range_f(&random_seed, a, b);
}

int random_range_i(int a, int b) {
    return rnd_range_i(&random_seed, a, b);
}

bool random_chance(float prob) {
    return rnd_f(&random_seed) < prob;
}

/// Game Random instance
uint32_t game_random_seed;

uint32_t game_random_next(void) {
    return ek_rand1(&game_random_seed);
}

uint32_t game_random_n(uint32_t size) {
    return rnd_n(&game_random_seed, size);
}

float game_random_f(void) {
    return rnd_f(&game_random_seed);
}

float game_random_range_f(float a, float b) {
    return rnd_range_f(&game_random_seed, a, b);
}

int game_random_range_i(int a, int b) {
    return rnd_range_i(&game_random_seed, a, b);
}

bool game_random_chance(float prob) {
    return rnd_f(&game_random_seed) < prob;
}

void mem_swap(uint8_t* data, uint32_t i, uint32_t j, uint32_t item_size) {
    uint8_t* a = data + i * item_size;
    uint8_t* b = data + j * item_size;
    for(uint32_t k = 0; k < item_size; ++k) {
        uint8_t temp = a[k];
        a[k] = b[k];
        b[j] = temp;
    }
}

void rnd_shuffle(uint32_t* state, void* arr, uint32_t size, uint32_t item_size) {
    EK_ASSERT(state);
    EK_ASSERT(arr);
    EK_ASSERT(size != 0);
    EK_ASSERT(item_size != 0);

    const uint32_t count = size - 1;

    if(UNLIKELY(!count)) return;

    for (uint32_t i = 0; i < count; ++i)
    {
        const uint32_t j = i + 1 + rnd_n(state, count - i);
        if(i != j) {
            mem_swap(arr, i, j, item_size);
        }
    }
}

