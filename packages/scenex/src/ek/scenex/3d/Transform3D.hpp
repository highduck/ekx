#pragma once

#include <ecx/ecx_fwd.hpp>
#include <ek/math.h>

struct Transform3D {
    mat4_t local = mat4_identity();
    mat4_t world = mat4_identity();
    vec3_t position = {};
    vec3_t scale = vec3(1,1,1);
    vec3_t rotation = {};

//    inline void set_euler_angles(const float3& angles) {
//        mat4f m = euler_angles(angles);
////        rotation = quat_t<float>{euler_angles};
//        rotation = quat_t<float>{m};
//    }
//
//    inline float3 get_euler_angles() const {
//        euler_angles(rotation);
//    }
};

void update_world_transform3d();

//extern ecx_component_type COMP_Transform3D;
//Transform3D* get_transform3d(entity_t e);
