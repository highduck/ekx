#include "script.hpp"
#include "Node.hpp"

#include <ek/scenex/game_time.hpp>

namespace ek {

script_cpp_base::~script_cpp_base() = default;

ecs::entity script_cpp_base::find_child(const std::string& name) const {
    return find(entity_, name);
}

void updateScripts() {
    float dt = TimeLayer::Root->dt;
    ecs::rview<script_holder>()
            .each([dt](script_holder& scripts) {
                for (auto& script : scripts.list) {
                    if (script) {
                        script->update(dt);
                    }
                }
            });
}

}