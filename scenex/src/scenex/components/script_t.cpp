#include "script_t.h"

#include <scenex/utility/scene_management.h>

namespace scenex {

script::~script() = default;

ecs::entity script::find_child(const std::string& name) const {
    return ::scenex::find(entity_, name);
}

}