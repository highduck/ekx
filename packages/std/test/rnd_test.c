#include <unit.h>

#include <ek/rnd.h>
#include <ek/time.h>
#include <ek/math.h>

#include <float.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

enum {
    SAMPLES = 1000000,
    BUCKETS = 100
};

static uint32_t frange(uint32_t v, uint32_t mod) {
    return (uint32_t) (unorm_f32_from_u32(v) * (float) mod);
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


SUITE(random) {

    IT("has fine distribution") {
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

    IT("gen ranges") {

        const uint32_t N = 5;

        for (uint32_t i = 0; i < N; ++i) {
            int n = random_range_i(-100, 100);
            CHECK(n >= -100);
            CHECK(n <= 100);
            n = random_range_i(100, -100);
            CHECK(n >= -100);
            CHECK(n <= 100);

            float f = random_range_f(-100, 100);
            CHECK(f >= -100.0f);
            CHECK(f < 100.0f);

            f = random_range_f(100, -100);
            CHECK(f > -100.0f);
            CHECK(f <= 100.0f);

            // always true
            CHECK(random_chance(1.0f));
            // always false
            CHECK_FALSE(random_chance(0.0f));

            CHECK_EQ(random_n(1), 0);

            CHECK(random_f() < 1.0f);
        }
    }
}
