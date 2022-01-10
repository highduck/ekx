#pragma once

#include <ek/math.h>
#include <ek/util/Type.hpp>

namespace ek {

struct Material3D {
    constexpr static float k = 0.8f;
    vec3_t ambient = vec3(k, k, k);
    vec3_t diffuse = vec3(k, k, k);
    vec3_t specular = vec3(k, k, k);
    vec3_t emission = vec3(0, 0, 0);
    float shininess = 32.0f;
    float roughness = 0.05f;

    inline void set_base_color(color_t color, float ao = 0.5f) {
        diffuse = vec4_color(color).xyz;
        ambient = scale_vec3(diffuse, ao);
    }
};

EK_DECLARE_TYPE(Material3D);
EK_TYPE_INDEX(Material3D, 9);

}

