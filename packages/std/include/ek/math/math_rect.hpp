#ifdef __cplusplus

/** rect c++ operators **/

inline static rect_t operator/(const rect_t a, const float s) {
    EK_ASSERT(s != 0.0f);
    return rect_scale_f(a, 1.0f / s);
}

inline static rect_t operator*(const rect_t a, const float s) {
    return rect_scale_f(a, s);
}

inline static rect_t operator*(const float s, const rect_t a) {
    return rect_scale_f(a, s);
}

#endif
