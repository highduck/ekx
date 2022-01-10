
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

inline static color_t operator*(const color_t a, const color_t b) {
    return mul_color(a, b);
}

inline static color_t operator+(const color_t a, const color_t b) {
    return add_color(a, b);
}

#endif
