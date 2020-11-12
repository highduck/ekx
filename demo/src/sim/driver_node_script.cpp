#include "driver_node_script.h"
#include "motion_system.h"

#include <ek/util/locator.hpp>
#include <ek/scenex/interactive_manager.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/components/display_2d.hpp>
#include <ek/scenex/goodies/helpers/trail_script.hpp>
#include <ek/scenex/goodies/helpers/mouse_follow_script.hpp>
#include <ek/scenex/goodies/helpers/target_follow_script.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <sim/components/attractor.h>
#include <ek/scenex/systems/game_time.hpp>
#include <ek/math/rand.hpp>
#include <sim/components/position.h>
#include <sim/components/motion.h>
#include <piko/examples.h>

namespace ek::piko {

void add_objects(ecs::entity game, int N) {
    for (int i = 0; i < N; ++i) {
        auto q = ecs::create<node_t, transform_2d>();
        float2 pos = {ek::random(0.0f, 720.0f),
                      ek::random(0.0f, 960.0f)};
        ecs::assign<position_t>(q, pos.x, pos.y);
        ecs::get<transform_2d>(q).matrix.position(pos);
        ecs::assign<motion_t>(q,
                              ek::random(-50.0f, 50.0f),
                              ek::random(-50.0f, 50.0f)
        );
        append(game, q);

        auto trail = ecs::create<node_t, transform_2d>();
        auto& trail_data = assign_script<trail_script>(trail);
        trail_data.width = 1.0f;
        trail_data.min_width = 1.0f;
        trail_data.drain_speed = 0.5f;
        append(game, trail);
        trail_data.track(q);
    }
}

ecs::entity create() {
    ecs::entity sampleContainer = create_node_2d("piko");

    // PIKO

    ecs::entity e;
    e = create_node_2d("book");
    assign_script<book>(e);
    ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
    ecs::get<transform_2d>(e).matrix.position(10.0f, 10.0f);
    // TODO: fix scissors stats
//    ecs::assign<scissors_2d>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f});
    append(sampleContainer, e);

    e = create_node_2d("dna");
    assign_script<dna>(e);
    ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
    ecs::get<transform_2d>(e).matrix.position(10.0f, 10.0f + 2.0f * 128.0f + 10.0f);
    // TODO: fix scissors stats
//    ecs::assign<scissors_2d>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f});
    append(sampleContainer, e);

    e = create_node_2d("diamonds");
    assign_script<diamonds>(e);
    ecs::get<transform_2d>(e).matrix.position(100.0f + 2.0f * 128.0f, 10.0f);

//    ecs::assign<scissors_2d>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f});
    append(sampleContainer, e);


    // SIMULATION SAMPLE

    // SIM
    add_objects(sampleContainer, 500);

    auto mouse_entity = ecs::create<transform_2d, node_t>();
    set_name(mouse_entity, "Mouse");
    assign_script<mouse_follow_script>(mouse_entity);

    auto attractor_entity = ecs::create<attractor_t, transform_2d, node_t>();
    set_name(attractor_entity, "Follower");
    auto& attr = assign_script<target_follow_script>(attractor_entity);
    attr.target_entity = mouse_entity;
    attr.k = 0.1f;
    ecs::get<attractor_t>(attractor_entity).radius = 100.0f;
    ecs::get<attractor_t>(attractor_entity).force = 5'000.0f;

    append(sampleContainer, mouse_entity);
    append(sampleContainer, attractor_entity);

    attractor_entity = ecs::create<attractor_t, transform_2d, node_t>();
    set_name(attractor_entity, "Centroid");
    ecs::get<attractor_t>(attractor_entity).radius = 200.0f;
    ecs::get<attractor_t>(attractor_entity).force = -1000.0f;
    ecs::get<transform_2d>(attractor_entity).matrix.position(300.0f, 400.0f);
    append(sampleContainer, attractor_entity);

    return sampleContainer;
}

void remove_objects(ecs::entity game, int N) {
    int i = 0;
    for (auto q : ecs::view<node_t, transform_2d, script_holder>()) {
        remove_from_parent(q);
        ecs::destroy(q);
        if (++i == N) {
            break;
        }
    }
}

void update(ecs::entity game) {
    auto dt = get_delta_time(game);
    update_motion_system(dt);
//
//    ImGui::Begin("test");
//    if (ImGui::Button("Add 1000")) {
//        add_objects(game, 1'000);
//    }
//    ImGui::SameLine();
//    if (ImGui::Button("Remove 1000")) {
//        remove_objects(game, 1'000);
//    }
//    ImGui::End();
}


}