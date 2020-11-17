#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <vector>

namespace ek {

struct UglyFilter2D {
    bool enabled = true;
    bool processing = false;
    std::vector<filter_data> filters;

    static bool process(ecs::entity e);
};

}


