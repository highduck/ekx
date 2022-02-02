#pragma once

#include <ek/math.h>

struct simple_animator_comp {
    float rotation = 0.0f;
    float rotation_speed = 1.0f;
    float hue = 0.0f;
    float hue_speed = 0.0f;
    float hue_mixup_factor = 0.0f;
    color_t base_color = COLOR_WHITE;
};

void update_simple_animator_comps(float dt);
