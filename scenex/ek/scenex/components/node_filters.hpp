#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <vector>

namespace ek {

struct node_filters_t {
    bool enabled = true;
    bool processing = false;
    std::vector<filter_data> filters;
};

bool process_node_filters(ecs::entity e);

}


