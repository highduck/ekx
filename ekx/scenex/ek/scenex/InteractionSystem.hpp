#pragma once

#include <ecxx/ecxx_fwd.hpp>

#include <ek/app/app.hpp>
#include <string>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/ds/Array.hpp>
#include <ek/util/Type.hpp>

namespace ek {

namespace interactive_event {

inline constexpr auto back_button = "back_button";
inline constexpr auto system_pause = "system_pause";

}

class InteractionSystem {
public:
    // screen-space pointer position
    float2 pointerScreenPosition_ = float2::zero;
    bool pointerDown_ = false;

    explicit InteractionSystem(ecs::EntityApi root);

    ~InteractionSystem() = delete;

    void process();

    ecs::EntityApi globalHitTest(float2& worldSpacePointer, ecs::EntityRef& capturedCamera);
    app::MouseCursor searchInteractiveTargets(Array<ecs::EntityApi>& out_entities);

    void sendBackButton();

    void handle_system_pause();

    void handle_mouse_event(const app::Event& ev, float2 pos);

    void handle_touch_event(const app::Event& ev, float2 pos);

    void drag(ecs::EntityApi entity);

    [[nodiscard]]
    ecs::EntityApi getHitTarget() const;

private:
    void fireInteraction(InteractionEvent event, bool prev = true, bool onlyIfChanged = false);

    Array<ecs::EntityApi>& getPrevTargets() {
        return targetLists[targetListIndex & 1];
    }

    Array<ecs::EntityApi>& getCurrentTargets() {
        return targetLists[(targetListIndex + 1) & 1];
    }

    void swapTargetLists() {
        targetListIndex = (++targetListIndex) & 1;
    }

private:
    Array<ecs::EntityApi> targetLists[2]{};
    int targetListIndex = 0;

    ecs::EntityRef hitTarget_;
    ecs::EntityApi root_;

    bool mouseActive_ = false;
    uint64_t touchID_ = 0ull;
    float2 touchPosition0_;
    float2 mousePosition0_;

    ecs::EntityRef dragEntity_;
};

EK_DECLARE_TYPE(InteractionSystem);

}


