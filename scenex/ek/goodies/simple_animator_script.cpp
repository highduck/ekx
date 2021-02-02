#include "simple_animator_script.h"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/math/hvs_color.hpp>

namespace ek {

void simple_animator_script::update(float dt) {
    rotation += dt * rotation_speed;
    hue += dt * hue_speed;

    auto& transform = get<Transform2D>();
    transform.setRotation(rotation);
    transform.color.scale = lerp(
            hvs_color_t::get_hue_color(math::reduce(hue, 1.0f, 0.0f)),
            base_color,
            1.0f - hue_mixup_factor
    );
}

}