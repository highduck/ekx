#ifdef __cplusplus

/** vec2 c++ operators **/
inline static vec2_t operator-(const vec2_t a) {
    return {{-a.x, -a.y}};
}

inline static vec2_t operator-(const vec2_t a, const vec2_t b) {
    return {{a.x - b.x, a.y - b.y}};
}

inline static vec2_t operator+(const vec2_t a, const vec2_t b) {
    return {{a.x + b.x, a.y + b.y}};
}

inline static vec2_t operator*(const vec2_t a, const vec2_t b) {
    return {{a.x * b.x, a.y * b.y}};
}

inline static vec2_t operator/(const vec2_t a, const vec2_t b) {
    return {{a.x / b.x, a.y / b.y}};
}

inline static vec2_t operator*(const vec2_t a, float s) {
    return {{s * a.x, s * a.y}};
}

inline static vec2_t operator*(float s, const vec2_t a) {
    return {{s * a.x, s * a.y}};
}

inline static vec2_t operator/(const vec2_t a, float s) {
    const float inv = 1.0f / s;
    return {{inv * a.x, inv * a.y}};
}

inline static vec2_t& operator*=(vec2_t& a, const float s) {
    a.x *= s;
    a.y *= s;
    return a;
}

inline static vec2_t& operator/=(vec2_t& a, const float s) {
    const float inv = 1.0f / s;
    a.x *= inv;
    a.y *= inv;
    return a;
}

inline static vec2_t& operator+=(vec2_t& a, const vec2_t b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

inline static vec2_t& operator-=(vec2_t& a, const vec2_t b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

inline static vec2_t& operator/=(vec2_t& a, const vec2_t b) {
    a.x /= b.x;
    a.y /= b.y;
    return a;
}

inline static vec2_t& operator*=(vec2_t& a, const vec2_t b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

inline static bool operator==(const vec2_t a, const vec2_t b) {
    return a.x == b.x && a.y == b.y;
}

inline static bool operator!=(const vec2_t a, const vec2_t b) {
    return !operator==(a, b);
}

/** vec4 c++ operators **/
inline static vec4_t operator-(const vec4_t a) {
    return {{-a.x, -a.y, -a.z, -a.w}};
}

inline static vec4_t operator-(const vec4_t a, const vec4_t b) {
    return {{
                    a.x - b.x,
                    a.y - b.y,
                    a.z - b.z,
                    a.w - b.w
            }};
}

inline static vec4_t operator+(const vec4_t a, const vec4_t b) {
    return {{
                    a.x + b.x,
                    a.y + b.y,
                    a.z + b.z,
                    a.w + b.w,
            }};
}

inline static vec4_t operator*(const vec4_t a, const vec4_t b) {
    return {{
                    a.x * b.x,
                    a.y * b.y,
                    a.z * b.z,
                    a.w * b.w,
            }};
}

inline static vec4_t operator/(const vec4_t a, const vec4_t b) {
    return {{
                    a.x / b.x,
                    a.y / b.y,
                    a.z / b.z,
                    a.w / b.w,
            }};
}

inline static vec4_t operator*(const vec4_t a, float s) {
    return {{
                    s * a.x,
                    s * a.y,
                    s * a.z,
                    s * a.w,
            }};
}

inline static vec4_t operator/(const vec4_t a, float s) {
    const float inv = 1.0f / s;
    return {{
                    inv * a.x,
                    inv * a.y,
                    inv * a.z,
                    inv * a.w,
            }};
}

inline static vec4_t& operator*=(vec4_t& a, const float s) {
    a.x *= s;
    a.y *= s;
    a.z *= s;
    a.w *= s;
    return a;
}

inline static vec4_t& operator/=(vec4_t& a, const float s) {
    const float inv = 1.0f / s;
    a.x *= inv;
    a.y *= inv;
    a.z *= inv;
    a.w *= inv;
    return a;
}

inline static vec4_t& operator+=(vec4_t& a, const vec4_t b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

inline static vec4_t& operator-=(vec4_t& a, const vec4_t b) {
    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;
    return a;
}

inline static vec4_t& operator/=(vec4_t& a, const vec4_t b) {
    a.x /= b.x;
    a.y /= b.y;
    a.z /= b.z;
    a.w /= b.w;
    return a;
}

inline static vec4_t& operator*=(vec4_t& a, const vec4_t b) {
    a.x *= b.x;
    a.y *= b.y;
    a.z *= b.z;
    a.w *= b.w;
    return a;
}

inline static bool operator==(const vec4_t a, const vec4_t b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline static bool operator!=(const vec4_t a, const vec4_t b) {
    return !operator==(a, b);
}

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
