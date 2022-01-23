#pragma once

#include <ek/app.h>
#include <ek/math.h>

#include <ek/util/Signal.hpp>
#include <ecxx/ecxx_fwd.hpp>

namespace ek {

enum class PointerEvent {
    Down,
    Up,
    Over,
    Out,
    Tap
};

struct Interactive {
    // events
    Signal<PointerEvent> onEvent;

    ek_mouse_cursor cursor = EK_MOUSE_CURSOR_PARENT;

    // TODO: Interactive::pointer is useless
    // global world space pointer, because of that maybe we don't dispatch this coordinate into components,
    // just use global pointer from Interaction System
    vec2_t pointer = {};
    ecs::EntityRef camera{};

    //bool enabled = true;
    // todo: check bubble was false by default
    bool bubble = true;
    // responsive state
    bool over = false;
    bool pushed = false;

    Interactive() = default;

    void handle(PointerEvent event);
};

}