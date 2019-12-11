#include "script.hpp"

#include <ek/scenex/utility/scene_management.hpp>

namespace ek {

script::~script() = default;

ecs::entity script::find_child(const std::string& name) const {
    return find(entity_, name);
}

}