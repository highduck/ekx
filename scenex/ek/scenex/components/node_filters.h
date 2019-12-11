#pragma once

#include <ek/scenex/config/ecs.h>
#include <ek/scenex/data/sg_data.h>
#include <vector>

namespace ek {

struct node_filters_t {
    bool enabled = true;
    bool processing = false;
    std::vector<filter_data> filters;
};

bool process_node_filters(ecs::entity e);

}


