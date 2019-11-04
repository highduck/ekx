#pragma once

#include <scenex/config/ecs.h>

namespace scenex {

class script {
public:

    script() = default;

    virtual ~script();

    void link_to_entity(ecs::entity e) {
        entity_ = e;
    }

    virtual void start() {}

    virtual void update(float dt) {}

    virtual void draw() {}

    virtual void gui_gizmo() {}

    virtual void gui_inspector() {}

    template<typename Component>
    inline Component& get() {
        return ecs::get<Component>(entity_);
    }

    template<typename Component>
    inline Component& get_or_create() {
        return ecs::get_or_create<Component>(entity_);
    }

    [[nodiscard]] ecs::entity find_child(const std::string& name) const;

protected:
    ecs::entity entity_;
};

struct script_holder {
    std::vector<std::unique_ptr<script>> list;
};

}