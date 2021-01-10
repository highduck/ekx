#pragma once

#include <ecxx/impl/entity.hpp>

namespace ek {

struct WorldTransform2D;

class RenderSystem2D {
public:
    static int currentLayerMask;

    static void draw(ecs::entity e, const WorldTransform2D* transform);

    static void drawStack(ecs::entity e);
};

}

