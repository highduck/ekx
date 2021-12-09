#pragma once

#include <ek/math/Vec.hpp>
#include <ek/math/Color32.hpp>
#include <ek/util/Type.hpp>

namespace ek {

struct Material3D {
    constexpr static float k = 0.8f;
    Vec3f ambient = k * Vec3f::one;
    Vec3f diffuse = k * Vec3f::one;
    Vec3f specular = k * Vec3f::one;
    Vec3f emission = Vec3f::zero;
    float shininess = 32.0f;
    float roughness = 0.05f;

    inline void set_base_color(argb32_t color, float ao = 0.5f) {
        diffuse = Vec3f{
                static_cast<float>(color.r) / 255.0f,
                static_cast<float>(color.g) / 255.0f,
                static_cast<float>(color.b) / 255.0f
        };
        ambient = ao * diffuse;
    }
};

EK_DECLARE_TYPE(Material3D);
EK_TYPE_INDEX(Material3D, 9);

}

