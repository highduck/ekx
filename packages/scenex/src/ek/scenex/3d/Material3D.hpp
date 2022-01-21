#pragma once

#include <ek/math.h>
#include <ek/rr.h>

namespace ek {

struct Material3D {
    // for c++ currently we are auto init bulk data, so this header is for `rr` debug checks
    uint32_t zero_header = 0;
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

}

struct res_material3d {
    string_hash_t names[32];
    ek::Material3D data[32];
    rr_man_t rr;
};

extern struct res_material3d res_material3d;

void setup_res_material3d(void);

