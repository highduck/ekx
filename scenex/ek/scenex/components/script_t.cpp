#include "script_t.h"

#include <ek/scenex/utility/scene_management.h>

namespace ek {

script::~script() = default;

ecs::entity script::find_child(const std::string& name) const {
    return find(entity_, name);
}

}