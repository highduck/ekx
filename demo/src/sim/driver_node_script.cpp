#include "driver_node_script.h"
#include "motion_system.h"

#include <ek/util/locator.hpp>
#include <ek/scenex/interactive_manager.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/helpers/mouse_follow_script.hpp>
#include <ek/goodies/helpers/target_follow_script.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <sim/components/attractor.h>
#include <ek/scenex/game_time.hpp>
#include <ek/math/rand.hpp>
#include <sim/components/position.h>
#include <sim/components/motion.h>
#include <piko/examples.h>

namespace ek::piko {

void add_objects(ecs::entity game, int N) {
    for (int i = 0; i < N; ++i) {
        auto q = ecs::create<node_t, Transform2D>();
        q.get<node_t>().setTouchable(false);
        float2 pos = {ek::random(0.0f, 720.0f),
                      ek::random(0.0f, 960.0f)};
        ecs::assign<position_t>(q, pos.x, pos.y);
        ecs::get<Transform2D>(q).position = pos;
        ecs::assign<motion_t>(q,
                              ek::random(-50.0f, 50.0f),
                              ek::random(-50.0f, 50.0f)
        );
        append(game, q);

        auto trail = ecs::create<node_t, Transform2D>();
        trail.get<node_t>().setTouchable(false);
        auto& trail_data = trail.assign<Trail2D>(trail);
        trail_data.drain_speed = 0.5f;

        auto* renderer = new TrailRenderer2D(trail);
        renderer->width = 1.0f;
        renderer->minWidth = 1.0f;
        trail.assign<Display2D>(renderer);

        append(game, trail);
        trail_data.track(q);
    }
}

bool showPiko = false;

ecs::entity create() {
    ecs::entity sampleContainer = create_node_2d("piko");

    // PIKO
    if (showPiko) {
        ecs::entity e;
        e = create_node_2d("book");
        assign_script<book>(e);
        //ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
        ecs::get<Transform2D>(e).position = {10.0f, 10.0f};
        // TODO: fix scissors stats
        ecs::assign<scissors_2d>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f});
        append(sampleContainer, e);

        e = create_node_2d("dna");
        assign_script<dna>(e);
//    ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
        ecs::get<Transform2D>(e).position = {10.0f, 10.0f + 128.0f + 10.0f};
        // TODO: fix scissors stats
        ecs::assign<scissors_2d>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f});
        append(sampleContainer, e);

        e = create_node_2d("diamonds");
        assign_script<diamonds>(e);
        ecs::get<Transform2D>(e).position = {10.0f + 128.0f + 10.0f, 20.0f};
        ecs::assign<scissors_2d>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f});
        append(sampleContainer, e);
    }

    // SIMULATION SAMPLE

    // SIM
    add_objects(sampleContainer, 5000);

    auto mouse_entity = ecs::create<Transform2D, node_t>();
    set_name(mouse_entity, "Mouse");
    assign_script<mouse_follow_script>(mouse_entity);

    auto attractor_entity = ecs::create<attractor_t, Transform2D, node_t>();
    set_name(attractor_entity, "Follower");
    auto& attr = assign_script<target_follow_script>(attractor_entity);
    attr.target_entity = mouse_entity;
    attr.k = 0.1f;
    ecs::get<attractor_t>(attractor_entity).radius = 100.0f;
    ecs::get<attractor_t>(attractor_entity).force = 5'000.0f;

    append(sampleContainer, mouse_entity);
    append(sampleContainer, attractor_entity);

    attractor_entity = ecs::create<attractor_t, Transform2D, node_t>();
    set_name(attractor_entity, "Centroid");
    ecs::get<attractor_t>(attractor_entity).radius = 200.0f;
    ecs::get<attractor_t>(attractor_entity).force = -1000.0f;
    ecs::get<Transform2D>(attractor_entity).position = {300.0f, 400.0f};
    append(sampleContainer, attractor_entity);

    return sampleContainer;
}

void remove_objects(ecs::entity game, int N) {
    int i = 0;
    for (auto q : ecs::view<node_t, Transform2D, script_holder>()) {
        remove_from_parent(q);
        ecs::destroy(q);
        if (++i == N) {
            break;
        }
    }
}

void update(ecs::entity game) {
    auto dt = TimeLayer::Game->dt;
    update_motion_system(dt);
}


}