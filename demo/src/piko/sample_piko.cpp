#include <sim/driver_node_script.h>
#include <ek/scenex/app/basic_application.hpp>
#include <ek/scenex/components/node.hpp>
#include "sample_piko.hpp"

namespace ek {

SamplePiko::SamplePiko() {
    auto& app = resolve<basic_application>();
    container = piko::create();
    append(app.game, container);
}

SamplePiko::~SamplePiko() {
    destroy_node(container);
}

void SamplePiko::draw() {
    SampleBase::draw();
}

void SamplePiko::update(float dt) {
    SampleBase::update(dt);
    auto& app = resolve<basic_application>();
    piko::update(app.game);
}
}