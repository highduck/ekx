#include "target_follow_script.h"

#include <scenex/config/ecs.h>
#include <scenex/utility/scene_management.h>
#include <ek/locator.hpp>
#include <draw2d/drawer.hpp>

using namespace ek;

namespace scenex {

void target_follow_script::update(float dt) {
    script::update(dt);

    ++counter;
    time_accum += dt;
    if (counter >= n) {

        auto& matrix = get<transform_2d>().matrix;

        if (ecs::valid(target_entity)) {
            target = local_to_global(target_entity, float2::zero);
            target = global_to_parent(entity_, target);
        } else {
            target_entity = ecs::null;
        }

        auto current = matrix.position() - offset;
        if (integration == integration_mode::Exp) {
            const float c = ::logf(1.0f - k) * fixed_frame_rate;
            current = current + (target - current) * (1.0f - exp(c * time_accum));
        } else if (integration == integration_mode::Steps) {
            const float timeStep = 1.0f / fixed_frame_rate;
            while (time_accum >= timeStep) {
                current = current + (target - current) * k;
                time_accum -= timeStep;
            }
            current = current + (target - current) * k * (time_accum * fixed_frame_rate);
        } else if (integration == integration_mode::None) {
            current = current + (target - current) * k * (time_accum * fixed_frame_rate);
        }

        matrix.position(offset + current);
        counter = 0;
        time_accum = 0.0f;
    }
}

void target_follow_script::gui_gizmo() {
    resolve<drawer_t>().fill_circle(circle_f{100.0f}, 0x00FFFFFF_argb, 0x77FFFFFF_argb, 10);
}

void target_follow_script::gui_inspector() {
//#if defined(DEV_MODE)
//    if (ImGui::CollapsingHeader("Target Follow")) {
//        ImGui::InputFloat2("Offset", (float*)&offset);
//        ImGui::InputFloat2("Target", (float*)&target);
//        ImGui::Text("Target Entity -> %u", target_entity.index());
//
//        ImGui::SliderFloat("k", &k, 0.0f, 1.0f);
//        ImGui::SliderFloat("fixed_frame_rate", &fixed_frame_rate, 1.0f, 60.0f);
//    }
//#endif
}

}