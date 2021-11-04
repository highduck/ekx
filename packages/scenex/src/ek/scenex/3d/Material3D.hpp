#pragma once

#include <ek/math/vec.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/util/Type.hpp>

namespace ek {

struct Material3D {
    constexpr static float k = 0.8f;
    float3 ambient = k * float3::one;
    float3 diffuse = k * float3::one;
    float3 specular = k * float3::one;
    float3 emission = float3::zero;
    float shininess = 32.0f;
    float roughness = 0.05f;

    inline void set_base_color(argb32_t color, float ao = 0.5f) {
        diffuse = float3{
                static_cast<float>(color.r) / 255.0f,
                static_cast<float>(color.g) / 255.0f,
                static_cast<float>(color.b) / 255.0f
        };
        ambient = ao * diffuse;
    }
};

EK_DECLARE_TYPE(Material3D);

}

