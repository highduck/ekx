#pragma once

#include <vector>
#include <ek/util/signals.hpp>
#include <ek/app/app.hpp>

namespace ek {

enum class InteractionEvent {
    PointerDown,
    PointerUp,
    PointerOver,
    PointerOut,
    Click
};

struct interactive_t {

    using mouse_cursor = app::mouse_cursor;
    using signal = signal_t<>;

    // events
    signal on_over;
    signal on_out;
    signal on_down;
    signal on_up;
    signal on_clicked;

    mouse_cursor cursor{mouse_cursor::parent};

    //bool enabled = true;
    bool bubble = false;
    // responsive state
    bool over = false;
    bool pushed = false;

    interactive_t() = default;

    void handle(InteractionEvent event);
};

}