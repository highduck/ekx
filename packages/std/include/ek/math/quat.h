#ifndef EK_MATH_QUAT_H
#define EK_MATH_QUAT_H

#ifdef __cplusplus
extern "C" {
#endif

/** quaternion **/
typedef union quat_t {
    struct {
        float x;
        float y;
        float z;
        float w;
    };
    vec4_t xyzw;
    struct {
        vec3_t xyz;
        float _w;
    };
    float data[4];
} quat_t;

quat_t quat_normalize(quat_t a);
quat_t quat_euler_angles(vec3_t angles);
vec3_t quat_to_euler_angles(quat_t q);
quat_t quat_mat4(mat4_t m);
quat_t quat_look_at_rh(vec3_t direction, vec3_t up);
float quat_roll(quat_t q);
float quat_pitch(quat_t q);
float quat_yaw(quat_t q);

mat4_t mat4_rotation_transform_quat(quat_t q);

#ifdef __cplusplus
}
#endif

#endif // EK_MATH_QUAT_H
