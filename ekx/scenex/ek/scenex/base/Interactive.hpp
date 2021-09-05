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
    // events
    Signal<> on_over;
    Signal<> on_out;
    Signal<> on_down;
    Signal<> on_up;
    Signal<> on_clicked;

    app::MouseCursor cursor{app::MouseCursor::Parent};

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