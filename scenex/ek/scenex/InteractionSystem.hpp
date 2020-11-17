#pragma once

#include <ecxx/ecxx.hpp>

#include <ek/app/app.hpp>
#include <string>
#include <ek/scenex/components/interactive.hpp>

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

    explicit InteractionSystem(ecs::entity root);

    ~InteractionSystem() = delete;

    void process();

    app::mouse_cursor searchInteractiveTargets(float2 pos, ecs::entity node, std::vector<ecs::entity>& out_entities);

    void sendBackButton();

    void handle_system_pause();

    void handle_mouse_event(const app::event_t& ev);

    void handle_touch_event(const app::event_t& ev);

    void drag(ecs::entity entity) {
        dragEntity_ = entity;
    }

    [[nodiscard]] ecs::entity getHitTarget() const {
        return hitTarget_.valid() ? hitTarget_ : nullptr;
    }

private:
    void fireInteraction(InteractionEvent event, bool prev = true, bool onlyIfChanged = false);

    std::vector<ecs::entity>& getPrevTargets() {
        return targetLists[targetListIndex];
    }

    std::vector<ecs::entity>& getCurrentTargets() {
        return targetLists[(targetListIndex + 1) & 1];
    }

    void swapTargetLists() {
        targetListIndex = (++targetListIndex) & 1;
    }

private:
    ecs::entity hitTarget_;
    ecs::entity root_;

    std::vector<ecs::entity> targetLists[2]{};
    int targetListIndex = 0;

    bool mouseActive_ = false;
    uint64_t touchID_ = 0ull;
    float2 touchPosition0_;
    float2 mousePosition0_;

    ecs::entity dragEntity_;
};

}


