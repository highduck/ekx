
#ifdef __cplusplus
extern "C" {
#endif

/** quaternion functions **/
quat_t quat_euler_angles(vec3_t angles) {
    vec3_t c = vec3(
            cosf(angles.x * 0.5f),
            cosf(angles.y * 0.5f),
            cosf(angles.z * 0.5f)
    );
    vec3_t s = vec3(
            sinf(angles.x * 0.5f),
            sinf(angles.y * 0.5f),
            sinf(angles.z * 0.5f)
    );

    quat_t result;
    result.x = s.x * c.y * c.z - c.x * s.y * s.z;
    result.y = c.x * s.y * c.z + s.x * c.y * s.z;
    result.z = c.x * c.y * s.z - s.x * s.y * c.z;
    result.w = c.x * c.y * c.z + s.x * s.y * s.z;
    return result;
}

quat_t quat_mat3(mat3_t m) {
    float four_x_squared_minus_1 = m.m00 - m.m11 - m.m22;
    float four_y_squared_minus_1 = m.m11 - m.m00 - m.m22;
    float four_z_squared_minus_1 = m.m22 - m.m00 - m.m11;
    float four_w_squared_minus_1 = m.m00 + m.m11 + m.m22;

    int biggest_index = 0;
    float four_biggest_squared_minus_1 = four_w_squared_minus_1;
    if (four_x_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_x_squared_minus_1;
        biggest_index = 1;
    }
    if (four_y_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_y_squared_minus_1;
        biggest_index = 2;
    }
    if (four_z_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_z_squared_minus_1;
        biggest_index = 3;
    }

    float biggest_val = sqrtf(four_biggest_squared_minus_1 + 1) * 0.5f;
    float mult = 0.25f / biggest_val;
    quat_t result;

    switch (biggest_index) {
        case 0:
            result.x = (m.m12 - m.m21) * mult;
            result.y = (m.m20 - m.m02) * mult;
            result.z = (m.m01 - m.m10) * mult;
            result.w = biggest_val;
            break;
        case 1:
            result.x = biggest_val;
            result.y = (m.m01 + m.m10) * mult;
            result.z = (m.m20 + m.m02) * mult;
            result.w = (m.m12 - m.m21) * mult;
            break;
        case 2:
            result.x = (m.m01 + m.m10) * mult;
            result.y = biggest_val;
            result.z = (m.m12 + m.m21) * mult;
            result.w = (m.m20 - m.m02) * mult;
            break;
        case 3:
            result.x = (m.m20 + m.m02) * mult;
            result.y = (m.m12 + m.m21) * mult;
            result.z = biggest_val;
            result.w = (m.m01 - m.m10) * mult;
            break;
        default:
            // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
            EK_ASSERT(false);
            result.x = 0;
            result.y = 0;
            result.z = 0;
            result.w = 1;
            break;
    }
    return result;
}

quat_t quat_mat4(mat4_t m) {
    const float four_x_squared_minus_1 = m.m00 - m.m11 - m.m22;
    const float four_y_squared_minus_1 = m.m11 - m.m00 - m.m22;
    const float four_z_squared_minus_1 = m.m22 - m.m00 - m.m11;
    const float four_w_squared_minus_1 = m.m00 + m.m11 + m.m22;

    int biggest_index = 0;
    float four_biggest_squared_minus_1 = four_w_squared_minus_1;
    if (four_x_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_x_squared_minus_1;
        biggest_index = 1;
    }
    if (four_y_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_y_squared_minus_1;
        biggest_index = 2;
    }
    if (four_z_squared_minus_1 > four_biggest_squared_minus_1) {
        four_biggest_squared_minus_1 = four_z_squared_minus_1;
        biggest_index = 3;
    }

    const float biggest_val = sqrtf(four_biggest_squared_minus_1 + 1) * 0.5f;
    const float mult = 0.25f / biggest_val;

    quat_t result;
    switch (biggest_index) {
        case 0:
            result.w = biggest_val;
            result.x = (m.m12 - m.m21) * mult;
            result.y = (m.m20 - m.m02) * mult;
            result.z = (m.m01 - m.m10) * mult;
            break;
        case 1:
            result.w = (m.m12 - m.m21) * mult;
            result.x = biggest_val;
            result.y = (m.m01 + m.m10) * mult;
            result.z = (m.m20 + m.m02) * mult;
            break;
        case 2:
            result.w = (m.m20 - m.m02) * mult;
            result.x = (m.m01 + m.m10) * mult;
            result.y = biggest_val;
            result.z = (m.m12 + m.m21) * mult;
            break;
        case 3:
            result.w = (m.m01 - m.m10) * mult;
            result.x = (m.m20 + m.m02) * mult;
            result.y = (m.m12 + m.m21) * mult;
            result.z = biggest_val;
            break;
        default:
            // Should never actually get here
            // Assert is just for sanity
            EK_ASSERT(false);
            result.x = 0;
            result.y = 0;
            result.z = 0;
            result.w = 1;
            break;
    }
    return result;
}

quat_t normalize_quat(quat_t a) {
    quat_t result;
    result.xyzw = normalize_vec4(a.xyzw);
    return result;
}

quat_t quat_between(const vec3_t u, const vec3_t v) {
    float norm_u_norm_v = sqrtf(dot_vec3(u, u) * dot_vec3(v, v));
    float real_part = norm_u_norm_v + dot_vec3(u, v);
    vec3_t t;

    if (real_part < 1.e-6f * norm_u_norm_v) {
        // If u and v are exactly opposite, rotate 180 degrees
        // around an arbitrary orthogonal axis. Axis normalisation
        // can happen later, when we normalise the quaternion.
        real_part = 0;
        t = fabsf(u.x) > fabsf(u.z) ? vec3(-u.y, u.x, 0) : vec3(0, -u.z, u.y);
    } else {
        // Otherwise, build quaternion the standard way.
        t = cross_vec3(u, v);
    }

    quat_t q;
    q.w = real_part;
    q.xyz = t;

    return normalize_quat(q);
}

quat_t quat_look_at_rh(vec3_t direction, vec3_t up) {
    const vec3_t u2 = neg_vec3(direction);
    const vec3_t u0 = normalize_vec3(cross_vec3(up, u2));
    const vec3_t u1 = cross_vec3(u2, u0);

    const mat3_t m = {
            .columns[0] = u0,
            .columns[1] = u1,
            .columns[2] = u2
    };

    return quat_mat3(m);
}

float quat_roll(quat_t q) {
    return atan2f(
            2.0f * (q.x * q.y + q.w * q.z),
            q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z
    );
}

float quat_pitch(quat_t q) {
    //return T(atan(T(2) * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z));
    const float y = 2 * (q.y * q.z + q.w * q.x);
    const float x = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;

    //avoid atan2(0,0) - handle singularity - Matiis
    if (almost_eq_f32(x, 0, MATH_F32_EPSILON) && almost_eq_f32(y, 0, MATH_F32_EPSILON)) {
        return 2 * atan2f(q.x, q.w);
    }

    return atan2f(y, x);
}

float quat_yaw(quat_t q) {
    return asinf(clamp(-2 * (q.x * q.z - q.w * q.y), -1, 1));
}

vec3_t quat_to_euler_angles(quat_t q) {
    return vec3(quat_pitch(q), quat_yaw(q), quat_roll(q));
}


mat4_t mat4_rotation_transform_quat(const quat_t q) {
    const float qxx = q.x * q.x;
    const float qyy = q.y * q.y;
    const float qzz = q.z * q.z;
    const float qxz = q.x * q.z;
    const float qxy = q.x * q.y;
    const float qyz = q.y * q.z;
    const float qwx = q.w * q.x;
    const float qwy = q.w * q.y;
    const float qwz = q.w * q.z;

    mat4_t m = mat4_identity();
    m.m00 = 1.0f - 2 * (qyy + qzz);
    m.m01 = 2 * (qxy + qwz);
    m.m02 = 2 * (qxz - qwy);
    m.m10 = 2 * (qxy - qwz);
    m.m11 = 1.0f - 2 * (qxx + qzz);
    m.m12 = 2 * (qyz + qwx);
    m.m20 = 2 * (qxz + qwy);
    m.m21 = 2 * (qyz - qwx);
    m.m22 = 1.0f - 2 * (qxx + qyy);
    return m;
}

#ifdef __cplusplus
}
#endif
