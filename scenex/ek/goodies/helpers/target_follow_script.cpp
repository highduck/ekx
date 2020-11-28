#include "target_follow_script.hpp"

#include <ecxx/ecxx.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/draw2d/drawer.hpp>

namespace ek {

void target_follow_script::update(float dt) {
    ++counter;
    time_accum += dt;
    if (counter >= n) {

        auto& tr = get<Transform2D>();

        if (ecs::valid(target_entity)) {
            auto parent = entity_.get<Node>().parent;
            if(parent) {
                target = Transform2D::localToLocal(target_entity, parent, float2::zero);
            }
        } else {
            target_entity = nullptr;
        }

        auto current = tr.position - offset;
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

        tr.position = offset + current;
        counter = 0;
        time_accum = 0.0f;
    }
}

void target_follow_script::gui_gizmo() {
    draw2d::fill_circle(circle_f{100.0f}, 0x00FFFFFF_argb, 0x77FFFFFF_argb, 10);
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