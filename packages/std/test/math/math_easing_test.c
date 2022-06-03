#include "math_test_common.h"

const int N = 25;

static const char* test_easing_fn(easing_t ease_fn, bool in_bounds) {
    if (!ease_fn) {
        return "ease_fn should not be NULL";
    }
    if (!almost_eq_f32(0.0f, ease_fn(0), MATH_F32_EPSILON)) {
        return "ease_fn(0) should be near 0";
    }
    if (!almost_eq_f32(1.0f, ease_fn(1), MATH_F32_EPSILON)) {
        return "ease_fn(1) should be near 1";
    }
    for (int i = 0; i <= N; ++i) {
        float x = (float) i / (float) N;
        const float R = ease_fn(x);
        if (!isfinite(R)) {
            return "Ease function must provide finite float values in interval from 0 to 1";
        }
        if (in_bounds) {
            if (R < 0.0f) {
                return "Bounded ease function should not be less than 0";
            }
            if (R > 1.0f) {
                return "Bounded ease function should not be greater than 1";
            }
        }
    }
    return NULL;
}

SUITE(easing_math) {

    IT("is simple") {
        CHECK_FALSE(test_easing_fn(smoothstep, true));
        CHECK_FALSE(test_easing_fn(smootherstep, true));
        CHECK_FALSE(test_easing_fn(ease_linear, true));
    }

    IT("has step") {
        CHECK_FALSE(test_easing_fn(ease_step_in, true));
        CHECK_FALSE(test_easing_fn(ease_step_inout, true));
        CHECK_FALSE(test_easing_fn(ease_step_out, true));
        CHECK_FALSE(test_easing_fn(ease_step_outin, true));
    }

    IT("has quadratic") {
        CHECK_FALSE(test_easing_fn(ease_p2_in, true));
        CHECK_FALSE(test_easing_fn(ease_p2_inout, true));
        CHECK_FALSE(test_easing_fn(ease_p2_out, true));
        CHECK_FALSE(test_easing_fn(ease_p2_outin, true));
    }

    IT("has cubic") {
        CHECK_FALSE(test_easing_fn(ease_p3_in, true));
        CHECK_FALSE(test_easing_fn(ease_p3_inout, true));
        CHECK_FALSE(test_easing_fn(ease_p3_out, true));
        CHECK_FALSE(test_easing_fn(ease_p3_outin, true));
    }

    IT("has quart") {
        CHECK_FALSE(test_easing_fn(ease_p4_in, true));
        CHECK_FALSE(test_easing_fn(ease_p4_inout, true));
        CHECK_FALSE(test_easing_fn(ease_p4_out, true));
        CHECK_FALSE(test_easing_fn(ease_p4_outin, true));
    }

    IT("has quint") {
        CHECK_FALSE(test_easing_fn(ease_p5_in, true));
        CHECK_FALSE(test_easing_fn(ease_p5_inout, true));
        CHECK_FALSE(test_easing_fn(ease_p5_out, true));
        CHECK_FALSE(test_easing_fn(ease_p5_outin, true));
    }

    IT("has circ") {
        CHECK_FALSE(test_easing_fn(ease_circ_in, true));
        CHECK_FALSE(test_easing_fn(ease_circ_inout, true));
        CHECK_FALSE(test_easing_fn(ease_circ_out, true));
        CHECK_FALSE(test_easing_fn(ease_circ_outin, true));
    }

    IT("has expo") {
        CHECK_FALSE(test_easing_fn(ease_expo_in, true));
        CHECK_FALSE(test_easing_fn(ease_expo_inout, true));
        CHECK_FALSE(test_easing_fn(ease_expo_out, true));
        CHECK_FALSE(test_easing_fn(ease_expo_outin, true));
    }

    IT("has sine") {
        CHECK_FALSE(test_easing_fn(ease_sine_in, true));
        CHECK_FALSE(test_easing_fn(ease_sine_inout, true));
        CHECK_FALSE(test_easing_fn(ease_sine_out, true));
        CHECK_FALSE(test_easing_fn(ease_sine_outin, true));
    }

    IT("has bounce") {
        CHECK_FALSE(test_easing_fn(ease_bounce_in, false));
        CHECK_FALSE(test_easing_fn(ease_bounce_inout, false));
        CHECK_FALSE(test_easing_fn(ease_bounce_out, false));
        CHECK_FALSE(test_easing_fn(ease_bounce_outin, false));
    }

    IT("has back") {
        CHECK_FALSE(test_easing_fn(ease_back_in, false));
        CHECK_FALSE(test_easing_fn(ease_back_inout, false));
        CHECK_FALSE(test_easing_fn(ease_back_out, false));
        CHECK_FALSE(test_easing_fn(ease_back_outin, false));
    }

    IT("has elastic") {
        CHECK_FALSE(test_easing_fn(ease_elastic_in, false));
        CHECK_FALSE(test_easing_fn(ease_elastic_inout, false));
        CHECK_FALSE(test_easing_fn(ease_elastic_out, false));
        CHECK_FALSE(test_easing_fn(ease_elastic_outin, false));
    }

}