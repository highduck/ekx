#pragma once

#include <ecxx/impl/entity.hpp>

namespace ek {

struct Transform2D;

class RenderSystem2D {
public:
    static int currentLayerMask;

    static void draw(ecs::entity e, const Transform2D* transform);

    static void drawStack(ecs::entity e);
};

}

