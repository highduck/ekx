#pragma once

#include <ecxx/ecxx_fwd.hpp>

#include <ek/app/app.hpp>
#include <string>
#include <ek/scenex/base/Interactive.hpp>

namespace ek {

namespace interactive_event {

static constexpr auto back_button = "back_button";
static constexpr auto system_pause = "system_pause";

}

class InteractionSystem {
public:
    // screen-space pointer position
    float2 pointerScreenPosition_ = float2::zero;
    bool pointerDown_ = false;

    explicit InteractionSystem(ecs::EntityApi root);

    ~InteractionSystem() = delete;

    void process();

    app::mouse_cursor searchInteractiveTargets(float2 pos, ecs::EntityApi node, std::vector<ecs::EntityApi>& out_entities);

    void sendBackButton();

    void handle_system_pause();

    void handle_mouse_event(const app::event_t& ev);

    void handle_touch_event(const app::event_t& ev);

    void drag(ecs::EntityApi entity);

    [[nodiscard]]
    ecs::EntityApi getHitTarget() const;

private:
    void fireInteraction(InteractionEvent event, bool prev = true, bool onlyIfChanged = false);

    std::vector<ecs::EntityApi>& getPrevTargets() {
        return targetLists[targetListIndex];
    }

    std::vector<ecs::EntityApi>& getCurrentTargets() {
        return targetLists[(targetListIndex + 1) & 1];
    }

    void swapTargetLists() {
        targetListIndex = (++targetListIndex) & 1;
    }

private:
    ecs::EntityRef hitTarget_;
    ecs::EntityApi root_;

    std::vector<ecs::EntityApi> targetLists[2]{};
    int targetListIndex = 0;

    bool mouseActive_ = false;
    uint64_t touchID_ = 0ull;
    float2 touchPosition0_;
    float2 mousePosition0_;

    ecs::EntityRef dragEntity_;
};

}


