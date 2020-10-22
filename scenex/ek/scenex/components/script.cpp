#include "script.hpp"

#include <ek/scenex/utility/scene_management.hpp>

namespace ek {

script_cpp_base::~script_cpp_base() = default;

ecs::entity script_cpp_base::find_child(const std::string& name) const {
    return find(entity_, name);
}

}