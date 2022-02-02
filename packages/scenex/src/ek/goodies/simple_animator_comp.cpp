#include "simple_animator_comp.h"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/math.h>
#include <ecxx/ecxx.hpp>

void update_simple_animator_comps(float dt) {
    using ek::Transform2D;
    for(auto e : ecs::view<simple_animator_comp>()) {
        auto& d = e.get<simple_animator_comp>();
        d.rotation += dt * d.rotation_speed;
        d.hue += dt * d.hue_speed;

        auto& transform = e.get<Transform2D>();
        transform.setRotation(d.rotation);
        transform.color.scale = lerp_color(
                color_hue(reduce(d.hue, 1.0f, 0.0f)),
                d.base_color,
                1.0f - d.hue_mixup_factor
        );
    }
}
