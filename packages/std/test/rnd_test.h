#ifndef RND_TEST_H
#define RND_TEST_H

#include <doctest.h>
#include <ek/rnd.h>
#include <ek/time.h>

TEST_SUITE_BEGIN("random");

enum {
    SAMPLES = 1000000,
    BUCKETS = 100
};

static uint32_t frange(uint32_t v, uint32_t mod) {
    return (uint32_t) (ek_norm_f32_from_u32(v) * (float) mod);
}

static void print_stats(const int* buckets) {
    double avg = 0.0;
    double min = DBL_MAX;
    double max = DBL_MIN;
    for (int i = 0; i < BUCKETS; ++i) {
        int v = buckets[i];
        avg += v;
        if (v > max) max = v;
        if (v < min) min = v;
    }
    printf("AVG: %0.3lf\n", avg / SAMPLES);
    printf("MIN: %0.3lf\n", min * BUCKETS / SAMPLES);
    printf("MAX: %0.3lf\n", max * BUCKETS / SAMPLES);
}


#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#endif

TEST_CASE("distribution") {
    ek_time_init();
    int buckets[BUCKETS];
    memset(buckets, 0, sizeof(buckets));

    uint32_t rnd1 = ek_time_seed32();
    uint32_t prev = 0;
    uint32_t repeats = 0;
    for (int i = 0; i < SAMPLES; ++i) {
        uint32_t idx = frange(ek_rand1(&rnd1), BUCKETS);
        if (idx == prev) ++repeats;
        prev = idx;
        ++buckets[idx];
    }
    print_stats(buckets);
    printf("repeats: %d\n", repeats);
    memset(buckets, 0, sizeof(buckets));
    uint64_t rnd2 = ek_time_seed32();
    prev = 0;
    repeats = 0;
    for (int i = 0; i < SAMPLES; ++i) {
        uint32_t idx = frange(ek_rand2(&rnd2), BUCKETS);
        if (idx == prev) ++repeats;
        prev = idx;
        ++buckets[idx];
    }
    print_stats(buckets);
    printf("repeats: %d\n", repeats);
}

TEST_SUITE_END();

#endif // RND_TEST_H
