#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <vector>

namespace ek {

struct UglyFilter2D {
    bool enabled = true;
    bool processing = false;
    std::vector<SGFilter> filters;

    bool pass(const ecs::World& w, ecs::EntityIndex e);
};

}


