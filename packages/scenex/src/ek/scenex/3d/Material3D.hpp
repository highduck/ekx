#pragma once

#include <ek/math/Color32.hpp>
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

    inline void set_base_color(argb32_t color, float ao = 0.5f) {
        diffuse = vec3(
                (float) color.r / 255.0f,
                (float) color.g / 255.0f,
                (float) color.b / 255.0f
        );
        ambient = vec3_scale(diffuse, ao);
    }
};

EK_DECLARE_TYPE(Material3D);
EK_TYPE_INDEX(Material3D, 9);

}

