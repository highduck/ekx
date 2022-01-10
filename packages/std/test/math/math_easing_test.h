#ifndef EK_MATH_EASING_TEST_H
#define EK_MATH_EASING_TEST_H

#include "math_test_common.h"

TEST_SUITE_BEGIN("math: easing");

static void check_easing_eq(easing_t ease_func, bool in_bounds) {
    CHECK(ease_func);
    CHECK_NEAR_EQ(0.0f, ease_func(0));
    CHECK_NEAR_EQ(1.0f, ease_func(1));
    const int N = 100;
    for(int i = 0; i <= N; ++i) {
        float x = (float)i / (float)N;
        const float R = ease_func(x);
        if(!isfinite(R)) {
            FAIL_CHECK("Easing must provide finite float value in 0..1 interval");
        }
        if(in_bounds) {
            CHECK(0.0f <= R);
            CHECK(1.0f >= R);
        }
    }
}

TEST_CASE("simple") {
    check_easing_eq(smoothstep, true);
    check_easing_eq(smootherstep, true);
    check_easing_eq(ease_linear, true);
}

TEST_CASE("step") {
    check_easing_eq(ease_step_in, true);
    check_easing_eq(ease_step_inout, true);
    check_easing_eq(ease_step_out, true);
    check_easing_eq(ease_step_outin, true);
}

TEST_CASE("quadratic") {
    check_easing_eq(ease_p2_in, true);
    check_easing_eq(ease_p2_inout, true);
    check_easing_eq(ease_p2_out, true);
    check_easing_eq(ease_p2_outin, true);
}

TEST_CASE("cubic") {
    check_easing_eq(ease_p3_in, true);
    check_easing_eq(ease_p3_inout, true);
    check_easing_eq(ease_p3_out, true);
    check_easing_eq(ease_p3_outin, true);
}

TEST_CASE("quart") {
    check_easing_eq(ease_p4_in, true);
    check_easing_eq(ease_p4_inout, true);
    check_easing_eq(ease_p4_out, true);
    check_easing_eq(ease_p4_outin, true);
}

TEST_CASE("quint") {
    check_easing_eq(ease_p5_in, true);
    check_easing_eq(ease_p5_inout, true);
    check_easing_eq(ease_p5_out, true);
    check_easing_eq(ease_p5_outin, true);
}

TEST_CASE("circ") {
    check_easing_eq(ease_circ_in, true);
    check_easing_eq(ease_circ_inout, true);
    check_easing_eq(ease_circ_out, true);
    check_easing_eq(ease_circ_outin, true);
}

TEST_CASE("expo") {
    check_easing_eq(ease_expo_in, true);
    check_easing_eq(ease_expo_inout, true);
    check_easing_eq(ease_expo_out, true);
    check_easing_eq(ease_expo_outin, true);
}

TEST_CASE("sine") {
    check_easing_eq(ease_sine_in, true);
    check_easing_eq(ease_sine_inout, true);
    check_easing_eq(ease_sine_out, true);
    check_easing_eq(ease_sine_outin, true);
}

TEST_CASE("bounce") {
    check_easing_eq(ease_bounce_in, false);
    check_easing_eq(ease_bounce_inout, false);
    check_easing_eq(ease_bounce_out, false);
    check_easing_eq(ease_bounce_outin, false);
}

TEST_CASE("back") {
    check_easing_eq(ease_back_in, false);
    check_easing_eq(ease_back_inout, false);
    check_easing_eq(ease_back_out, false);
    check_easing_eq(ease_back_outin, false);
}

TEST_CASE("elastic") {
    check_easing_eq(ease_elastic_in, false);
    check_easing_eq(ease_elastic_inout, false);
    check_easing_eq(ease_elastic_out, false);
    check_easing_eq(ease_elastic_outin, false);
}

#endif // EK_MATH_EASING_TEST_H
