#pragma once

#include <ek/scenex/components/script.hpp>
#include <ek/math/packed_color.hpp>

namespace ek {

EK_DECL_SCRIPT_CPP(simple_animator_script) {
public:
    void update(float dt) override;

    float rotation = 0.0f;
    float rotation_speed = 1.0f;
    float hue = 0.0f;
    float hue_speed = 0.0f;
    float hue_mixup_factor = 0.0f;
    argb32_t base_color = 0xFFFFFFFF_argb;
};

}


