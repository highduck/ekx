#pragma once

#include <ecxx/ecxx_fwd.hpp>

#include <ek/app.h>
#include <ek/math.h>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

namespace interactive_event {

inline constexpr auto back_button = "back_button";
inline constexpr auto system_pause = "system_pause";

}

class InteractionSystem {
public:
    // screen-space pointer position
    vec2_t pointerScreenPosition_ = {};
    bool pointerDown_ = false;

    explicit InteractionSystem();

    ~InteractionSystem() = delete;

    void process();

    ecs::EntityApi globalHitTest(vec2_t* worldSpacePointer, ecs::EntityRef* capturedCamera);
    ek_mouse_cursor searchInteractiveTargets(Array<ecs::EntityApi>& out_entities);

    void sendBackButton();

    void handle_system_pause();

    void handle_mouse_event(const ek_app_event& ev, vec2_t pos);

    void handle_touch_event(const ek_app_event& ev, vec2_t pos);

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

public:
    Array<ecs::EntityApi> targetLists[2]{};
    int targetListIndex = 0;

    ecs::EntityApi root_ = nullptr;
    ecs::EntityRef hitTarget_;

    bool mouseActive_ = false;
    uint64_t touchID_ = 0ull;
    vec2_t touchPosition0_ = {};
    vec2_t mousePosition0_ = {};

    ecs::EntityRef dragEntity_;
};

}

extern ek::InteractionSystem* g_interaction_system;
void init_interaction_system(void);
