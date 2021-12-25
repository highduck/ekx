#pragma once

#include <ek/math/Math.hpp>
#include <ek/math/Vec.hpp>
#include <ek/util/Res.hpp>
#include <ek/util/Type.hpp>
#include <ecxx/ecxx_fwd.hpp>
#include <ek/temp_res_man.h>

namespace ek {

struct Camera3D {
    // clip plane near-far
    float zNear = 10.0f;
    float zFar = 1000.0f;

    // orthogonal mode
    bool orthogonal = false;
    float orthogonalSize = 30.0f;

    // field of view in radians
    float fov = Math::to_radians(45.0f);

    // camera up vector
    Vec3f up{0.0f, 0.0f, 1.0f};

    bool clearColorEnabled = true;
    bool clearDepthEnabled = true;

    Vec4f clearColor{0.5f, 0.5f, 0.5f, 1.0f};
    float clearDepth = 1.0f;

    ek_texture_reg_id cubeMap;
};

ECX_TYPE(29, Camera3D);

}

