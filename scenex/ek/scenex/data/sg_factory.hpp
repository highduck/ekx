#pragma once

#include "sg_data.hpp"
#include <ek/scenex/components/Node.hpp>

namespace ek {

sg_file* sg_load(const std::vector<uint8_t>& buffer);

ecs::entity sg_create(const std::string& library, const std::string& name);

rect_f sg_get_bounds(const std::string& library, const std::string& name);

}


