#ifdef __cplusplus
extern "C" {
#endif

static float back(float t) {
    return ease_back(t, 1.70158f);
}

inline static float bounce(float t) {
    static const float B[6] = {
            1.0f / 2.75f,
            2.0f / 2.75f,
            1.5f / 2.75f,
            2.5f / 2.75f,
            2.25f / 2.75f,
            2.625f / 2.75f,
    };

    t = 1.0f - t;
    if (t < B[0]) return 1.0f - 7.5625f * t * t;
    if (t < B[1]) return 1.0f - (7.5625f * (t - B[2]) * (t - B[2]) + 0.75f);
    if (t < B[3]) return 1.0f - (7.5625f * (t - B[4]) * (t - B[4]) + 0.9375f);
    return 1.0f - (7.5625f * (t - B[5]) * (t - B[5]) + 0.984375f);
}

inline static float circ(float t) {
    return 1.0f - sqrtf(1.0f - t * t);
}

inline static float elastic(float t) {
    const float amplitude = 0.1f;
    const float period = 0.4f;

    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;

    float s;
    float a = amplitude;
    if (a < 1.0f) {
        a = 1.0f;
        s = period / 4.0f;
    } else {
        s = period / (MATH_TAU * asinf(1.0f / a));
    }
    const float tm1 = t - 1.0f;
    return -(a * powf(2.0f, 10.0f * tm1) * sinf((tm1 - s) * MATH_TAU / period));
}

inline static float expo(float t) {
    return t <= 0.0f ? 0.0f : powf(2.0f, 10.0f * (t - 1.0f));
}

inline static float linear(float t) {
    return t;
}

inline static float p2(float t) { return t * t; }

inline static float p3(float t) { return t * t * t; }

inline static float p4(float t) { return t * t * t * t; }

inline static float p5(float t) { return t * t * t * t * t; }

inline static float sine(float t) {
    return t < 1.0f ? (1.0f - cosf(0.5f * MATH_PI * t)) : 1.0f;
}

inline static float step(float t) {
    return t < 1.0f ? 0.0f : 1.0f;
}

#define IMPL_EASE_FUNC(func) \
float ease_##func##_in(float t) { return ease_in(t, func); } \
float ease_##func##_inout(float t) {return ease_inout(t, func); } \
float ease_##func##_out(float t) { return ease_out(t, func); } \
float ease_##func##_outin(float t) { return ease_inout(t, func); }

IMPL_EASE_FUNC(p2)

IMPL_EASE_FUNC(p3)

IMPL_EASE_FUNC(p4)

IMPL_EASE_FUNC(p5)

IMPL_EASE_FUNC(back)

IMPL_EASE_FUNC(elastic)

IMPL_EASE_FUNC(step)

IMPL_EASE_FUNC(circ)

IMPL_EASE_FUNC(sine)

IMPL_EASE_FUNC(bounce)

IMPL_EASE_FUNC(expo)

#ifdef __cplusplus
}
#endif

