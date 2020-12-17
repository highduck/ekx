#include "driver_node_script.h"
#include "motion_system.h"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/helpers/mouse_follow_script.hpp>
#include <ek/goodies/helpers/target_follow_script.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <sim/components/attractor.h>
#include <ek/timers.hpp>
#include <ek/math/rand.hpp>
#include <sim/components/motion.h>
#include <piko/examples.h>

namespace ek::piko {

inline const float WIDTH = 360;
inline const float HEIGHT = 480;
bool showPiko = true;
int simParticlesCount = 500;

void add_objects(ecs::entity game, int N) {
    for (int i = 0; i < N; ++i) {
        auto q = ecs::create<Node, Transform2D>();
        q.get<Node>().setTouchable(false);
        q.get<Transform2D>().position = {ek::random(0.0f, WIDTH),
                                         ek::random(0.0f, HEIGHT)};
        q.assign<motion_t>(ek::random(-50.0f, 50.0f),
                           ek::random(-50.0f, 50.0f));
        append(game, q);

        auto trail = ecs::create<Node, Transform2D>();
        trail.get<Node>().setTouchable(false);
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

ecs::entity create() {
    ecs::entity sampleContainer = createNode2D("piko");

    // PIKO
    if (showPiko) {
        ecs::entity e;
        e = createNode2D("book");
        assignScript<book>(e);
        //ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
        ecs::get<Transform2D>(e).position = {20.0f, 20.0f};
        // TODO: fix scissors stats
        ecs::assign<Bounds2D>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;
        append(sampleContainer, e);

        e = createNode2D("dna");
        assignScript<dna>(e);
//    ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
        ecs::get<Transform2D>(e).position = {20.0f, 20.0f + 128.0f + 10.0f};
        // TODO: fix scissors stats
        ecs::assign<Bounds2D>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;
        append(sampleContainer, e);

        e = createNode2D("diamonds");
        assignScript<diamonds>(e);
        ecs::get<Transform2D>(e).position = {20.0f + 128.0f + 10.0f, 20.0f};
        ecs::assign<Bounds2D>(e, rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;;
        append(sampleContainer, e);
    }

    // SIMULATION SAMPLE

    // SIM
    add_objects(sampleContainer, simParticlesCount);

    auto mouse_entity = ecs::create<Transform2D, Node>();
    setName(mouse_entity, "Mouse");
    assignScript<mouse_follow_script>(mouse_entity);

    auto attractor_entity = ecs::create<attractor_t, Transform2D, Node>();
    setName(attractor_entity, "Follower");
    auto& attr = assignScript<target_follow_script>(attractor_entity);
    attr.target_entity = mouse_entity;
    attr.k = 0.1f;
    ecs::get<attractor_t>(attractor_entity).radius = 100.0f;
    ecs::get<attractor_t>(attractor_entity).force = 5'000.0f;

    append(sampleContainer, mouse_entity);
    append(sampleContainer, attractor_entity);

    attractor_entity = ecs::create<attractor_t, Transform2D, Node>();
    setName(attractor_entity, "Centroid");
    ecs::get<attractor_t>(attractor_entity).radius = 200.0f;
    ecs::get<attractor_t>(attractor_entity).force = -1000.0f;
    ecs::get<Transform2D>(attractor_entity).position = {300.0f, 400.0f};
    append(sampleContainer, attractor_entity);

    return sampleContainer;
}

void remove_objects(ecs::entity game, int N) {
    int i = 0;
    for (auto q : ecs::view<Node, Transform2D, ScriptHolder>()) {
        removeFromParent(q);
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