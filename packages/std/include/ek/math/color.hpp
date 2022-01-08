
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

#endif
