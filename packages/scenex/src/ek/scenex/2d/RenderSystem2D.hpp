#pragma once

#include <ecxx/ecxx_fwd.hpp>

namespace ek {

struct WorldTransform2D;

class RenderSystem2D {
public:
    static int currentLayerMask;

    static void draw(entity_t e, const WorldTransform2D* transform);

    static void drawStack(entity_t e);
};

}

