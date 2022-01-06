#pragma once

#include <ek/math.h>

namespace ek {

enum class Light3DType {
    Directional,
    Point,
    Spot
};

struct Light3D {
    Light3DType type = Light3DType::Point;

   vec3_t ambient = vec3(0.1f, 0.1f, 0.1f);
   vec3_t diffuse = vec3(1, 1, 1);
   vec3_t specular = vec3(1, 1, 1);

    float radius = 100.0f;
    float falloff = 1.0f;

    bool cast_shadows = true;
};

ECX_TYPE(28, Light3D);

}

