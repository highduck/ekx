#pragma once

#include <ecxx/ecxx.hpp>

#include <ek/app/app.hpp>
#include <string>

namespace ek {

namespace interactive_event {

static constexpr auto back_button = "back_button";
static constexpr auto system_pause = "system_pause";

}

class interactive_manager {
public:
    // screen-space pointer position
    float2 pointer_global_space = float2::zero;
    float2 pointer = float2::zero;
    bool pointer_down = false;

    interactive_manager() = default;

    ~interactive_manager() {
        assert(false);
    }

    void set_entity(ecs::entity e) {
        entity_ = e;
    }

    void update();

    app::mouse_cursor search_interactive_targets(ecs::entity node, std::vector<ecs::entity>& out_entities);

    void send_back_button();

    void handle_system_pause();

    void handle_mouse_event(const app::event_t& ev);

    void handle_touch_event(const app::event_t& ev);

    [[nodiscard]]
    ecs::entity entity() const {
        return entity_;
    }

    void drag(ecs::entity entity) {
        drag_entity_ = entity;
    }

    [[nodiscard]] ecs::entity getHitTarget() const {
        return hitTarget_.valid() ? hitTarget_ : nullptr;
    }

private:
    ecs::entity hitTarget_;
    ecs::entity entity_;

    std::vector<ecs::entity> last_targets_;
    std::vector<ecs::entity> targets_;

    bool mouse_active_ = false;
    uint64_t primary_touch_id_ = 0ull;
    float2 primary_touch_;
    float2 primary_mouse_;

    ecs::entity drag_entity_;
};

}


