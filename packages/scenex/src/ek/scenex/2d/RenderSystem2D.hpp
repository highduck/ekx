#pragma once

#include <ecx/ecx_fwd.hpp>

namespace ek {

struct WorldTransform2D;

class RenderSystem2D {
public:
    static void draw(entity_t e, const WorldTransform2D* transform);

    static void drawStack(entity_t e);
};

}

