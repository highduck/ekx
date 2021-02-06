#include "sample_piko.hpp"

#include "examples.h"
#include <sim/driver_node_script.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/scenex/utility/scene_management.hpp>

namespace ek {

namespace piko {

void create(ecs::entity container) {

    ecs::entity e;
    e = createNode2D("book");
    assignScript<book>(e);
    //ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
    e.get<Transform2D>().setPosition(20.0f, 20.0f);
    // TODO: fix scissors stats
    e.assign<Bounds2D>(rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;
    append(container, e);

    e = createNode2D("dna");
    assignScript<dna>(e);
//    ecs::get<transform_2d>(e).scale = {2.0f, 2.0f};
    e.get<Transform2D>().setPosition(20.0f, 20.0f + 128.0f + 10.0f);
    // TODO: fix scissors stats
    e.assign<Bounds2D>(rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;
    append(container, e);

    e = createNode2D("diamonds");
    assignScript<diamonds>(e);
    e.get<Transform2D>().setPosition(20.0f + 128.0f + 10.0f, 20.0f);
    e.assign<Bounds2D>(rect_f{0.0f, 0.0f, 128.0f, 128.0f}).scissors = true;;
    append(container, e);
}

}

SamplePiko::SamplePiko() {
    piko::create(container);
    title = "PIKO PIKO";
}

void SamplePiko::draw() {
    SampleBase::draw();
}

void SamplePiko::update(float dt) {
    SampleBase::update(dt);
}

}