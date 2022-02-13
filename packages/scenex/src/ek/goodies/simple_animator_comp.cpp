#include "simple_animator_comp.h"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/math.h>
#include <ecx/ecx.hpp>

void update_simple_animator_comps(float dt) {
    using ek::Transform2D;
    for(auto e : ecs::view<simple_animator_comp>()) {
        auto& d = ecs::get<simple_animator_comp>(e);
        d.rotation += dt * d.rotation_speed;
        d.hue += dt * d.hue_speed;

        auto& transform = ecs::get<Transform2D>(e);
        transform.set_rotation(d.rotation);
        transform.color.scale = lerp_color(
                color_hue(reduce(d.hue, 1.0f, 0.0f)),
                d.base_color,
                1.0f - d.hue_mixup_factor
        );
    }
}
