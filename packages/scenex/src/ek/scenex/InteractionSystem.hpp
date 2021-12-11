#pragma once

#include <ecxx/ecxx_fwd.hpp>

#include <ek/app.h>
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
    Vec2f pointerScreenPosition_ = Vec2f::zero;
    bool pointerDown_ = false;

    explicit InteractionSystem(ecs::EntityApi root);

    ~InteractionSystem() = delete;

    void process();

    ecs::EntityApi globalHitTest(Vec2f& worldSpacePointer, ecs::EntityRef& capturedCamera);
    ek_mouse_cursor searchInteractiveTargets(Array<ecs::EntityApi>& out_entities);

    void sendBackButton();

    void handle_system_pause();

    void handle_mouse_event(const ek_app_event& ev, Vec2f pos);

    void handle_touch_event(const ek_app_event& ev, Vec2f pos);

    void drag(ecs::EntityApi entity);

    [[nodiscard]]
    ecs::EntityApi getHitTarget() const;

private:
    void fireInteraction(PointerEvent event, bool prev = true, bool onlyIfChanged = false);

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
    Vec2f touchPosition0_;
    Vec2f mousePosition0_;

    ecs::EntityRef dragEntity_;
};

EK_DECLARE_TYPE(InteractionSystem);

}


