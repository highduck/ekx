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

//int simParticlesCount = 500;
//int simParticlesCount = 1000;
int simParticlesCount = 5000;
//int simParticlesCount = 20000;
//int simParticlesCount = 20000;

bool showPiko = false;
//bool showPiko = true;

void add_objects(ecs::entity game, int N) {
    for (int i = 0; i < N; ++i) {
        auto q = createNode2D();
        q.get<Node>().setTouchable(false);
        const float2 pos{ek::random(0.0f, WIDTH),
                         ek::random(0.0f, HEIGHT)};
        q.get<Transform2D>().setPosition(pos);

        q.assign<motion_t>(ek::random(-50.0f, 50.0f),
                           ek::random(-50.0f, 50.0f));

        auto* quad = new Quad2D();
        quad->rect = {-0.25f, -0.25f, 0.5f, 0.5f};
        quad->setColor(0xFFFFFF_rgb);
        q.assign<Display2D>(quad);

        append(game, q);

        auto& trail_data = q.assign<Trail2D>();
        trail_data.drain_speed = 1.0f;

        auto trailRenderer = createNode2D();
        auto* renderer = new TrailRenderer2D(q);
        renderer->width = 0.25f;
        renderer->minWidth = 0.0f;
        trailRenderer.assign<Display2D>(renderer);
        append(game, trailRenderer);
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
        e.get<Transform2D>().setPosition(20.0f, 20.0f);
        // TODO: fix scissors stats
        e.assign<Bounds2D>(rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;
        append(sampleContainer, e);

        e = createNode2D("dna");
        assignScript<dna>(e);
//    ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
        e.get<Transform2D>().setPosition(20.0f, 20.0f + 128.0f + 10.0f);
        // TODO: fix scissors stats
        e.assign<Bounds2D>(rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;
        append(sampleContainer, e);

        e = createNode2D("diamonds");
        assignScript<diamonds>(e);
        e.get<Transform2D>().setPosition(20.0f + 128.0f + 10.0f, 20.0f);
        e.assign<Bounds2D>(rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;;
        append(sampleContainer, e);
    }

    // SIMULATION SAMPLE

    // SIM
    add_objects(sampleContainer, simParticlesCount);

    auto mouse_entity = createNode2D("Mouse");
    assignScript<mouse_follow_script>(mouse_entity);

    auto attractor_entity = createNode2D("Follower");
    attractor_entity.assign<attractor_t>();
    auto& attr = assignScript<target_follow_script>(attractor_entity);
    attr.target_entity = ecs::EntityRef{mouse_entity};
    attr.k = 0.1f;
    attractor_entity.get<attractor_t>().radius = 100.0f;
    attractor_entity.get<attractor_t>().force = 5'000.0f;

    append(sampleContainer, mouse_entity);
    append(sampleContainer, attractor_entity);

    attractor_entity = createNode2D("Centroid");
    attractor_entity.assign<attractor_t>();
    attractor_entity.get<attractor_t>().radius = 200.0f;
    attractor_entity.get<attractor_t>().force = -1000.0f;
    attractor_entity.get<Transform2D>().setPosition(300.0f, 400.0f);
    append(sampleContainer, attractor_entity);

    return sampleContainer;
}

void update() {
    auto dt = TimeLayer::Game->dt;
    update_motion_system(dt);
}

}