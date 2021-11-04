#pragma once

#include <ecxx/ecxx_fwd.hpp>

namespace ek {

struct WorldTransform2D;

class RenderSystem2D {
public:
    static int currentLayerMask;

    static void draw(const ecs::World& w, ecs::EntityIndex e, const WorldTransform2D* transform);

    static void drawStack(const ecs::World& w, ecs::EntityIndex e);
};

}

