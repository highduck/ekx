#ifdef __cplusplus

/// vec2_t operator overloading
inline static vec2_t operator-(const vec2_t a) {
    return {{ -a.x, -a.y }};
}

inline static vec2_t operator-(const vec2_t a, const vec2_t b) {
    return {{ a.x - b.x, a.y - b.y }};
}

inline static vec2_t operator+(const vec2_t a, const vec2_t b) {
    return {{ a.x + b.x, a.y + b.y }};
}

inline static vec2_t operator*(const vec2_t a, const vec2_t b) {
    return {{ a.x * b.x, a.y * b.y }};
}

inline static vec2_t operator/(const vec2_t a, const vec2_t b) {
    EK_ASSERT( b.x != 0 && b.y != 0 );
    return {{ a.x / b.x, a.y / b.y }};
}

inline static vec2_t operator*(const vec2_t a, float s) {
    return {{ s * a.x, s * a.y }};
}

inline static vec2_t operator*(float s, const vec2_t a) {
    return operator*(a, s);
}

inline static vec2_t operator/(const vec2_t a, float s) {
    EK_ASSERT(s != 0);
    const float inv = 1.0f / s;
    return {{ inv * a.x, inv * a.y }};
}

inline static vec2_t& operator*=(vec2_t& a, const float s) {
    a.x *= s; a.y *= s;
    return a;
}

inline static vec2_t& operator/=(vec2_t& a, const float s) {
    EK_ASSERT(s != 0);
    const float inv = 1.0f / s;
    return operator*=(a, inv);
}

inline static vec2_t& operator+=(vec2_t& a, const vec2_t b) {
    a.x += b.x; a.y += b.y;
    return a;
}

inline static vec2_t& operator-=(vec2_t& a, const vec2_t b) {
    a.x -= b.x; a.y -= b.y;
    return a;
}

inline static vec2_t& operator/=(vec2_t& a, const vec2_t b) {
    EK_ASSERT( b.x != 0 && b.y != 0 );
    a.x /= b.x; a.y /= b.y;
    return a;
}

inline static vec2_t& operator*=(vec2_t& a, const vec2_t b) {
    a.x *= b.x; a.y *= b.y;
    return a;
}

inline static bool operator==(const vec2_t a, const vec2_t b) {
    return a.x == b.x && a.y == b.y;
}

inline static bool operator!=(const vec2_t a, const vec2_t b) {
    return !operator==(a, b);
}

/// vec3_t operator overloading
inline static vec3_t operator-(const vec3_t a) {
    return {{ -a.x, -a.y, -a.z }};
}

inline static vec3_t operator-(const vec3_t a, const vec3_t b) {
    return {{ a.x - b.x, a.y - b.y, a.z - b.z }};
}

inline static vec3_t operator+(const vec3_t a, const vec3_t b) {
    return {{ a.x + b.x, a.y + b.y, a.z + b.z }};
}

inline static vec3_t operator*(const vec3_t a, const vec3_t b) {
    return {{ a.x * b.x, a.y * b.y, a.z * b.z }};
}

inline static vec3_t operator/(const vec3_t a, const vec3_t b) {
    EK_ASSERT( b.x != 0 && b.y != 0 && b.z != 0 );
    return {{ a.x / b.x, a.y / b.y, a.z / b.z }};
}

inline static vec3_t operator*(const vec3_t a, float s) {
    return {{ s * a.x, s * a.y, s * a.z }};
}

inline static vec3_t operator*(float s, const vec3_t a) {
    return operator*(a, s);
}

inline static vec3_t operator/(const vec3_t a, float s) {
    EK_ASSERT(s != 0);
    const float inv = 1.0f / s;
    return {{ inv * a.x, inv * a.y, inv * a.z }};
}

inline static vec3_t& operator*=(vec3_t& a, const float s) {
    a.x *= s; a.y *= s; a.z *= s;
    return a;
}

inline static vec3_t& operator/=(vec3_t& a, const float s) {
    EK_ASSERT(s != 0);
    const float inv = 1.0f / s;
    return operator*=(a, inv);
}

inline static vec3_t& operator+=(vec3_t& a, const vec3_t b) {
    a.x += b.x; a.y += b.y; a.z += b.z;
    return a;
}

inline static vec3_t& operator-=(vec3_t& a, const vec3_t b) {
    a.x -= b.x; a.y -= b.y; a.z -= b.z;
    return a;
}

inline static vec3_t& operator/=(vec3_t& a, const vec3_t b) {
    EK_ASSERT( b.x != 0 && b.y != 0 && b.z != 0 );
    a.x /= b.x; a.y /= b.y; a.z /= b.z;
    return a;
}

inline static vec3_t& operator*=(vec3_t& a, const vec3_t b) {
    a.x *= b.x; a.y *= b.y; a.z *= b.z;
    return a;
}

inline static bool operator==(const vec3_t a, const vec3_t b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

inline static bool operator!=(const vec3_t a, const vec3_t b) {
    return !operator==(a, b);
}

/// vec4_t operator overloading
inline static vec4_t operator-(const vec4_t a) {
    return {{ -a.x, -a.y, -a.z, -a.w }};
}

inline static vec4_t operator-(const vec4_t a, const vec4_t b) {
    return {{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }};
}

inline static vec4_t operator+(const vec4_t a, const vec4_t b) {
    return {{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }};
}

inline static vec4_t operator*(const vec4_t a, const vec4_t b) {
    return {{ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }};
}

inline static vec4_t operator/(const vec4_t a, const vec4_t b) {
    EK_ASSERT( b.x != 0 && b.y != 0 && b.z != 0 && b.w != 0 );
    return {{ a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }};
}

inline static vec4_t operator*(const vec4_t a, float s) {
    return {{ s * a.x, s * a.y, s * a.z, s * a.w }};
}

inline static vec4_t operator*(float s, const vec4_t a) {
    return operator*(a, s);
}

inline static vec4_t operator/(const vec4_t a, float s) {
    EK_ASSERT(s != 0);
    const float inv = 1.0f / s;
    return {{ inv * a.x, inv * a.y, inv * a.z, inv * a.w }};
}

inline static vec4_t& operator*=(vec4_t& a, const float s) {
    a.x *= s; a.y *= s; a.z *= s; a.w *= s;
    return a;
}

inline static vec4_t& operator/=(vec4_t& a, const float s) {
    EK_ASSERT(s != 0);
    const float inv = 1.0f / s;
    return operator*=(a, inv);
}

inline static vec4_t& operator+=(vec4_t& a, const vec4_t b) {
    a.x += b.x; a.y += b.y; a.z += b.z; a.w += b.w;
    return a;
}

inline static vec4_t& operator-=(vec4_t& a, const vec4_t b) {
    a.x -= b.x; a.y -= b.y; a.z -= b.z; a.w -= b.w;
    return a;
}

inline static vec4_t& operator/=(vec4_t& a, const vec4_t b) {
    EK_ASSERT( b.x != 0 && b.y != 0 && b.z != 0 && b.w != 0 );
    a.x /= b.x; a.y /= b.y; a.z /= b.z; a.w /= b.w;
    return a;
}

inline static vec4_t& operator*=(vec4_t& a, const vec4_t b) {
    a.x *= b.x; a.y *= b.y; a.z *= b.z; a.w *= b.w;
    return a;
}

inline static bool operator==(const vec4_t a, const vec4_t b) {
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

inline static bool operator!=(const vec4_t a, const vec4_t b) {
    return !operator==(a, b);
}

#endif // __cplusplus
