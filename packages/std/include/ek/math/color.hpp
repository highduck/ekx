
#ifdef __cplusplus

inline static color2f_t operator-(const color2f_t a, const color2f_t b) {
    return {{a.scale - b.scale, a.offset - b.offset}};
}

inline static color2f_t operator+(const color2f_t a, const color2f_t b) {
    return {{a.scale + b.scale, a.offset + b.offset}};
}

inline static color2f_t operator*(const color2f_t a, const color2f_t b) {
    return mul_color2f(a, b);
}

inline static rgba_t operator*(const rgba_t a, const rgba_t b) {
    return mul_rgba(a, b);
}

#endif
