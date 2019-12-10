#pragma once

#include <vector>
#include <ek/util/signals.hpp>
#include <ek/app/app.hpp>

namespace scenex {

struct interactive_t {

    using mouse_cursor = ek::app::mouse_cursor;
    using signal = ek::signal_t<>;

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