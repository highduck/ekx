#pragma once

#include <ek/util/Signal.hpp>
#include <ek/app/app.hpp>
#include <ecxx/ecxx_fwd.hpp>
#include <ek/util/Type.hpp>
#include <ek/math/Vec.hpp>

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

    app::MouseCursor cursor{app::MouseCursor::Parent};

    // TODO: Interactive::pointer is useless
    // global world space pointer, because of that maybe we don't dispatch this coordinate into components,
    // just use global pointer from Interaction System
    Vec2f pointer{};
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

ECX_TYPE(7, Interactive);

}