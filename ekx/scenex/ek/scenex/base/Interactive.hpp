#pragma once

#include <ek/util/Signal.hpp>
#include <ek/app/app.hpp>
#include <ecxx/ecxx_fwd.hpp>
#include <ek/util/Type.hpp>

namespace ek {

enum class InteractionEvent {
    PointerDown,
    PointerUp,
    PointerOver,
    PointerOut,
    Click
};

struct Interactive {

    using mouse_cursor = app::mouse_cursor;
    using signal = Signal<>;

    // events
    signal on_over;
    signal on_out;
    signal on_down;
    signal on_up;
    signal on_clicked;

    mouse_cursor cursor{mouse_cursor::parent};

    // TODO: Interactive::pointer is useless
    // global world space pointer, because of that maybe we don't dispatch this coordinate into components,
    // just use global pointer from Interaction System
    float2 pointer{};
    ecs::EntityRef camera{};

    //bool enabled = true;
    // todo: check bubble was false by default
    bool bubble = true;
    // responsive state
    bool over = false;
    bool pushed = false;

    Interactive() = default;

    void handle(InteractionEvent event);
};

EK_DECLARE_TYPE(Interactive);

}