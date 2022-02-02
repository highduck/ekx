#include "follow.h"

#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/InteractionSystem.hpp>

void update_target_follow_comps(float dt) {
    using ek::Transform2D;
    using ek::Node;
    for(auto e : ecs::view<target_follow_comp>()) {
        auto& data = e.get<target_follow_comp>();
        ++data.counter;
        data.time_accum += dt;
        if (data.counter >= data.n) {

            auto& tr = e.get<Transform2D>();

            if (data.target_entity.valid()) {
                auto parent = e.get<Node>().parent;
                if (parent) {
                    data.target = Transform2D::localToLocal(data.target_entity.ent(), parent, {});
                }
            } else {
                data.target_entity = nullptr;
            }

            auto current = vec2_t(tr.getPosition()) - data.offset;
            if (data.integration == target_follow_comp::integration_mode::Exp) {
                const float c = logf(1.0f - data.k) * data.fixed_frame_rate;
                current = current + (data.target - current) * (1.0f - exp(c * data.time_accum));
            } else if (data.integration == target_follow_comp::integration_mode::Steps) {
                const float timeStep = 1.0f / data.fixed_frame_rate;
                while (data.time_accum >= timeStep) {
                    current = current + (data.target - current) * data.k;
                    data.time_accum -= timeStep;
                }
                current = current + (data.target - current) * data.k * (data.time_accum * data.fixed_frame_rate);
            } else if (data.integration == target_follow_comp::integration_mode::None) {
                current = current + (data.target - current) * data.k * (data.time_accum * data.fixed_frame_rate);
            }

            tr.setPosition(data.offset + current);
            data.counter = 0;
            data.time_accum = 0.0f;
        }
    }
}

void update_mouse_follow_comps(void) {
    using namespace ek;
    for(auto e : ecs::view<mouse_follow_comp>()) {
        auto parent = e.get<Node>().parent;
        if (parent) {
            auto* im = &g_interaction_system;
            const auto& camera = Camera2D::Main.get<Camera2D>();
            const auto cameraPointer = vec2_transform(im->pointerScreenPosition_, camera.screenToWorldMatrix);
            const auto pos = Transform2D::globalToLocal(parent, cameraPointer);
            e.get<Transform2D>().setPosition(pos);
        }
    }
}
