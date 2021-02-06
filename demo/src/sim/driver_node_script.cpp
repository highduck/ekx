#include "driver_node_script.h"
#include "Motion.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/goodies/helpers/mouse_follow_script.hpp>
#include <ek/goodies/helpers/target_follow_script.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/timers.hpp>
#include <ek/math/rand.hpp>

namespace ek::sim {

inline const float WIDTH = 360;
inline const float HEIGHT = 480;

//int simParticlesCount = 500;
//int simParticlesCount = 1000;
//int simParticlesCount = 5000;
int simParticlesCount = 20000;
//int simParticlesCount = 20000;

void add_objects(ecs::entity game, int N) {
    for (int i = 0; i < N; ++i) {
        auto q = createNode2D();
        q.get<Node>().setTouchable(false);
        const float2 pos{random(0.0f, WIDTH),
                         random(0.0f, HEIGHT)};
        q.get<Transform2D>().setPosition(pos);

        auto& mot = q.assign<motion_t>();
        mot.velocity = {random(-50.0f, 50.0f),random(-50.0f, 50.0f)};

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

void create(ecs::entity container) {
    auto particles = createNode2D("particles");
    add_objects(particles, simParticlesCount);

    append(container, particles);

    auto mouse_entity = createNode2D("Mouse");
    assignScript<mouse_follow_script>(mouse_entity);

    auto attractor_entity = createNode2D("Follower");
    attractor_entity.assign<attractor_t>();
    auto& attr = assignScript<target_follow_script>(attractor_entity);
    attr.target_entity = ecs::EntityRef{mouse_entity};
    attr.k = 0.1f;
    attractor_entity.get<attractor_t>().radius = 100.0f;
    attractor_entity.get<attractor_t>().force = 5'000.0f;

    append(container, mouse_entity);
    append(container, attractor_entity);

    attractor_entity = createNode2D("Centroid");
    attractor_entity.assign<attractor_t>();
    attractor_entity.get<attractor_t>().radius = 200.0f;
    attractor_entity.get<attractor_t>().force = -1000.0f;
    attractor_entity.get<Transform2D>().setPosition(300.0f, 400.0f);
    append(container, attractor_entity);
}

void update() {
    auto dt = TimeLayer::Game->dt;
    update_motion_system(dt);
}

}