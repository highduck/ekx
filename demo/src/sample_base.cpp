#include "sample_base.hpp"
#include <ek/scenex/utility/scene_management.hpp>

namespace ek {

SampleBase::SampleBase() {
    container = createNode2D("sample content");
    append(SampleBase::samplesContainer, container);

}

SampleBase::~SampleBase() {
    destroyNode(container);
}

void SampleBase::draw() {}

void SampleBase::update(float dt) {
    (void)dt;
}

ecs::entity SampleBase::samplesContainer{};

}