#pragma once

#include <ecx/ecx_fwd.hpp>

#include <ek/app.h>
#include <ek/math.h>
#include <ek/hash.h>

#define INTERACTIVE_EVENT_BACK_BUTTON H("back_button")
#define INTERACTIVE_EVENT_SYSTEM_PAUSE H("system_pause")

namespace ek {

struct InteractionSystem {

    entity_t globalHitTest(vec2_t* worldSpacePointer, entity_t* capturedCamera);
    ek_mouse_cursor searchInteractiveTargets(entity_t list[32]);

    void sendBackButton();

    void handle_system_pause();

    void handle_mouse_event(const ek_app_event* ev, vec2_t pos);

    void handle_touch_event(const ek_app_event* ev, vec2_t pos);

    void drag(entity_t entity);

    void fireInteraction(string_hash_t event, bool prev = true, bool onlyIfChanged = false);

    // screen-space pointer position
    vec2_t pointerScreenPosition_ = {};
    bool pointerDown_ = false;

    entity_t targetLists[2][32];
    int targetListIndex =0;

    entity_t root_ = NULL_ENTITY;
    entity_t hitTarget_;

    bool mouseActive_ = false;
    uint64_t touchID_ = 0ull;
    vec2_t touchPosition0_ = {};
    vec2_t mousePosition0_ = {};

    entity_t dragEntity_;
};

}

extern ek::InteractionSystem g_interaction_system;
void init_interaction_system(void);
void update_interaction_system(void);
