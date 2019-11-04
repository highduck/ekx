#pragma once

#include <scenex/config/ecs.h>
#include <scenex/data/sg_data.h>
#include <vector>

namespace scenex {

struct node_filters_t {
    bool enabled = true;
    bool processing = false;
    std::vector<filter_data> filters;
};

bool process_node_filters(ecs::entity e);

}


