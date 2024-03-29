#include "Script.hpp"
#include "Node.hpp"

#include <ek/timers.hpp>

namespace ek {

ScriptBase::~ScriptBase() = default;

ecs::EntityApi ScriptBase::find_child(const std::string& name) const {
    return find(entity_, name);
}

void updateScripts() {
    float dt = TimeLayer::Root->dt;
    ecs::view_backward<ScriptHolder>()
            .each([dt](ScriptHolder& scripts) {
                for (auto& script : scripts.list) {
                    if (script) {
                        script->update(dt);
                    }
                }
            });

    for(auto e : ecs::view<Updater>()) {
        auto& updater = e.get<Updater>();
        updater.onUpdate(e, updater.timeLayer);
    }
}

void ScriptHolder::link(ecs::EntityApi owner_) {
    if (!owner) {
        owner = owner_;
        auto& display = owner_.get_or_create<Display2D>();
        display.drawable = std::make_unique<ScriptDrawable2D>(owner_, std::move(display.drawable));
    }
}
}