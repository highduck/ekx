#ifndef EK_MATH_EASING_H
#define EK_MATH_EASING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef float (* easing_t)(float t);

#define DECLARE_EASE_FUNC(name) \
float ease_##name##_in(float t);\
float ease_##name##_inout(float t);\
float ease_##name##_out(float t);\
float ease_##name##_outin(float t);

DECLARE_EASE_FUNC(p2)
DECLARE_EASE_FUNC(p3)
DECLARE_EASE_FUNC(p4)
DECLARE_EASE_FUNC(p5)
DECLARE_EASE_FUNC(back)
DECLARE_EASE_FUNC(elastic)
DECLARE_EASE_FUNC(step)
DECLARE_EASE_FUNC(linear)
DECLARE_EASE_FUNC(circ)
DECLARE_EASE_FUNC(sine)
DECLARE_EASE_FUNC(bounce)
DECLARE_EASE_FUNC(expo)

#undef DECLARE_EASE_FUNC

// http://robertpenner.com/easing/

inline static float smoothstep(float x) {
    return x * x * (3.0f - 2.0f * x);
}

inline static float smootherstep(float x) {
    return x * x * x * (x * (x * 6 - 15) + 10);
}

inline static float ease_back(float t, float overshoot) {
    return t >= 1.0f ? 1.0f : t * t * ((overshoot + 1.0f) * t - overshoot);
}

inline static float ease_in(float t, easing_t eq) {
    return eq(t);
}

inline static float ease_inout(float t, easing_t eq) {
    return t < 0.5f ? 0.5f * eq(t * 2.0f) : 1.0f - 0.5f * eq(2.0f - t * 2.0f);
}

inline static float ease_out(float t, easing_t eq) {
    return 1.0f - eq(1.0f - t);
}

inline static float ease_outin(float t, easing_t eq) {
    return t < 0.5f ? 0.5f * (1.0f - eq(1.0f - t * 2.0f)) : 0.5f + 0.5f * eq(t * 2.0f - 1.0f);
}

#ifdef __cplusplus
}
#endif

#endif // EK_MATH_EASING_H
