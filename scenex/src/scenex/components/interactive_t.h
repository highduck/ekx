#pragma once

#include <vector>
#include <ek/util/signals.hpp>
#include <ek/app/app.hpp>

namespace scenex {

struct interactive_t {

    using mouse_cursor_t = ek::mouse_cursor;
    using signal_t = ek::signal_t<>;

    // events
    signal_t on_over;
    signal_t on_out;
    signal_t on_down;
    signal_t on_up;
    signal_t on_clicked;

    mouse_cursor_t cursor{mouse_cursor_t::parent};

    //bool enabled = true;
    bool bubble = false;
    // responsive state
    bool over = false;
    bool pushed = false;

    interactive_t() = default;

    void set_pointer_out() {
        on_out();
        over = false;
        pushed = false;
    }

    void set_pointer_over() {
        over = true;
        on_over();
    }

    void set_pointer_up() {
        bool shouldBeClicked = pushed && over;
        pushed = false;
        on_up();

        if (shouldBeClicked) {
            on_clicked();
        }
    }

    void set_pointer_down() {
        pushed = true;
        on_down();
    }
};

}