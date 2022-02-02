#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/math.h>

struct target_follow_comp {
    enum class integration_mode {
        Exp = 0,
        Steps = 1,
        None = 2,
    };

    vec2_t offset = {};
    vec2_t target = {};
    ecs::EntityRef target_entity{};

    float k = 0.1f;
    float fixed_frame_rate = 60.0f;

    int n = 1;
    int counter = 0;
    float time_accum = 0.0f;

    int frame = 0;
    int reset_in = 100;

    integration_mode integration = integration_mode::Exp;
};

struct mouse_follow_comp {};

ECX_COMP_TYPE_SPARSE(mouse_follow_comp)

void update_target_follow_comps(float dt);
void update_mouse_follow_comps(void);

